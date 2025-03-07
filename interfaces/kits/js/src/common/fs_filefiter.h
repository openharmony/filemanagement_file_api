/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FS_FILE_FILTER_H
#define FS_FILE_FILTER_H

#include <optional>
#include <string>
#include <vector>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

class FsFileFilter {
public:
    FsFileFilter() = default;
    ~FsFileFilter() = default;

    FsFileFilter(const FsFileFilter &filter) = default;
    FsFileFilter &operator=(const FsFileFilter &filter) = default;

    void SetSuffix(const std::optional<std::vector<std::string>> &suffix)
    {
        suffix_ = std::move(suffix);
    }

    const std::optional<std::vector<std::string>> &GetSuffix() const
    {
        return suffix_;
    }

    void SetDisplayName(const std::optional<std::vector<std::string>> &displayName)
    {
        displayName_ = std::move(displayName);
    }

    const std::optional<std::vector<std::string>> &GetDisplayName() const
    {
        return displayName_;
    }

    void SetMimeType(const std::optional<std::vector<std::string>> &mimeType)
    {
        mimeType_ = std::move(mimeType);
    }

    const std::optional<std::vector<std::string>> &GetMimeType() const
    {
        return mimeType_;
    }

    void SetFileSizeOver(const std::optional<int64_t> &fileSizeOver)
    {
        fileSizeOver_ = std::move(fileSizeOver);
    }

    const std::optional<int64_t> &GetFileSizeOver() const
    {
        return fileSizeOver_;
    }

    void SetLastModifiedAfter(const std::optional<int64_t> &lastModifiedAfter)
    {
        lastModifiedAfter_ = std::move(lastModifiedAfter);
    }

    const std::optional<double> &GetLastModifiedAfter() const
    {
        return lastModifiedAfter_;
    }

    void SetExcludeMedia(const bool &excludeMedia)
    {
        excludeMedia_ = excludeMedia;
    }

    bool GetExcludeMedia() const
    {
        return excludeMedia_;
    }

    void SetHasFilter(const bool &hasFilter)
    {
        hasFilter_ = hasFilter;
    }

    bool GetHasFilter() const
    {
        return hasFilter_;
    }

private:
    std::optional<std::vector<std::string>> suffix_ = std::nullopt;
    std::optional<std::vector<std::string>> displayName_ = std::nullopt;
    std::optional<std::vector<std::string>> mimeType_ = std::nullopt;
    std::optional<int64_t> fileSizeOver_ = std::nullopt;
    std::optional<double> lastModifiedAfter_ = std::nullopt;
    bool excludeMedia_ = false;
    bool hasFilter_ = false;
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // FS_FILE_FILTER_H