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

#include "fs_file.h"

#include <sys/file.h>

#include "file_uri.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
static tuple<int, unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *>> RealPathCore(const string &srcPath)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> realpathReq = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!realpathReq) {
        HILOGE("Failed to request heap memory.");
        return { ENOMEM, move(realpathReq) };
    }
    int ret = uv_fs_realpath(nullptr, realpathReq.get(), srcPath.c_str(), nullptr);
    return { ret, move(realpathReq) };
}

void FsFile::RemoveEntity()
{
    fileEntity = nullptr;
}

FsResult<int32_t> FsFile::GetFD() const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<int32_t>::Error(EINVAL);
    }
    return FsResult<int32_t>::Success(fileEntity->fd_.get()->GetFD());
}

FsResult<string> FsFile::GetPath() const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<string>::Error(EINVAL);
    }
    if (fileEntity->uri_.length() != 0) {
        AppFileService::ModuleFileUri::FileUri fileUri(fileEntity->uri_);
        return FsResult<string>::Success(fileUri.GetPath());
    }
    auto [realPathRes, realPath] = RealPathCore(fileEntity->path_);
    if (realPathRes != ERRNO_NOERR) {
        HILOGE("Failed to get real path");
        return FsResult<string>::Error(realPathRes);
    }
    return FsResult<string>::Success(string(static_cast<const char *>(realPath->ptr)));
}

FsResult<string> FsFile::GetName() const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<string>::Error(EINVAL);
    }
    if (fileEntity->uri_.length() != 0) {
        AppFileService::ModuleFileUri::FileUri fileUri(fileEntity->uri_);
        return FsResult<string>::Success(fileUri.GetName());
    }
    auto [realPathRes, realPath] = RealPathCore(fileEntity->path_);
    if (realPathRes != ERRNO_NOERR) {
        HILOGE("Failed to get real path");
        return FsResult<string>::Error(realPathRes);
    }
    string path(static_cast<const char *>(realPath->ptr));
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        HILOGE("Failed to split filename from path");
        return FsResult<string>::Error(ENOENT);
    }
    return FsResult<string>::Success(path.substr(pos + 1));
}

FsResult<string> FsFile::GetParent() const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<string>::Error(EINVAL);
    }

    string path(fileEntity->path_);
    if (fileEntity->uri_.length() != 0) {
        AppFileService::ModuleFileUri::FileUri fileUri(fileEntity->uri_);
        path = fileUri.GetPath();
    } else {
        auto [realPathRes, realPath] = RealPathCore(path);
        if (realPathRes) {
            HILOGE("Failed to get real path");
            return FsResult<string>::Error(realPathRes);
        }
        path = static_cast<const char *>(realPath->ptr);
    }
    auto pos = path.find_last_of('/');
    if (pos == string::npos) {
        HILOGE("Failed to split filename from path");
        return FsResult<string>::Error(ENOENT);
    }
    return FsResult<string>::Success(path.substr(0, pos));
}

FsResult<void> FsFile::Lock(bool exclusive) const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<void>::Error(EINVAL);
    }

    if (!fileEntity || !fileEntity->fd_.get()) {
        HILOGE("File has been closed in Lock cbExec possibly");
        return FsResult<void>::Error(EIO);
    }
    int ret = 0;
    auto mode = exclusive ? LOCK_EX : LOCK_SH;
    ret = flock(fileEntity->fd_.get()->GetFD(), mode);
    if (ret < 0) {
        HILOGE("Failed to lock file");
        return FsResult<void>::Error(errno);
    } else {
        return FsResult<void>::Success();
    }
}

FsResult<void> FsFile::TryLock(bool exclusive) const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<void>::Error(EINVAL);
    }

    int ret = 0;
    auto mode = static_cast<uint32_t>(exclusive ? LOCK_EX : LOCK_SH);
    ret = flock(fileEntity->fd_.get()->GetFD(), mode | LOCK_NB);
    if (ret < 0) {
        HILOGE("Failed to try to lock file");
        return FsResult<void>::Error(errno);
    }

    return FsResult<void>::Success();
}

FsResult<void> FsFile::UnLock() const
{
    if (!fileEntity) {
        HILOGE("Failed to get file entity");
        return FsResult<void>::Error(EINVAL);
    }

    int ret = 0;
    ret = flock(fileEntity->fd_.get()->GetFD(), LOCK_UN);
    if (ret < 0) {
        HILOGE("Failed to unlock file");
        return FsResult<void>::Error(errno);
    }
    return FsResult<void>::Success();
}
#endif

FsResult<FsFile *> FsFile::Constructor()
{
    auto rafEntity = CreateUniquePtr<FileEntity>();
    if (rafEntity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsFile *>::Error(ENOMEM);
    }
    FsFile *fsFilePtr = new FsFile(move(rafEntity));

    if (fsFilePtr == nullptr) {
        HILOGE("Failed to create FsFile object on heap.");
        return FsResult<FsFile *>::Error(ENOMEM);
    }

    return FsResult<FsFile *>::Success(move(fsFilePtr));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
