/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <tuple>
#include <unistd.h>

#include "../../common/napi/n_async/n_ref.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"

#include "../class_watcher/watcher_entity.h"
#include "../class_watcher/watcher_n_exporter.h"
namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

void Watcher::RunCommand(uv_fs_event_t *handle, const char *filename, int events, int status)
{
    WatcherInforArg *information = (WatcherInforArg *)handle->data;
    uint32_t eventsFirst { events };
    uint32_t eventsSecond { information->events };
    if (eventsFirst & eventsSecond) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(information->env, &scope);
        napi_value callback = nullptr;
        napi_get_reference_value(information->env, information->ref, &callback);
        vector<napi_value> argv;
        argv.push_back(NVal::CreateInt64(information->env, events).val_);
        napi_value global = nullptr;
        napi_get_global(information->env, &global);
        napi_value tmp = nullptr;
        napi_call_function(information->env, global, callback, argv.size(), argv.data(), &tmp);
        napi_close_handle_scope(information->env, scope);
    }
}

napi_value Watcher::CreateWatcher(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto [succGetPath, filename, unused] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succGetPath) {
        UniError(EINVAL).ThrowErr(env, "Invalid filename");
        return nullptr;
    }

    auto [succGetEvent, event] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt32();
    if (!succGetEvent) {
        UniError(EINVAL).ThrowErr(env, "Invalid event");
        return nullptr;
    }

    unique_ptr<WatcherInforArg> data = make_unique<WatcherInforArg>();
    data->events = event;
    data->env = env;
    NVal val = NVal(env, funcArg[NARG_POS::THIRD]);
    napi_create_reference(val.env_, val.val_, 1, &(data->ref));
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    unique_ptr<uv_fs_event_t, WatcherHandleDeleter> fsEventReq(new uv_fs_event_t);
    uv_fs_event_init(loop, fsEventReq.get());
    fsEventReq->data = data.get();
    uv_fs_event_start(fsEventReq.get(), RunCommand, filename.get(), UV_FS_EVENT_RECURSIVE);

    napi_value objWatcher = NClass::InstantiateClass(env, WatcherNExporter::className_, {});
    if (!objWatcher) {
        UniError(EINVAL).ThrowErr(env, "objWatcher create failed");
        return nullptr;
    }

    auto watcherEntity = NClass::GetEntityOf<WatcherEntity>(env, objWatcher);
    if (!watcherEntity) {
        UniError(EINVAL).ThrowErr(env, "watcherEntity get failed");
        return nullptr;
    }

    watcherEntity->fsEventReq_ = std::move(fsEventReq);
    watcherEntity->data_ = std::move(data);

    return objWatcher;
}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS