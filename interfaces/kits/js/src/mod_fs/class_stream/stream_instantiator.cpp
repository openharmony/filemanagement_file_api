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

#include "stream_instantiator.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "stream_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<FsStream *> StreamInstantiator::InstantiateStream(FILE *file)
{
    FsResult<FsStream *> result = FsStream::Constructor();
    if (!result.IsSuccess()) {
        HILOGE("Failed to instantiate class");
        int ret = fclose(file);
        if (ret < 0) {
            HILOGE("Failed to close file");
        }
        return FsResult<FsStream *>::Error(EIO);
    }

    const FsStream *objStream = result.GetData().value();
    if (!objStream) {
        HILOGE("Failed to get FsStream");
        int ret = fclose(file);
        if (ret < 0) {
            HILOGE("Failed to close file");
        }
        return FsResult<FsStream *>::Error(EIO);
    }

    auto *streamEntity = objStream->GetStreamEntity();
    if (!streamEntity) {
        HILOGE("Failed to get streamEntity");
        int ret = fclose(file);
        if (ret < 0) {
            HILOGE("Failed to close file");
        }
        delete objStream;
        objStream = nullptr;
        return FsResult<FsStream *>::Error(EIO);
    }

    auto fp = std::shared_ptr<FILE>(file, fclose);
    if (fp == nullptr) {
        HILOGE("Failed to request heap memory.");
        int ret = fclose(file);
        if (ret < 0) {
            HILOGE("Failed to close file");
        }
        delete objStream;
        objStream = nullptr;
        return FsResult<FsStream *>::Error(ENOMEM);
    }

    streamEntity->fp.swap(fp);
    return result;
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS