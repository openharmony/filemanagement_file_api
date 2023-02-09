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
#include "filemgmt_libhilog.h"
#include "../class_watcher/file_watcher.h"
#include "../class_watcher/watcher_entity.h"
#include "../class_watcher/watcher_n_exporter.h"
namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
napi_value Watcher::CreateWatcher(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succGetPath, filename, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succGetPath) {
        NError(EINVAL).ThrowErr(env, "Invalid filename");
        return nullptr;
    }

    auto [succGetEvent, events] = NVal(env, funcArg[NARG_POS::SECOND]).ToUint32();
    if (!succGetEvent) {
        NError(EINVAL).ThrowErr(env, "Invalid event");
        return nullptr;
    }

    int fd = -1;
    shared_ptr<FileWatcher> watcherPtr = make_shared<FileWatcher>();
    if (!watcherPtr->InitNotify(fd)) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }  

    WatcherInfoArg data;
    data.events = events;
    data.env = env;
    data.filename = string(filename.get());
    data.fd = fd;

    NVal val = NVal(env, funcArg[NARG_POS::THIRD]);
    napi_create_reference(val.env_, val.val_, 1, &(data.ref));


    napi_value objWatcher = NClass::InstantiateClass(env, WatcherNExporter::className_, {});
    if (!objWatcher) {
        NError(EINVAL).ThrowErr(env, "objWatcher create failed");
        return nullptr;
    }

    auto watcherEntity = NClass::GetEntityOf<WatcherEntity>(env, objWatcher);
    if (!watcherEntity) {
        NError(EINVAL).ThrowErr(env, "watcherEntity get failed");
        return nullptr;
    }
    watcherEntity->data_ = data;

    return objWatcher;
}
} // namespace OHOS::FileManagement::ModuleFileIO
