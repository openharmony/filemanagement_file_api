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

#include <cerrno>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "swapfs_session_cleaner.h"
#include "swapfs_syscall_mock.h"
#include "unique_fd.h"

namespace {
using OHOS::FileManagement::Swapfs::SwapfsSessionCleaner;
using OHOS::FileManagement::Swapfs::Test::SwapfsSyscallMock;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetErrnoAndReturn;
using testing::StrEq;

constexpr const char *TEST_SWAP_BASE = "/data/swapfs_test/swapfs_cleaner_ut";
constexpr const char *TEST_SWAP_ROOT = "/data/swapfs_test/swapfs_cleaner_ut/swapfs";
constexpr const char *TEST_EXTERNAL_ROOT = "/data/swapfs_test/swapfs_cleaner_external";
constexpr mode_t TEST_DIR_MODE = S_IRWXU;
constexpr mode_t TEST_FILE_MODE = S_IRUSR | S_IWUSR;

void Mkdir(const std::string &path)
{
    ASSERT_EQ(mkdir(path.c_str(), TEST_DIR_MODE), 0);
}

void CreateFile(const std::string &path)
{
    int fd = open(path.c_str(), O_CREAT | O_CLOEXEC | O_WRONLY, TEST_FILE_MODE);
    ASSERT_GE(fd, 0);
    (void)close(fd);
}

bool Exists(const std::string &path)
{
    struct stat st {};
    return lstat(path.c_str(), &st) == 0;
}

std::string CreateSession(const std::string &name, bool marker = true)
{
    std::string path = std::string(TEST_SWAP_ROOT) + "/" + name;
    Mkdir(path);
    if (marker) {
        CreateFile(path + "/.swapfs-session");
    }
    return path;
}

int CleanupWithLock(SwapfsSessionCleaner &cleaner)
{
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;
    int ret = cleaner.AcquireCleanupLock(rootFd, cleanupLock);
    if (ret != SWAPFS_E_OK) {
        return ret;
    }
    return cleaner.CleanupInactiveSessionsLocked(rootFd);
}

class SwapfsSessionCleanerTest : public testing::Test {
public:
    void SetUp() override
    {
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_EXTERNAL_ROOT);
        Mkdir(TEST_SWAP_BASE);
        Mkdir(TEST_SWAP_ROOT);
        CreateFile(std::string(TEST_SWAP_ROOT) + "/.swapfs-root");
    }

