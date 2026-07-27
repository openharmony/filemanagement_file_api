/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "swapfs_c_api.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <new>

#include "filemgmt_libhilog.h"

#include "swapfs_manager.h"

namespace {
struct ManagerApiState {
    uint32_t activeCalls = 0;
    bool destroying = false;
};

std::mutex g_managerStateMutex;
std::map<OH_SwapfsManager *, ManagerApiState> g_managerStates;

void RegisterManager(OH_SwapfsManager *manager)
{
    std::lock_guard<std::mutex> lock(g_managerStateMutex);
    g_managerStates.emplace(manager, ManagerApiState {});
}

class ApiCallGuard final {
public:
    explicit ApiCallGuard(OH_SwapfsManager *manager) : manager_(manager)
    {
        if (manager_ == nullptr) {
            status_ = SWAPFS_E_INVAL;
            return;
        }
        std::lock_guard<std::mutex> lock(g_managerStateMutex);
        auto iter = g_managerStates.find(manager_);
        if (iter == g_managerStates.end() || iter->second.destroying) {
            status_ = SWAPFS_E_SHUTTING_DOWN;
            manager_ = nullptr;
            return;
        }
        ++iter->second.activeCalls;
    }

    ~ApiCallGuard()
    {
        if (manager_ == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(g_managerStateMutex);
        auto iter = g_managerStates.find(manager_);
        if (iter != g_managerStates.end() && iter->second.activeCalls > 0) {
            --iter->second.activeCalls;
        }
    }

    int Status() const
    {
        return status_;
    }

private:
    OH_SwapfsManager *manager_ = nullptr;
    int status_ = SWAPFS_E_OK;
};

int BeginDestroy(OH_SwapfsManager *manager)
{
    std::lock_guard<std::mutex> lock(g_managerStateMutex);
    auto iter = g_managerStates.find(manager);
    if (iter == g_managerStates.end()) {
        return SWAPFS_E_SHUTTING_DOWN;
    }
    if (iter->second.destroying || iter->second.activeCalls > 0) {
        return SWAPFS_E_BUSY;
    }
    iter->second.destroying = true;
    return SWAPFS_E_OK;
}

void RestoreDestroyable(OH_SwapfsManager *manager)
{
    std::lock_guard<std::mutex> lock(g_managerStateMutex);
    auto iter = g_managerStates.find(manager);
    if (iter != g_managerStates.end()) {
        iter->second.destroying = false;
    }
}

void UnregisterManager(OH_SwapfsManager *manager)
{
    std::lock_guard<std::mutex> lock(g_managerStateMutex);
    g_managerStates.erase(manager);
}
} // namespace

__attribute__((visibility("default"))) int SwapfsNativeCreateManager(
    const OH_SwapfsConfig *config, OH_SwapfsManager **manager)
{
    if (manager == nullptr) {
        return SWAPFS_E_INVAL;
    }
    *manager = new (std::nothrow) OH_SwapfsManager();
    if (*manager == nullptr) {
        HILOGE("[Swapfs] CreateManager allocation failed");
        return SWAPFS_E_NOMEM;
    }
    int ret = (*manager)->impl.Init(config);
    if (ret != SWAPFS_E_OK) {
        delete *manager;
        *manager = nullptr;
        return ret;
    }
    RegisterManager(*manager);
    return SWAPFS_E_OK;
}

__attribute__((visibility("default"))) int SwapfsNativeDestroyManager(OH_SwapfsManager *manager)
{
    if (manager == nullptr) {
        return SWAPFS_E_INVAL;
    }
    int beginRet = BeginDestroy(manager);
    if (beginRet != SWAPFS_E_OK) {
        return beginRet;
    }
    int ret = manager->impl.Destroy();
    if (ret == SWAPFS_E_OK) {
        UnregisterManager(manager);
        delete manager;
    } else {
        RestoreDestroyable(manager);
    }
    // When BUSY: caller must retry Destroy after pending operations complete; object is not freed.
    return ret;
}

__attribute__((visibility("default"))) int SwapfsNativeSwapOut(
    OH_SwapfsManager *manager, const OH_SwapfsSwapOutRequest *request, uint64_t *keyId)
{
    if (request == nullptr || keyId == nullptr) {
        return SWAPFS_E_INVAL;
    }
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.SwapOut(request, keyId);
}

__attribute__((visibility("default"))) int SwapfsNativeSwapIn(
    OH_SwapfsManager *manager, const OH_SwapfsSwapInRequest *request, uint64_t *readSize)
{
    if (request == nullptr) {
        return SWAPFS_E_INVAL;
    }
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.SwapIn(request, readSize);
}

__attribute__((visibility("default"))) int SwapfsNativeQueryData(
    OH_SwapfsManager *manager, uint64_t keyId, OH_SwapfsDataInfo *info)
{
    if (keyId == 0 || info == nullptr) {
        return SWAPFS_E_INVAL;
    }
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.QueryData(keyId, info);
}

__attribute__((visibility("default"))) int SwapfsNativeGetStats(
    OH_SwapfsManager *manager, OH_SwapfsStats *stats)
{
    if (stats == nullptr) {
        return SWAPFS_E_INVAL;
    }
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.GetStats(stats);
}

__attribute__((visibility("default"))) int SwapfsNativeRemoveData(
    OH_SwapfsManager *manager, uint64_t keyId)
{
    if (keyId == 0) {
        return SWAPFS_E_INVAL;
    }
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.RemoveData(keyId);
}

__attribute__((visibility("default"))) int SwapfsNativeRemoveAllData(OH_SwapfsManager *manager)
{
    ApiCallGuard guard(manager);
    if (guard.Status() != SWAPFS_E_OK) {
        return guard.Status();
    }
    return manager->impl.RemoveAllData();
}
