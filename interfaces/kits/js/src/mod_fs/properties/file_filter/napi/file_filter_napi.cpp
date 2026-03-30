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

#include "file_filter_napi.h"

#include <condition_variable>
#include <memory>
#include <mutex>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

struct FilterContext {
    bool filterResult = false;
    bool filterComplete = false;
    std::mutex mutex;
    std::condition_variable cv;
};

std::thread::id FileFilterNapi::mainThreadId;
std::once_flag FileFilterNapi::initFlag;

FileFilterNapi::FileFilterNapi(napi_env env, napi_value filterObj, LibN::NVal callback)
    : env_(env), filterObj_(filterObj), nRef_(callback)
{
    std::call_once(initFlag, InitMainThreadId);
}

FileFilterNapi::~FileFilterNapi() {}

void FileFilterNapi::InitMainThreadId()
{
    mainThreadId = std::this_thread::get_id();
}

bool FileFilterNapi::CallFilterFunction(const std::string &name)
{
    if (filterFailed_) {
        return false;
    }

    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env_, &scope);
    if (status != napi_ok) {
        HILOGE("Failed to open handle scope, status: %{public}d", status);
        filterFailed_ = true;
        return false;
    }

    napi_value nameValue = NVal::CreateUTF8String(env_, name).val_;
    napi_value callback = nRef_.Deref(env_).val_;
    napi_value args[1] = { nameValue };
    napi_value result;
    status = napi_call_function(env_, filterObj_, callback, 1, args, &result);
    if (status != napi_ok) {
        HILOGE("Failed to call filter function");
        filterFailed_ = true;
        napi_close_handle_scope(env_, scope);
        return false;
    }

    auto [succ, filterResult] = NVal(env_, result).ToBool();
    if (!succ) {
        HILOGE("Failed to get boolean result");
        filterFailed_ = true;
        napi_close_handle_scope(env_, scope);
        return false;
    }

    napi_close_handle_scope(env_, scope);
    return filterResult;
}

bool FileFilterNapi::AsyncCallFilterFunction(const std::string &name)
{
    if (filterFailed_) {
        return false;
    }

    auto context = CreateSharedPtr<FilterContext>();
    if (!context) {
        HILOGE("Failed to request heap memory.");
        filterFailed_ = true;
        return false;
    }

    auto task = [this, name, context]() {
        bool filterResult = CallFilterFunction(name);
        {
            lock_guard<mutex> lock(context->mutex);
            context->filterComplete = true;
            context->filterResult = filterResult;
            context->cv.notify_one();
        }
    };

    int ret = napi_send_event(env_, task, napi_eprio_immediate, "fs.fileFilter");
    if (ret != 0) {
        HILOGE("Failed to call napi_send_event, ret: %{public}d", ret);
        filterFailed_ = true;
        return false;
    }

    unique_lock<mutex> lock(context->mutex);
    context->cv.wait(lock, [context] {
        return context->filterComplete;
    });

    return context->filterResult;
}

bool FileFilterNapi::Filter(const std::string &name)
{
    if (std::this_thread::get_id() == mainThreadId) {
        return CallFilterFunction(name);
    } else {
        return AsyncCallFilterFunction(name);
    }
}

} // namespace OHOS::FileManagement::ModuleFileIO
