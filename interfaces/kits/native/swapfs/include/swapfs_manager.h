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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_MANAGER_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_MANAGER_H

#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "swapfs.h"
#include "swapfs_control.h"
#include "swapfs_proxy_control.h"
#include "swapfs_uring_read_engine.h"

namespace OHOS::FileManagement::Swapfs {
constexpr uint32_t DIO_ALIGNMENT = SWAPFS_DIO_ALIGNMENT;
constexpr const char *DEFAULT_SWAPFS_TEMP_PATH = "/data/storage/el2/base/temp";
constexpr const char *DEFAULT_SWAPFS_ROOT_PATH = "/data/storage/el2/base/temp/swapfs";

struct SwapfsConfigInner {
    std::string swapRootPath = DEFAULT_SWAPFS_ROOT_PATH;
    uint64_t spaceLimitBytes = DEFAULT_SPACE_LIMIT_BYTES;
    bool useDirectIo = false;
    std::string managerId;
};

struct SwapKeyEntry {
    uint64_t keyId = 0;
    std::string path;
    uint64_t dataSize = 0;
    int64_t createTime = 0;
    OH_SwapfsKeyStatus status = OH_SWAPFS_KEY_STATUS_ACTIVE;
    uint32_t readCount = 0;
};

class SwapfsManager {
public:
    SwapfsManager();
    explicit SwapfsManager(std::unique_ptr<SwapControlProvider> controlProvider);
    ~SwapfsManager();

    int Init(const OH_SwapfsConfig *config);
    int Destroy();
    int SwapOut(const OH_SwapfsSwapOutRequest *request, uint64_t *keyId);
    int SwapIn(const OH_SwapfsSwapInRequest *request, uint64_t *readSize);
    int QueryData(uint64_t keyId, OH_SwapfsDataInfo *info);
    int GetStats(OH_SwapfsStats *stats);
    int RemoveData(uint64_t keyId);
    int RemoveAllData();

private:
    void EndOperation();
    class ActiveOperationGuard final {
    public:
        explicit ActiveOperationGuard(SwapfsManager &manager) : manager_(manager) {}
        ~ActiveOperationGuard()
        {
            manager_.EndOperation();
        }
        ActiveOperationGuard(const ActiveOperationGuard &) = delete;
        ActiveOperationGuard &operator=(const ActiveOperationGuard &) = delete;

    private:
        SwapfsManager &manager_;
    };

    void ResolveConfig(const OH_SwapfsConfig *config, SwapfsConfigInner &inner);
    int PrepareSwapRoot();
    int PrepareSession();
    int CreateSessionLocked(int rootFd);
    void CloseSessionLock();
    void RemoveSessionDir();
    int PrepareForSwapOut(const OH_SwapfsSwapOutRequest *request, uint64_t &newKey,
        std::string &tmpPath, std::string &swapPath, bool &useDirectIo);
    void CommitSwapOutEntry(const SwapKeyEntry &entry, uint64_t *keyId);
    int PrepareForSwapIn(bool &useDirectIo);
    int LookupKeyForSwapIn(uint64_t keyId, SwapKeyEntry &entry);
    int ExecuteSwapInRead(
        const SwapKeyEntry &entry, void *buffer, size_t readIoSize, bool useDirectIo);
    void FinishSwapIn(uint64_t keyId);
    int PrepareRemoveEntry(uint64_t keyId, SwapKeyEntry &entry);
    void FinalizeRemoveEntry(const SwapKeyEntry &entry, bool removed);
    int CollectRemovableEntries(std::vector<SwapKeyEntry> &entriesToRemove);
    bool AllEntriesCleanLocked();
    bool WaitForActiveOps(uint32_t timeoutMs);

    std::mutex mutex_;
    std::condition_variable activeOpsCv_;
    SwapfsConfigInner config_;
    std::unique_ptr<SwapControlProvider> control_;
    UringReadEngine uringReader_;
    std::unordered_map<uint64_t, SwapKeyEntry> entries_;
    std::string sessionPath_;
    uint64_t nextKeyId_ = 1;
    uint32_t activeOps_ = 0;
    int sessionLockFd_ = -1;
    bool removeAllInProgress_ = false;
    bool initialized_ = false;
    bool shuttingDown_ = false;
};
} // namespace OHOS::FileManagement::Swapfs

struct OH_SwapfsManager {
    OHOS::FileManagement::Swapfs::SwapfsManager impl;
};

#endif
