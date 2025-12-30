/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef UTILS_FILEMGMT_LIBFS_INCLUDE_FS_RESULT_H
#define UTILS_FILEMGMT_LIBFS_INCLUDE_FS_RESULT_H

#include "fs_error.h"

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace OHOS::FileManagement::ModuleFileIO {

template <typename T>
class FsResult {
    using OptionalData = std::optional<T>;

public:
    static FsResult<T> Success()
    {
        return FsResult(FsError(ERRNO_NOERR), std::nullopt);
    }

    static FsResult<T> Success(const T &data)
    {
        return FsResult(FsError(ERRNO_NOERR), std::make_optional<T>(data));
    }

    static FsResult<T> Success(T &&data)
    {
        return FsResult(FsError(ERRNO_NOERR), std::make_optional<T>(std::move(data)));
    }

    template <typename U = T, std::enable_if_t<std::is_same_v<U, std::string>, int> = 0>
    static FsResult<U> Success(const char *data)
    {
        return FsResult(FsError(ERRNO_NOERR), std::make_optional<U>(std::string(data)));
    }

    static FsResult<T> Error(const int32_t code)
    {
        return FsResult(FsError(code), std::nullopt);
    }

    static FsResult<T> ErrorWithMsg(const int32_t code, const std::string &msg)
    {
        return FsResult(FsError(code, msg), std::nullopt);
    }

    bool IsSuccess() const
    {
        return !error_;
    }

    const OptionalData &GetData() const
    {
        return data_;
    }

    OptionalData &GetData()
    {
        return data_;
    }

    const FsError &GetError() const
    {
        return error_;
    }

    FsResult(const FsResult &) = delete;
    FsResult &operator=(const FsResult &) = delete;

    FsResult(FsResult &&other) noexcept : error_(std::move(other.error_)), data_(std::move(other.data_)) {}

    FsResult &operator=(FsResult &&other) noexcept
    {
        if (this != &other) {
            error_ = std::move(other.error_);
            data_ = std::move(other.data_);
        }
        return *this;
    }

    ~FsResult<T>() = default;

private:
    FsError error_;
    OptionalData data_;

    FsResult(const FsError &err, const OptionalData &data) : error_(err), data_(data) {}

    FsResult(const FsError &err, OptionalData &&data) : error_(err), data_(std::move(data)) {}
};

template <>
class FsResult<void> {
private:
    FsError error_;
    explicit FsResult(const FsError &err) : error_(err) {}

public:
    static FsResult<void> Success()
    {
        return FsResult(FsError(ERRNO_NOERR));
    }

    static FsResult<void> ErrorWithMsg(const int32_t code, const std::string &msg)
    {
        return FsResult(FsError(code, msg));
    }

    static FsResult<void> Error(const int32_t code)
    {
        return FsResult(FsError(code));
    }

    bool IsSuccess() const
    {
        return !error_;
    }

    const FsError &GetError() const
    {
        return error_;
    }

    FsResult(const FsResult &) = delete;
    FsResult &operator=(const FsResult &) = delete;

    FsResult(FsResult &&other) noexcept : error_(std::move(other.error_)) {}

    FsResult &operator=(FsResult &&other) noexcept
    {
        if (this != &other) {
            error_ = std::move(other.error_);
        }
        return *this;
    }

    ~FsResult<void>() = default;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // UTILS_FILEMGMT_LIBFS_INCLUDE_FS_RESULT_H
