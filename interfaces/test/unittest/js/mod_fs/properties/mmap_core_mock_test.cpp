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

#include "mmap_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "mmap_mock.h"
#include "sys_file_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MmapCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void MmapCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MmapCoreMockTest");
    MmapMock::EnableMock();
}

void MmapCoreMockTest::TearDownTestSuite()
{
    MmapMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MmapCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MmapCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_001
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when fstat fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_001";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, fstat(fd, _)).WillOnce(Return(-1));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_001";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_002
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when file type is not regular/block.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_002";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillOnce(Invoke([](int fd, struct stat* st) {
            st->st_mode = S_IFDIR;
            return 0;
        }));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_002";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_003
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when fstatfs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_003";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _)).WillOnce(Return(-1));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_003";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_004
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when filesystem type is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_004";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0x12345678;
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_004";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_005
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when sysconf fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_005";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xEF53;
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(-1));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_005";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_006
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when ftruncate fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_006";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    mockStat.st_size = 100;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xEF53;
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, ftruncate(fd, _))
        .WillOnce(Invoke([](int fd, off_t length) {
            errno = EINVAL;
            return -1;
        }));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, 4096);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_006";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_007
 * @tc.desc: Test function of MmapCore::DoMmap interface for FAILURE when mmap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_007";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    mockStat.st_size = 4096;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xEF53;
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, mmap(_, _, _, _, _, _)).WillOnce(Return(MAP_FAILED));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_007";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_008
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with EXT4 filesystem.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_008";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    mockStat.st_size = 4096;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xEF53;
    char mockBuffer[4096] = {0};
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, mmap(_, _, _, _, _, _)).WillOnce(Return(mockBuffer));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    delete mapping;

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_008";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_009
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with F2FS filesystem.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_009";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    mockStat.st_size = 4096;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xF2F52010;
    char mockBuffer[4096] = {0};
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, mmap(_, _, _, _, _, _)).WillOnce(Return(mockBuffer));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    delete mapping;

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_009";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_010
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with HMDFS_LOCAL filesystem.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_010";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFREG;
    mockStat.st_size = 4096;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0x20200302;
    char mockBuffer[4096] = {0};
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, mmap(_, _, _, _, _, _)).WillOnce(Return(mockBuffer));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    delete mapping;

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_010";
}

/**
 * @tc.name: MmapCoreMockTest_DoMmap_011
 * @tc.desc: Test function of MmapCore::DoMmap interface for SUCCESS with block device.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MmapCoreMockTest, MmapCoreMockTest_DoMmap_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MmapCoreMockTest-begin MmapCoreMockTest_DoMmap_011";

    int fd = 10;
    auto mmapMock = MmapMock::GetMock();
    struct stat mockStat = {0};
    mockStat.st_mode = S_IFBLK;
    mockStat.st_size = 4096;
    struct statfs mockFsInfo = {0};
    mockFsInfo.f_type = 0xEF53;
    char mockBuffer[4096] = {0};
    EXPECT_CALL(*mmapMock, fstat(fd, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mockStat), Return(0)));
    EXPECT_CALL(*mmapMock, fstatfs(fd, _))
        .WillOnce(DoAll(SetArgPointee<1>(mockFsInfo), Return(0)));
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, mmap(_, _, _, _, _, _)).WillOnce(Return(mockBuffer));

    auto result = MmapCore::DoMmap(fd, MappingMode::READ_ONLY, 0, 1024);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    delete mapping;

    GTEST_LOG_(INFO) << "MmapCoreMockTest-end MmapCoreMockTest_DoMmap_011";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
