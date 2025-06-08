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

#include "file_instantiator.h"

#include "file_entity.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::DistributedFS;

FsResult<FsFile *> FileInstantiator::InstantiateFile(int fd, string pathOrUri, bool isUri)
{
    FsResult<FsFile *> result = FsFile::Constructor();
    if (!result.IsSuccess()) {
        HILOGE("Failed to instantiate class");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return FsResult<FsFile *>::Error(EIO);
    }

    const FsFile *objFile = result.GetData().value();
    if (!objFile) {
        HILOGE("Failed to get fsFile");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        return FsResult<FsFile *>::Error(EIO);
    }

    auto *fileEntity = objFile->GetFileEntity();
    if (!fileEntity) {
        HILOGE("Failed to get fileEntity");
        int ret = close(fd);
        if (ret < 0) {
            HILOGE("Failed to close fd");
        }
        delete objFile;
        objFile = nullptr;
        return FsResult<FsFile *>::Error(EIO);
    }

    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(fd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(fd);
        delete objFile;
        objFile = nullptr;
        return FsResult<FsFile *>::Error(ENOMEM);
    }
    fileEntity->fd_.swap(fdg);
    if (isUri) {
        fileEntity->path_ = "";
        fileEntity->uri_ = pathOrUri;
    } else {
        fileEntity->path_ = pathOrUri;
        fileEntity->uri_ = "";
    }
    return result;
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS