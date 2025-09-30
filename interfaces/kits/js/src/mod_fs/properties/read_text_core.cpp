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
#include "read_text_core.h"

#include <cinttypes>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>
#include <sys/stat.h>

#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static tuple<bool, int64_t, bool, int64_t, unique_ptr<char[]>> ValidReadTextArg(
    const std::optional<ReadTextOptions> &options)
{
    int64_t offset = -1;
    int64_t len = 0;
    bool hasLen = false;
    unique_ptr<char[]> encoding { new char[]{ "utf-8" } };

    if (!options.has_value()) {
        return { true, offset, hasLen, len, move(encoding) };
    }

    ReadTextOptions op = options.value();
    if (op.offset.has_value()) {
        offset = op.offset.value();
        if (offset < 0) {
            HILOGE("Illegal option.offset parameter");
            return { false, offset, hasLen, len, nullptr };
        }
    }

    if (op.length.has_value()) {
        len = op.length.value();
        if (len < 0 || len > UINT_MAX) {
            HILOGE("Illegal option.length parameter");
            return { false, offset, hasLen, len, nullptr };
        }
        hasLen = true;
    }

    if (op.encoding.has_value()) {
        auto encoding = op.encoding.value();
        if (encoding != "utf-8") {
            HILOGE("Illegal option.encoding parameter");
            return { false, offset, hasLen, len, nullptr };
        }
    }

    return { true, offset, hasLen, len, move(encoding) };
}

static int OpenFile(const std::string& path)
{
    FileFsTrace traceOpenFile("OpenFile");
    if (FileApiDebug::isLogEnabled) {
        HILOGD("Path is %{public}s", path.c_str());
    }
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> openReq = {
        new uv_fs_t, FsUtils::FsReqCleanup
    };
    if (openReq == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }

    return uv_fs_open(nullptr, openReq.get(), path.c_str(), O_RDONLY,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, nullptr);
}

static int ReadFromFile(int fd, int64_t offset, string& buffer)
{
    FileFsTrace traceReadFromFile("ReadFromFile");
    uv_buf_t readbuf = uv_buf_init(const_cast<char *>(buffer.c_str()), static_cast<unsigned int>(buffer.size()));
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> readReq = {
        new uv_fs_t, FsUtils::FsReqCleanup };
    if (readReq == nullptr) {
        HILOGE("Failed to request heap memory.");
        return -ENOMEM;
    }

    return uv_fs_read(nullptr, readReq.get(), fd, &readbuf, 1, offset, nullptr);
}

FsResult<tuple<string, int64_t>> ReadTextCore::DoReadText(const std::string &path,
    const std::optional<ReadTextOptions> &options)
{
    FileFsTrace traceDoReadText("DoReadText");
    auto [resGetReadTextArg, offset, hasLen, len, encoding] = ValidReadTextArg(options);
    if (!resGetReadTextArg) {
        return FsResult<tuple<string, int64_t>>::Error(EINVAL);
    }

    OHOS::DistributedFS::FDGuard sfd;
    int fd = OpenFile(path);
    if (fd < 0) {
        HILOGD("Failed to open file by ret: %{public}d", fd);
        return FsResult<tuple<string, int64_t>>::Error(fd);
    }
    sfd.SetFD(fd);

    struct stat statbf;
    FileFsTrace traceFstat("fstat");
    if ((!sfd) || (fstat(sfd.GetFD(), &statbf) < 0)) {
        HILOGE("Failed to get stat of file by fd: %{public}d", sfd.GetFD());
        return FsResult<tuple<string, int64_t>>::Error(errno);
    }
    traceFstat.End();

    if (offset > statbf.st_size) {
        HILOGE("Invalid offset: %{public}" PRIu64, offset);
        return FsResult<tuple<string, int64_t>>::Error(EINVAL);
    }

    len = (!hasLen || len > statbf.st_size) ? statbf.st_size : len;
    string buffer(len, '\0');
    int readRet = ReadFromFile(sfd.GetFD(), offset, buffer);
    if (readRet < 0) {
        HILOGE("Failed to read file by fd: %{public}d", fd);
        return FsResult<tuple<string, int64_t>>::Error(readRet);
    }

    return FsResult<tuple<string, int64_t>>::Success(make_tuple(move(buffer), readRet));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS