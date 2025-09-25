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

#include "copy_file_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyFileCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CopyFileCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "CopyFileCoreMockTest");
    UvFsMock::EnableMock();
}

void CopyFileCoreMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyFileCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyFileCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_001
 * @tc.desc: Test function of CopyFileCore::ValidMode interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_001";

    FileInfo src;
    FileInfo dest;
    optional<int32_t> mode = std::make_optional(1);

    auto res = CopyFileCore::DoCopyFile(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_001";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_003
 * @tc.desc: Test function of CopyFileCore::OpenFile.OpenCore interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_003";

    FileInfo src;
    FileInfo dest;
    src.isPath = true;
    dest.isPath = false;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(-1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_003";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_005
 * @tc.desc: Test function of CopyFileCore::OpenFile.OpenCore interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_005";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;
    int fd = open("CopyFileCoreMockTest_DoCopyFile_005.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(fd, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(fd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(-1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_005";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_006
 * @tc.desc: Test function of CopyFileCore::OpenFile.OpenCore interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_006";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;
    int fd = open("CopyFileCoreMockTest_DoCopyFile_006.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(fd, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(fd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_006";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_007
 * @tc.desc: Test function of CopyFileCore::OpenFile.TruncateCore interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_007";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = false;
    int srcFd = open("CopyFileCoreMockTest_DoCopyFile_007_src.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcFd, -1);
    int destFd = open("CopyFileCoreMockTest_DoCopyFile_007_dest.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(destFd, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcFd);
    dest.fdg = make_unique<DistributedFS::FDGuard>(destFd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).Times(1).WillOnce(Return(-1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_007";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_008
 * @tc.desc: Test function of CopyFileCore::OpenFile.TruncateCore interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_008";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = false;
    int fd = open("CopyFileCoreMockTest_DoCopyFile_008.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(fd, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(fd);
    dest.fdg = make_unique<DistributedFS::FDGuard>();

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).Times(1).WillOnce(Return(1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_008";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_009
 * @tc.desc: Test function of CopyFileCore::OpenFile.TruncateCore interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_009";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = false;

    int srcfd = open("CopyFileCoreMockTest_DoCopyFile_009_src.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    int destfd = open("CopyFileCoreMockTest_DoCopyFile_009_dest.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcfd, -1);
    ASSERT_NE(destfd, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcfd);
    dest.fdg = make_unique<DistributedFS::FDGuard>(destfd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_ftruncate(_, _, _, _, _)).Times(1).WillOnce(Return(1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_009";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_0010
 * @tc.desc: Test function of CopyFileCore::OpenFile.SendFileCore interface for false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_0010";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;

    int srcfd = open("CopyFileCoreMockTest_DoCopyFile_0010.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcfd, -1);
    const char *data = "Hello, World!";
    ssize_t len = write(srcfd, data, strlen(data));
    ASSERT_NE(len, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcfd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(-1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_0010";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_0011
 * @tc.desc: Test function of CopyFileCore::OpenFile.SendFileCore interface for false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_0011";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;

    int srcfd = open("CopyFileCoreMockTest_DoCopyFile_0011.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcfd, -1);
    const char *data = "Hello, World!";
    ssize_t len = write(srcfd, data, strlen(data));
    ASSERT_NE(len, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcfd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(len + 1));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_0011";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_0012
 * @tc.desc: Test function of CopyFileCore::OpenFile.SendFileCore interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_0012";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;

    int srcfd = open("CopyFileCoreMockTest_DoCopyFile_0012.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcfd, -1);
    const char *data = "Hello, World!";
    ssize_t len = write(srcfd, data, strlen(data));
    ASSERT_NE(len, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcfd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(len));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_0012";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_0013
 * @tc.desc: Test function of CopyFileCore::OpenFile.SendFileCore interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_0013";

    FileInfo src;
    FileInfo dest;
    src.isPath = false;
    dest.isPath = true;

    int srcfd = open("CopyFileCoreMockTest_DoCopyFile_0013.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_NE(srcfd, -1);
    const char *data = "Hello, World!";
    ssize_t len = write(srcfd, data, strlen(data));
    ASSERT_NE(len, -1);
    src.fdg = make_unique<DistributedFS::FDGuard>(srcfd);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_open(_, _, _, _, _, _)).Times(1).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_sendfile(_, _, _, _, _, _, _)).Times(1).WillOnce(Return(0));

    auto res = CopyFileCore::DoCopyFile(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_0013";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test