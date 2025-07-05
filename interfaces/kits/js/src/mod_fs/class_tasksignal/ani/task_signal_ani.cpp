/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "task_signal_ani.h"

#include "ani_helper.h"
#include "copy_core.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fs_task_signal.h"
#include "task_signal_wrapper.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

void TaskSignalAni::Cancel(ani_env *env, [[maybe_unused]] ani_object object)
{
    FsTaskSignal *copySignal = TaskSignalWrapper::Unwrap(env, object);
    if (copySignal == nullptr) {
        HILOGE("Cannot unwrap copySignal!");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    auto ret = copySignal->Cancel();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot Cancel!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

void TaskSignalAni::OnCancel(ani_env *env, [[maybe_unused]] ani_object object)
{
    FsTaskSignal *copySignal = TaskSignalWrapper::Unwrap(env, object);
    if (copySignal == nullptr) {
        HILOGE("Cannot unwrap copySignal!");
        ErrorHandler::Throw(env, EINVAL);
        return;
    }
    auto ret = copySignal->OnCancel();
    if (!ret.IsSuccess()) {
        HILOGE("Cannot Cancel!");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS