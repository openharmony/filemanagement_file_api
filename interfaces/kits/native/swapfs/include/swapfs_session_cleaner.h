/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_SESSION_CLEANER_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_SESSION_CLEANER_H

#include <cstdint>
#include <string>

#include "swapfs_errcode.h"
#include "unique_fd.h"

namespace OHOS::FileManagement::Swapfs {
class SwapfsSessionCleaner {
public:
    explicit SwapfsSessionCleaner(std::string swapRootPath);
    static int RemoveSessionTree(const std::string &path, const std::string &safeRoot);
    static int RemoveSessionAt(int rootFd, const std::string &name);
    int AcquireCleanupLock(OHOS::UniqueFd &rootFd, OHOS::UniqueFd &lockFd) const;
    int CleanupInactiveSessionsLocked(int rootFd);

private:
    enum class SessionState {
        ACTIVE,
        INACTIVE,
        UNKNOWN,
    };

    SessionState GetSessionStateAt(int sessionFd, OHOS::UniqueFd &lockFd) const;
    bool CleanupSessionEntry(int rootFd, const std::string &name) const;
    static bool RemoveEntryAt(int parentFd, const char *name, uint32_t depth);
    static bool RemoveTreeContentsAt(int dirFd, uint32_t depth);

    std::string swapRootPath_;
};
} // namespace OHOS::FileManagement::Swapfs

#endif
