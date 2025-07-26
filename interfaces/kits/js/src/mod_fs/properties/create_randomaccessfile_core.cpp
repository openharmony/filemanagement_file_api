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
#include "create_randomaccessfile_core.h"

#include <securec.h>

#include "file_entity.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "fs_randomaccessfile.h"
#include "fs_utils.h"
#include "randomaccessfile_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, FileInfo, int> ParseStringToFileInfo(const string &path)
{
    OHOS::DistributedFS::FDGuard sfd;
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(sfd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(sfd);
        return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
    }
    size_t length = path.length() + 1;
    auto chars = std::make_unique<char[]>(length);
    auto ret = strncpy_s(chars.get(), length, path.c_str(), length - 1);
    if (ret != EOK) {
        HILOGE("Copy file path failed!");
        return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
    }
    return { true, FileInfo { true, move(chars), move(fdg) }, ERRNO_NOERR};
}

static tuple<bool, FileInfo, int> ParseFdToFileInfo(const int32_t &fd)
{
    if (fd < 0) {
        HILOGE("Invalid fd");
        return { false, FileInfo { false, nullptr, nullptr }, EINVAL};
    }
    auto dupFd = dup(fd);
    if (dupFd < 0) {
        HILOGE("Failed to get valid fd, fail reason: %{public}s, fd: %{public}d", strerror(errno), fd);
        return { false, FileInfo { false, nullptr, nullptr }, EINVAL};
    }
    auto fdg = CreateUniquePtr<DistributedFS::FDGuard>(dupFd, false);
    if (fdg == nullptr) {
        HILOGE("Failed to request heap memory.");
        close(dupFd);
        return { false, FileInfo { false, nullptr, nullptr }, ENOMEM};
    }
    return { true, FileInfo { false, nullptr, move(fdg) }, ERRNO_NOERR};
}

static tuple<bool, int64_t, int64_t> ValidRafOptions(const optional<RandomAccessFileOptions> &options)
{
    RandomAccessFileOptions op = options.value();
    int64_t opStart = INVALID_POS;
    int64_t opEnd = INVALID_POS;

    optional<int64_t> startOp = op.start;
    optional<int64_t> endOp = op.end;

    if (startOp.has_value()) {
        int64_t start = 0;
        start = startOp.value();
        if (start < 0) {
            HILOGE("Invalid option.start, positive integer is desired");
            return {false, opStart, opEnd};
        }
        opStart = start;
    }
    if (endOp.has_value()) {
        int64_t end = 0;
        end = endOp.value();
        if (end < 0) {
            HILOGE("Invalid option.end, positive integer is desired");
            return {false, opStart, opEnd};
        }
        opEnd = end;
    }
    return {true, opStart, opEnd};
}

static tuple<bool, uint32_t, int64_t, int64_t> ValidAndConvertFlags(const optional<int32_t> &mode,
    const optional<RandomAccessFileOptions> &options, FileInfo &fileInfo)
{
    uint32_t flags = O_RDONLY;
    int64_t start = INVALID_POS;
    int64_t end = INVALID_POS;
    if (fileInfo.isPath && mode.has_value()) {
        auto modeValue = mode.value();
        if (modeValue < 0) {
            HILOGE("Invalid flags");
            return {false, flags, start, end};
        }
        flags = FsUtils::ConvertFlags(static_cast<uint32_t>(modeValue));
    }

    if (options.has_value()) {
        auto [succOpt, start, end] = ValidRafOptions(options);
        if (!succOpt) {
            HILOGE("Invalid RandomAccessFile options");
            return {false, flags, start, end};
        }
    }

    return {true, flags, start, end};
}

static FsResult<FsRandomAccessFile *> InstantiateRandomAccessFile(unique_ptr<DistributedFS::FDGuard> fdg,
                                                                  int64_t fp,
                                                                  int64_t start = INVALID_POS,
                                                                  int64_t end = INVALID_POS)
{
    FsResult<FsRandomAccessFile *> result = FsRandomAccessFile::Constructor();
    if (!result.IsSuccess()) {
        HILOGE("Failed to instantiate class");
        return FsResult<FsRandomAccessFile *>::Error(EIO);
    }

    const FsRandomAccessFile *objRAF = result.GetData().value();
    if (!objRAF) {
        HILOGE("Cannot instantiate randomaccessfile");
        return FsResult<FsRandomAccessFile *>::Error(EIO);
    }

    auto *rafEntity = objRAF->GetRAFEntity();
    if (!rafEntity) {
        HILOGE("Cannot instantiate randomaccessfile because of void entity");
        return FsResult<FsRandomAccessFile *>::Error(EIO);
    }
    rafEntity->fd.swap(fdg);
    rafEntity->filePointer = fp;
    rafEntity->start = start;
    rafEntity->end = end;
    return result;
}

FsResult<FsRandomAccessFile *> CreateRandomAccessFileCore::DoCreateRandomAccessFile(
    const string &path, const optional<int32_t> &mode, const optional<RandomAccessFileOptions> &options)
{
    auto [succ, fileInfo, err] = ParseStringToFileInfo(path);
    if (!succ) {
        return FsResult<FsRandomAccessFile *>::Error(err);
    }

    auto [succFlags, flags, ignoreStart, ignoreEnd] = ValidAndConvertFlags(mode, options, fileInfo);
    if (!succFlags) {
        return FsResult<FsRandomAccessFile *>::Error(EINVAL);
    }

    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> openReq = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!openReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsRandomAccessFile *>::Error(ENOMEM);
    }

    int ret = uv_fs_open(nullptr, openReq.get(), fileInfo.path.get(), flags, S_IRUSR |
        S_IWUSR | S_IRGRP | S_IWGRP, NULL);
    if (ret < 0) {
        return FsResult<FsRandomAccessFile *>::Error(ret);
    }

    fileInfo.fdg->SetFD(openReq.get()->result, false);

    if (options.has_value()) {
        auto [succ, start, end] = ValidRafOptions(options);
        if (succ) {
            return InstantiateRandomAccessFile(move(fileInfo.fdg), 0, start, end);
        }
    }
    return InstantiateRandomAccessFile(move(fileInfo.fdg), 0);
}

FsResult<FsRandomAccessFile *> CreateRandomAccessFileCore::DoCreateRandomAccessFile(
    const int32_t &fd, const optional<RandomAccessFileOptions> &options)
{
    auto [succ, fileInfo, err] = ParseFdToFileInfo(fd);
    if (!succ) {
        return FsResult<FsRandomAccessFile *>::Error(err);
    }
    if (options.has_value()) {
        auto [succ, start, end] = ValidRafOptions(options);
        if (succ) {
            return InstantiateRandomAccessFile(move(fileInfo.fdg), 0, start, end);
        }
    }
    return InstantiateRandomAccessFile(move(fileInfo.fdg), 0);
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS