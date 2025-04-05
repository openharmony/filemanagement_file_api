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

#include "create_stream_core.h"

#include <memory>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"
#include "stream_instantiator.h"
#include "stream_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<FsStream *> CreateStreamCore::DoCreateStream(const std::string &path, const std::string &mode)
{
    FILE *file = fopen(path.c_str(), mode.c_str());
    if (!file) {
        HILOGE("Failed to fopen file by path");
        return FsResult<FsStream *>::Error(errno);
    }

    return StreamInstantiator::InstantiateStream(move(file));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS