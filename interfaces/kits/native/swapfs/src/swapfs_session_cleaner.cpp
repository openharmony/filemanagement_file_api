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

#include "swapfs_session_cleaner.h"

#include <cerrno>
#include <chrono>
#include <thread>
#include <utility>

#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "filemgmt_libhilog.h"
#include "swapfs_err_mapper.h"

namespace OHOS::FileManagement::Swapfs {
namespace {
constexpr mode_t LOCK_FILE_MODE = S_IRUSR | S_IWUSR;
constexpr const char *SESSION_PREFIX = "session-";
constexpr const char *CREATING_PREFIX = ".creating-";
constexpr const char *SESSION_MARKER = ".swapfs-session";
constexpr uint32_t CLEANUP_LOCK_RETRY_COUNT = 10;
constexpr uint32_t CLEANUP_LOCK_RETRY_INTERVAL_MS = 10;
constexpr uint32_t MAX_CLEANUP_DEPTH = 16;

bool StartsWith(const std::string &value, const char *prefix)
{
    return value.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

} // namespace

SwapfsSessionCleaner::SwapfsSessionCleaner(std::string swapRootPath)
    : swapRootPath_(std::move(swapRootPath))
{
}

int SwapfsSessionCleaner::AcquireCleanupLock(
    OHOS::UniqueFd &rootFd, OHOS::UniqueFd &lockFd) const
{
    int rootFlags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    rootFd = OHOS::UniqueFd(open(swapRootPath_.c_str(), rootFlags));
    if (rootFd < 0) {
        HILOGW("[Swapfs] swap root open failed, errno: %{public}d", errno);
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    int flags = O_CREAT | O_CLOEXEC | O_RDWR | O_NOFOLLOW;
    lockFd = OHOS::UniqueFd(openat(rootFd, "cleanup.lock", flags, LOCK_FILE_MODE));
    if (lockFd < 0) {
        HILOGW("[Swapfs] cleanup.lock open failed, errno: %{public}d", errno);
        return MapErrno(errno, SwapfsErrContext::PATH_OPERATION);
    }
    for (uint32_t attempt = 0; attempt < CLEANUP_LOCK_RETRY_COUNT; ++attempt) {
        if (flock(lockFd, LOCK_EX | LOCK_NB) == 0) {
            return SWAPFS_E_OK;
        }
        int err = errno;
        if (err != EWOULDBLOCK && err != EAGAIN && err != EINTR) {
            HILOGW("[Swapfs] cleanup.lock flock failed, errno: %{public}d", err);
            lockFd = OHOS::UniqueFd();
            return MapErrno(err, SwapfsErrContext::PATH_OPERATION);
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(CLEANUP_LOCK_RETRY_INTERVAL_MS));
    }
    HILOGW("[Swapfs] cleanup.lock acquire timed out");
    lockFd = OHOS::UniqueFd();
    return SWAPFS_E_BUSY;
}

SwapfsSessionCleaner::SessionState SwapfsSessionCleaner::GetSessionStateAt(
    int sessionFd, OHOS::UniqueFd &lockFd) const
{
    int flags = O_CLOEXEC | O_RDWR | O_NOFOLLOW;
    lockFd = OHOS::UniqueFd(openat(sessionFd, "session.lock", flags));
    if (lockFd < 0) {
        if (errno == ENOENT) {
            return SessionState::INACTIVE;
        }
        HILOGW("[Swapfs] session.lock open failed, errno: %{public}d", errno);
        return SessionState::UNKNOWN;
    }
    int err = 0;
    do {
        if (flock(lockFd, LOCK_EX | LOCK_NB) == 0) {
            return SessionState::INACTIVE;
        }
        err = errno;
    } while (err == EINTR);
    lockFd = OHOS::UniqueFd();
    if (err == EWOULDBLOCK || err == EAGAIN) {
        return SessionState::ACTIVE;
    }
    HILOGW("[Swapfs] session.lock state unknown, errno: %{public}d", err);
    return SessionState::UNKNOWN;
}

bool SwapfsSessionCleaner::RemoveEntryAt(int parentFd, const char *name, uint32_t depth)
{
    if (depth > MAX_CLEANUP_DEPTH) {
        HILOGW("[Swapfs] RemoveEntryAt depth exceeded, name: %{public}s", name);
        return false;
    }
    struct stat st {};
    if (fstatat(parentFd, name, &st, AT_SYMLINK_NOFOLLOW) != 0) {
        HILOGW("[Swapfs] RemoveEntryAt stat failed, name: %{public}s, errno: %{public}d",
            name, errno);
        return false;
    }
    if (!S_ISDIR(st.st_mode)) {
        if (unlinkat(parentFd, name, 0) == 0) {
            return true;
        }
        HILOGW("[Swapfs] RemoveEntryAt unlink failed, name: %{public}s, errno: %{public}d",
            name, errno);
        return false;
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    OHOS::UniqueFd childFd(openat(parentFd, name, flags));
    if (childFd < 0) {
        HILOGW("[Swapfs] RemoveEntryAt open dir failed, name: %{public}s, errno: %{public}d",
            name, errno);
        return false;
    }
    if (!RemoveTreeContentsAt(childFd, depth)) {
        HILOGW("[Swapfs] RemoveEntryAt remove child failed, name: %{public}s", name);
        return false;
    }
    childFd = OHOS::UniqueFd();
    if (unlinkat(parentFd, name, AT_REMOVEDIR) == 0) {
        return true;
    }
    HILOGW("[Swapfs] RemoveEntryAt rmdir failed, name: %{public}s, errno: %{public}d",
        name, errno);
    return false;
}

bool SwapfsSessionCleaner::RemoveTreeContentsAt(int dirFd, uint32_t depth)
{
    OHOS::UniqueFd scanFd(dup(dirFd));
    if (scanFd < 0) {
        HILOGW("[Swapfs] RemoveTreeContentsAt dup failed, errno: %{public}d", errno);
        return false;
    }
    int releasedFd = scanFd.Release();
    DIR *dir = fdopendir(releasedFd);
    if (dir == nullptr) {
        HILOGW("[Swapfs] RemoveTreeContentsAt fdopendir failed, errno: %{public}d", errno);
        (void)close(releasedFd);
        return false;
    }
    bool removed = true;
    errno = 0;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }
        struct stat markerStat {};
        bool keepMarker = name == SESSION_MARKER &&
            fstatat(dirFd, entry->d_name, &markerStat, AT_SYMLINK_NOFOLLOW) == 0 &&
            S_ISREG(markerStat.st_mode);
        if (keepMarker) {
            continue;
        }
        if (!RemoveEntryAt(dirFd, entry->d_name, depth + 1)) {
            removed = false;
        }
        errno = 0;
    }
    if (errno != 0) {
        HILOGW("[Swapfs] RemoveTreeContentsAt readdir failed, errno: %{public}d", errno);
        removed = false;
    }
    if (removed && unlinkat(dirFd, SESSION_MARKER, 0) != 0 && errno != ENOENT) {
        HILOGW("[Swapfs] RemoveTreeContentsAt marker unlink failed, errno: %{public}d",
            errno);
        removed = false;
    }
    (void)closedir(dir);
    return removed;
}

int SwapfsSessionCleaner::RemoveSessionTree(const std::string &path)
{
    struct stat st {};
    if (lstat(path.c_str(), &st) != 0) {
        return errno == ENOENT ? SWAPFS_E_OK : SWAPFS_E_PATH_UNAVAILABLE;
    }
    if (!S_ISDIR(st.st_mode)) {
        return unlink(path.c_str()) == 0 ? SWAPFS_E_OK : SWAPFS_E_PATH_UNAVAILABLE;
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    OHOS::UniqueFd dirFd(open(path.c_str(), flags));
    if (dirFd < 0 || !RemoveTreeContentsAt(dirFd, 0)) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    dirFd = OHOS::UniqueFd();
    return rmdir(path.c_str()) == 0 ? SWAPFS_E_OK : SWAPFS_E_PATH_UNAVAILABLE;
}

int SwapfsSessionCleaner::RemoveSessionAt(int rootFd, const std::string &name)
{
    return RemoveEntryAt(rootFd, name.c_str(), 0) ?
        SWAPFS_E_OK : SWAPFS_E_PATH_UNAVAILABLE;
}

bool SwapfsSessionCleaner::CleanupSessionEntry(int rootFd, const std::string &name) const
{
    bool isSession = StartsWith(name, SESSION_PREFIX);
    if (!isSession && !StartsWith(name, CREATING_PREFIX)) {
        return false;
    }
    struct stat st {};
    if (fstatat(rootFd, name.c_str(), &st, AT_SYMLINK_NOFOLLOW) != 0 ||
        !S_ISDIR(st.st_mode)) {
        return false;
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    OHOS::UniqueFd sessionFd(openat(rootFd, name.c_str(), flags));
    if (sessionFd < 0) {
        return false;
    }
    if (isSession) {
        struct stat markerStat {};
        if (fstatat(sessionFd, SESSION_MARKER, &markerStat,
            AT_SYMLINK_NOFOLLOW) != 0 || !S_ISREG(markerStat.st_mode)) {
            return false;
        }
    }
    OHOS::UniqueFd sessionLock;
    if (GetSessionStateAt(sessionFd, sessionLock) != SessionState::INACTIVE) {
        return false;
    }
    return RemoveEntryAt(rootFd, name.c_str(), 0);
}

int SwapfsSessionCleaner::CleanupInactiveSessionsLocked(int rootFd)
{
    if (rootFd < 0) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    OHOS::UniqueFd scanFd(dup(rootFd));
    if (scanFd < 0) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    int releasedFd = scanFd.Release();
    DIR *root = fdopendir(releasedFd);
    if (root == nullptr) {
        (void)close(releasedFd);
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    int cleanedCount = 0;
    struct dirent *entry = nullptr;
    errno = 0;
    while ((entry = readdir(root)) != nullptr) {
        if (CleanupSessionEntry(rootFd, entry->d_name)) {
            ++cleanedCount;
        }
        errno = 0;
    }
    int readErr = errno;
    (void)closedir(root);
    if (readErr != 0) {
        return SWAPFS_E_PATH_UNAVAILABLE;
    }
    if (cleanedCount > 0) {
        HILOGI("[Swapfs] cleaned stale session, count: %{public}d", cleanedCount);
    }
    return SWAPFS_E_OK;
}
} // namespace OHOS::FileManagement::Swapfs
