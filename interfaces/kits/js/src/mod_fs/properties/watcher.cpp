/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "watcher.h"

#include <cstring>
#include <fcntl.h>
#include <memory>
#include <tuple>
#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "../class_watcher/watcher_entity.h"
#include "../class_watcher/watcher_n_exporter.h"
namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static tuple<napi_value, int32_t> CreateAndCheckForWatcherEntity(napi_env env)
{
    if (FileWatcher::GetInstance().GetNotifyId() < 0 && !FileWatcher::GetInstance().InitNotify()) {
        HILOGE("Failed to get notifyId or initnotify fail");
        return {nullptr, errno};
    }
    napi_value objWatcher = NClass::InstantiateClass(env, WatcherNExporter::className_, {});
    if (!objWatcher) {
        HILOGE("Failed to instantiate watcher");
        return {nullptr, EIO};
    }
    return {objWatcher, ERRNO_NOERR};
}

shared_ptr<WatcherInfoArg> ParseParam(const napi_env &env, const napi_callback_info &info, int32_t &errCode)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        HILOGE("Failed to get param.");
        errCode = EINVAL;
        return nullptr;
    }

    auto [succGetPath, filename, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succGetPath) {
        HILOGE("Failed to get watcher path.");
        errCode = EINVAL;
        return nullptr;
    }

    auto [succGetEvent, events] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succGetEvent || events <= 0 || !FileWatcher::GetInstance().CheckEventValid(events)) {
        HILOGE("Failed to get watcher event.");
        errCode = EINVAL;
        return nullptr;
    }

    if (!NVal(env, funcArg[NARG_POS::THIRD]).TypeIs(napi_function)) {
        HILOGE("Failed to get callback");
        errCode = EINVAL;
        return nullptr;
    }
    auto infoArg = CreateSharedPtr<WatcherInfoArg>(NVal(env, funcArg[NARG_POS::THIRD]));
    if (infoArg == nullptr) {
        HILOGE("Failed to request heap memory.");
        errCode = ENOMEM;
        return nullptr;
    }
    infoArg->events = static_cast<uint32_t>(events);
    infoArg->env = env;
    infoArg->fileName = string(filename.get());

    return infoArg;
}

napi_value Watcher::CreateWatcher(napi_env env, napi_callback_info info)
{
    int errCode = 0;
    auto infoArg = ParseParam(env, info, errCode);
    if (errCode != 0) {
        HILOGE("Failed to parse param");
        NError(errCode).ThrowErr(env);
        return nullptr;
    }

    auto [objWatcher, err] = CreateAndCheckForWatcherEntity(env);
    if (!objWatcher) {
        HILOGE("Failed to create watcher entity.");
        NError(err).ThrowErr(env);
        return nullptr;
    }

    auto watcherEntity = NClass::GetEntityOf<WatcherEntity>(env, objWatcher);
    if (!watcherEntity) {
        HILOGE("Failed to get WatcherEntity.");
        NError(EIO).ThrowErr(env);
        return nullptr;
    }
    watcherEntity->data_ = infoArg;

    bool ret = FileWatcher::GetInstance().AddWatcherInfo(infoArg->fileName, infoArg);
    if (!ret) {
        HILOGE("Failed to add watcher info.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    return objWatcher;
}
} // namespace OHOS::FileManagement::ModuleFileIO