    void TearDown() override
    {
        SwapfsSyscallMock::DisableMock();
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_EXTERNAL_ROOT);
    }
};

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleansUnlockedHistoricalSession_0000,
    testing::ext::TestSize.Level1)
{
    std::string inactive = CreateSession("session-inactive-random");
    CreateFile(inactive + "/session.lock");
    Mkdir(inactive + "/data");
    CreateFile(inactive + "/data/entry.swap");

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);
    EXPECT_FALSE(Exists(inactive));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleansNestedHistoricalSession_0000,
    testing::ext::TestSize.Level1)
{
    std::string inactive = CreateSession("session-nested-random");
    Mkdir(inactive + "/data");
    Mkdir(inactive + "/data/nested");
    CreateFile(inactive + "/data/nested/entry.swap");

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);
    EXPECT_FALSE(Exists(inactive));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_SkipsLockedHistoricalSession_0000,
    testing::ext::TestSize.Level1)
{
    std::string active = CreateSession("session-active-random");
    OHOS::UniqueFd lockFd(open((active + "/session.lock").c_str(),
        O_CREAT | O_CLOEXEC | O_RDWR, TEST_FILE_MODE));
    ASSERT_GE(lockFd, 0);
    ASSERT_EQ(flock(lockFd, LOCK_EX | LOCK_NB), 0);

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);
    EXPECT_TRUE(Exists(active));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleansSessionWithoutLock_0000,
    testing::ext::TestSize.Level1)
{
    std::string noLock = CreateSession("session-no-lock-random");
    ASSERT_FALSE(Exists(noLock + "/session.lock"));

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_FALSE(Exists(noLock));
    EXPECT_TRUE(Exists(TEST_SWAP_ROOT));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_SkipsUnknownSessionLockState_0000,
    testing::ext::TestSize.Level1)
{
    std::string unknown = CreateSession("session-unknown-lock-random");
    Mkdir(unknown + "/session.lock");

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_TRUE(Exists(unknown));
    EXPECT_TRUE(Exists(unknown + "/session.lock"));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_RetriesInterruptedSessionLockState_0000,
    testing::ext::TestSize.Level1)
{
    std::string inactive = CreateSession("session-interrupted-lock-random");
    CreateFile(inactive + "/session.lock");
    OHOS::UniqueFd sessionFd(open(inactive.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(sessionFd, 0);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Flock(_, LOCK_EX | LOCK_NB))
        .WillOnce(SetErrnoAndReturn(EINTR, -1))
        .WillOnce(Return(0));
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd lockFd;

    EXPECT_EQ(cleaner.GetSessionStateAt(sessionFd, lockFd),
        SwapfsSessionCleaner::SessionState::INACTIVE);
    EXPECT_GE(lockFd, 0);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleanupLockSerializesIndependentFds_0000,
    testing::ext::TestSize.Level1)
{
    SwapfsSessionCleaner first(TEST_SWAP_ROOT);
    SwapfsSessionCleaner second(TEST_SWAP_ROOT);
    OHOS::UniqueFd firstRoot;
    OHOS::UniqueFd secondRoot;
    OHOS::UniqueFd firstLock;
    OHOS::UniqueFd secondLock;

    ASSERT_EQ(first.AcquireCleanupLock(firstRoot, firstLock), SWAPFS_E_OK);
    EXPECT_EQ(second.AcquireCleanupLock(secondRoot, secondLock), SWAPFS_E_BUSY);
    firstLock = OHOS::UniqueFd();
    EXPECT_EQ(second.AcquireCleanupLock(secondRoot, secondLock), SWAPFS_E_OK);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleanupLockHandlesInterruptedAndFatalErrors_0000,
    testing::ext::TestSize.Level1)
{
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Flock(_, LOCK_EX | LOCK_NB))
        .WillOnce(Invoke([](int, int) {
            errno = EINTR;
            return -1;
        }))
        .WillOnce(Invoke([](int, int) {
            errno = EBADF;
            return -1;
        }));
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;

    EXPECT_NE(cleaner.AcquireCleanupLock(rootFd, cleanupLock), SWAPFS_E_OK);

    EXPECT_LT(cleanupLock, 0);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleanupLockRejectsSymlink_0000,
    testing::ext::TestSize.Level1)
{
    Mkdir(TEST_EXTERNAL_ROOT);
    CreateFile(std::string(TEST_EXTERNAL_ROOT) + "/lock");
    std::string lockPath = std::string(TEST_SWAP_ROOT) + "/cleanup.lock";
    ASSERT_EQ(symlink((std::string(TEST_EXTERNAL_ROOT) + "/lock").c_str(),
        lockPath.c_str()), 0);
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;

    EXPECT_NE(cleaner.AcquireCleanupLock(rootFd, cleanupLock), SWAPFS_E_OK);

    EXPECT_TRUE(Exists(std::string(TEST_EXTERNAL_ROOT) + "/lock"));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_CleansCreatingDirectoriesWithOptionalMarker_0000,
    testing::ext::TestSize.Level1)
{
    std::string marked = CreateSession(".creating-marked");
    std::string unmarked = CreateSession(".creating-unmarked", false);
    CreateFile(marked + "/partial.tmp");
    CreateFile(unmarked + "/partial.tmp");

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_FALSE(Exists(marked));
    EXPECT_FALSE(Exists(unmarked));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_IgnoresUnownedAndNonDirectoryCandidates_0000,
    testing::ext::TestSize.Level1)
{
    std::string unowned = CreateSession("session-business-random", false);
    std::string badMarker = CreateSession("session-marker-directory", false);
    Mkdir(badMarker + "/.swapfs-session");
    std::string regular = std::string(TEST_SWAP_ROOT) + "/session-regular-file";
    CreateFile(regular);

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_TRUE(Exists(unowned));
    EXPECT_TRUE(Exists(badMarker));
    EXPECT_TRUE(Exists(regular));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_DoesNotFollowSessionChildSymlink_0000,
    testing::ext::TestSize.Level1)
{
    Mkdir(TEST_EXTERNAL_ROOT);
    CreateFile(std::string(TEST_EXTERNAL_ROOT) + "/sentinel");
    std::string inactive = CreateSession("session-symlink-child");
    Mkdir(inactive + "/data");
    ASSERT_EQ(symlink(TEST_EXTERNAL_ROOT, (inactive + "/data/external").c_str()), 0);

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_FALSE(Exists(inactive));
    EXPECT_TRUE(Exists(std::string(TEST_EXTERNAL_ROOT) + "/sentinel"));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_IgnoresSessionDirectorySymlink_0000,
    testing::ext::TestSize.Level1)
{
    Mkdir(TEST_EXTERNAL_ROOT);
    CreateFile(std::string(TEST_EXTERNAL_ROOT) + "/sentinel");
    std::string linkPath = std::string(TEST_SWAP_ROOT) + "/session-external-link";
    ASSERT_EQ(symlink(TEST_EXTERNAL_ROOT, linkPath.c_str()), 0);

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);

    EXPECT_TRUE(Exists(linkPath));
    EXPECT_TRUE(Exists(std::string(TEST_EXTERNAL_ROOT) + "/sentinel"));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_RemoveTreeHandlesMissingFileAndSymlink_0000,
    testing::ext::TestSize.Level1)
{
    std::string missing = std::string(TEST_SWAP_ROOT) + "/missing";
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionTree(missing), SWAPFS_E_OK);

    std::string regular = std::string(TEST_SWAP_ROOT) + "/regular";
    CreateFile(regular);
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionTree(regular), SWAPFS_E_OK);

    Mkdir(TEST_EXTERNAL_ROOT);
    CreateFile(std::string(TEST_EXTERNAL_ROOT) + "/sentinel");
    std::string linkPath = std::string(TEST_SWAP_ROOT) + "/external";
    ASSERT_EQ(symlink(TEST_EXTERNAL_ROOT, linkPath.c_str()), 0);
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionTree(linkPath), SWAPFS_E_OK);
    EXPECT_TRUE(Exists(std::string(TEST_EXTERNAL_ROOT) + "/sentinel"));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_StopsCleanupAtMaximumDirectoryDepth_0000,
    testing::ext::TestSize.Level1)
{
    std::string inactive = CreateSession("session-deep-tree");
    std::string nested = inactive;
    std::vector<std::string> directories;
    for (uint32_t depth = 0; depth < 20; ++depth) {
        nested += "/nested";
        Mkdir(nested);
        directories.emplace_back(nested);
    }
    CreateFile(nested + "/entry.swap");

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(CleanupWithLock(cleaner), SWAPFS_E_OK);
    EXPECT_TRUE(Exists(inactive));
    EXPECT_TRUE(Exists(inactive + "/.swapfs-session"));
    EXPECT_EQ(
        SwapfsSessionCleaner::RemoveSessionTree(inactive), SWAPFS_E_PATH_UNAVAILABLE);

    (void)unlink((nested + "/entry.swap").c_str());
    for (auto iter = directories.rbegin(); iter != directories.rend(); ++iter) {
        (void)rmdir(iter->c_str());
    }
    (void)unlink((inactive + "/.swapfs-session").c_str());
    (void)rmdir(inactive.c_str());
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_MissingRootReportsPathUnavailable_0000,
    testing::ext::TestSize.Level1)
{
    (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE);
    ASSERT_FALSE(Exists(TEST_SWAP_ROOT));

    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    EXPECT_EQ(cleaner.CleanupInactiveSessionsLocked(-1), SWAPFS_E_PATH_UNAVAILABLE);

    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;
    EXPECT_EQ(
        cleaner.AcquireCleanupLock(rootFd, cleanupLock), SWAPFS_E_PATH_UNAVAILABLE);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_RejectsInvalidDescriptorsAndMissingEntries_0000,
    testing::ext::TestSize.Level1)
{
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd rootFd(open(TEST_SWAP_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionAt(rootFd, "session-missing"),
        SWAPFS_E_PATH_UNAVAILABLE);
    EXPECT_FALSE(cleaner.CleanupSessionEntry(rootFd, "session-missing"));
    EXPECT_FALSE(SwapfsSessionCleaner::RemoveTreeContentsAt(-1, 0));
    int staleFd = open(TEST_SWAP_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    ASSERT_GE(staleFd, 0);
    ASSERT_EQ(close(staleFd), 0);
    EXPECT_EQ(cleaner.CleanupInactiveSessionsLocked(staleFd),
        SWAPFS_E_PATH_UNAVAILABLE);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_RejectsDupFailures_0000,
    testing::ext::TestSize.Level1)
{
    OHOS::UniqueFd rootFd(open(TEST_SWAP_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Dup(_))
        .WillOnce(SetErrnoAndReturn(EMFILE, -1))
        .WillOnce(SetErrnoAndReturn(EMFILE, -1));
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);

    EXPECT_FALSE(SwapfsSessionCleaner::RemoveTreeContentsAt(rootFd, 0));
    EXPECT_EQ(cleaner.CleanupInactiveSessionsLocked(rootFd),
        SWAPFS_E_PATH_UNAVAILABLE);
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_RejectsInjectedDirectoryOpenFailures_0000,
    testing::ext::TestSize.Level1)
{
    std::string scanFailure = CreateSession("session-scan-open-failure");
    std::string removeFailure = CreateSession("session-remove-open-failure");
    OHOS::UniqueFd rootFd(open(TEST_SWAP_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);

    EXPECT_CALL(*mock, OpenAt(_, StrEq("session-scan-open-failure"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_FALSE(cleaner.CleanupSessionEntry(rootFd, "session-scan-open-failure"));
    EXPECT_CALL(*mock, OpenAt(_, StrEq("session-remove-open-failure"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionAt(
        rootFd, "session-remove-open-failure"), SWAPFS_E_PATH_UNAVAILABLE);
    EXPECT_TRUE(Exists(scanFailure));
    EXPECT_TRUE(Exists(removeFailure));
}

HWTEST_F(SwapfsSessionCleanerTest, Swapfs_HandlesDirectLockAndUnlinkFailures_0000,
    testing::ext::TestSize.Level1)
{
    std::string session = CreateSession("session-lock-error");
    CreateFile(session + "/session.lock");
    OHOS::UniqueFd sessionFd(open(session.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(sessionFd, 0);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Flock(_, LOCK_EX | LOCK_NB))
        .WillOnce(SetErrnoAndReturn(EIO, -1));
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd lockFd;
    EXPECT_EQ(cleaner.GetSessionStateAt(sessionFd, lockFd),
        SwapfsSessionCleaner::SessionState::UNKNOWN);

    std::string regular = std::string(TEST_SWAP_ROOT) + "/unlink-failure";
    CreateFile(regular);
    EXPECT_CALL(*mock, Unlink(StrEq(regular.c_str())))
        .WillOnce(SetErrnoAndReturn(EACCES, -1));
    EXPECT_EQ(SwapfsSessionCleaner::RemoveSessionTree(regular),
        SWAPFS_E_PATH_UNAVAILABLE);
}
} // namespace
