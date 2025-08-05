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

#include "fdopen_stream_core.h"

#include <memory>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"
#include "stream_entity.h"
#include "stream_instantiator.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<FsStream *> FdopenStreamCore::DoFdopenStream(const int &fd, const string &mode)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return FsResult<FsStream *>::Error(EINVAL);
    }

    FILE *file = fdopen(fd, mode.c_str());
    if (!file) {
        HILOGE("Failed to fopen file by fd");
        return FsResult<FsStream *>::Error(errno);
    }
    return StreamInstantiator::InstantiateStream(move(file));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS