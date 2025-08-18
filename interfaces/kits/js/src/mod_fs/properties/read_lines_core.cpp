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
#include "read_lines_core.h"

#include <unistd.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "rust_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static int CheckOptionArg(Options option)
{
    auto encoding = option.encoding;
    if (encoding != "utf-8") {
        return EINVAL;
    }

    return ERRNO_NOERR;
}

static int GetFileSize(const string &path, int64_t &offset)
{
    std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> readLinesReq = { new (std::nothrow) uv_fs_t,
        FsUtils::FsReqCleanup };
    if (!readLinesReq) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_stat(nullptr, readLinesReq.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get file stat by path");
        return ret;
    }

    offset = static_cast<int64_t>(readLinesReq->statbuf.st_size);
    return ERRNO_NOERR;
}

static FsResult<FsReaderIterator *> InstantiateReaderIterator(void *iterator, int64_t offset)
{
    if (iterator == nullptr) {
        HILOGE("Invalid argument iterator");
        return FsResult<FsReaderIterator *>::Error(EINVAL);
    }

    auto readeriterator = FsReaderIterator::Constructor();
    if (!readeriterator.IsSuccess()) {
        HILOGE("Failed to instantiate class ReaderIterator");
        return FsResult<FsReaderIterator *>::Error(UNKNOWN_ERR);
    }

    auto readerIteratorEntity = readeriterator.GetData().value()->GetReaderIteratorEntity();
    if (!readerIteratorEntity) {
        HILOGE("Failed to get readerIteratorEntity");
        return FsResult<FsReaderIterator *>::Error(UNKNOWN_ERR);
    }
    readerIteratorEntity->iterator = iterator;
    readerIteratorEntity->offset = offset;
    return FsResult<FsReaderIterator *>::Success(readeriterator.GetData().value());
}

FsResult<FsReaderIterator *> ReadLinesCore::DoReadLines(const string &path, optional<Options> option)
{
    if (option.has_value()) {
        int ret = CheckOptionArg(option.value());
        if (ret) {
            HILOGE("Invalid option.encoding parameter");
            return FsResult<FsReaderIterator *>::Error(ret);
        }
    }

    auto iterator = ::ReaderIterator(path.c_str());
    if (iterator == nullptr) {
        HILOGE("Failed to read lines of the file, error: %{public}d", errno);
        return FsResult<FsReaderIterator *>::Error(errno);
    }

    int64_t offset = 0;
    int ret = GetFileSize(path, offset);
    if (ret != 0) {
        HILOGE("Failed to get size of the file");
        return FsResult<FsReaderIterator *>::Error(ret);
    }
    return InstantiateReaderIterator(iterator, offset);
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS