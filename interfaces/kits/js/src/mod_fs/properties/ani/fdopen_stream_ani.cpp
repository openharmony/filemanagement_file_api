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

#include "fdopen_stream_ani.h"

#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "fdopen_stream_core.h"
#include "stream_wrapper.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

ani_object FdopenStreamAni::FdopenStreamSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_int fd, ani_string mode)
{
    auto [succMode, openMode] = TypeConverter::ToUTF8String(env, mode);
    if (!succMode) {
        HILOGE("Invalid mode");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    FsResult<FsStream *> ret = FdopenStreamCore::DoFdopenStream(static_cast<int32_t>(fd), openMode);
    if (!ret.IsSuccess()) {
        HILOGE("Fdopen stream failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const FsStream *stream = ret.GetData().value();
    auto result = StreamWrapper::Wrap(env, move(stream));
    if (result == nullptr) {
        delete stream;
        stream = nullptr;
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }

    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS