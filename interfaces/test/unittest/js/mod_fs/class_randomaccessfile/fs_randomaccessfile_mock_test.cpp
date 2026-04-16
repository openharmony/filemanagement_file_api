/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "fs_randomaccessfile.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fdtag_func.h"
#include "fdsan_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsRandomAccessFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

protected:
    unique_ptr<RandomAccessFileEntity> rafEntity;
    unique_ptr<FsRandomAccessFile> raf;
};

void FsRandomAccessFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsRandomAccessFileMockTest");
    FdsanMock::EnableMock();
    UvFsMock::EnableMock();
}

void FsRandomAccessFileMockTest::TearDownTestSuite()
{
    FdsanMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsRandomAccessFileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = make_unique<RandomAccessFileEntity>();
    rafEntity->fd = make_unique<DistributedFS::FDGuard>(1, false);
    rafEntity->filePointer = 0;
    raf = make_unique<FsRandomAccessFile>(std::move(rafEntity));
}

void FsRandomAccessFileMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t MAX_FD = 2048;
/**
 * @tc.name: FsRandomAccessFileMockTest_ReadSync_001
 * @tc.desc: Test function of FsRandomAccessFile::ReadSync interface for FAILURE when nullopt and uv_fs_read fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_001";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->ReadSync(buffer, std::nullopt);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_ReadSync_001";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_ReadSync_002
 * @tc.desc: Test function of FsRandomAccessFile::ReadSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_002";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    ReadOptions options;
    options.offset = 1;
    options.length = 4;
    auto filePointer = 20;
    raf->rafEntity->filePointer = filePointer;
    auto expectedLen = options.length.value();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(expectedLen));

    auto result = raf->ReadSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto readLen = result.GetData().value();
    EXPECT_EQ(readLen, expectedLen);
    auto newFilePointer = filePointer + options.offset.value() + expectedLen;
    EXPECT_EQ(raf->rafEntity->filePointer, newFilePointer);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_ReadSync_002";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_003
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(string) interface for FAILURE when uv_fs_write fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_003";

    std::string data = "test data";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->WriteSync(data, std::nullopt);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_003";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_004
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(string) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_004";

    string data = "test data";
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto expectedLen = options.length.value();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(expectedLen));

    auto result = raf->WriteSync(data, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto writeLen = result.GetData().value();
    EXPECT_EQ(writeLen, expectedLen);
    auto newFilePointer = options.offset.value() + expectedLen;
    EXPECT_EQ(raf->rafEntity->filePointer, newFilePointer);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_004";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_005
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(ArrayBuffer) interface for FAILURE when uv_fs_write fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_005";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->WriteSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_005";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_006
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(ArrayBuffer) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_006";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto expectedLen = options.length.value();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(expectedLen));

    auto result = raf->WriteSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(result.IsSuccess());
    auto writeLen = result.GetData().value();
    EXPECT_EQ(writeLen, expectedLen);
    auto newFilePointer = options.offset.value() + expectedLen;
    EXPECT_EQ(raf->rafEntity->filePointer, newFilePointer);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_006";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_007
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for FAILURE when uv_fs_close fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_007";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(-1));
    raf->rafEntity->fd = make_unique<DistributedFS::FDGuard>(2048, false);

    auto result = raf->CloseSync();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_007";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_008
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_008";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
    raf->rafEntity->fd = make_unique<DistributedFS::FDGuard>(2048, false);
    auto result = raf->CloseSync();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_008";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_009
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for SUCCESS
 *           when fd < FD_SAN_OVERFLOW_END (fdsan path on non-Win/iOS/CROSS).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_009";
    raf->rafEntity->fd = make_unique<DistributedFS::FDGuard>(3, false);
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_)).WillOnce(testing::Return(0));
#else
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
#endif
    auto result = raf->CloseSync();
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    testing::Mock::VerifyAndClearExpectations(fdsanMock.get());
#else
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
#endif
    EXPECT_TRUE(result.IsSuccess());
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_009";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_010
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for SUCCESS
 *           when fd >= FD_SAN_OVERFLOW_END (always uv_fs_close path).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_010";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
    raf->rafEntity->fd = make_unique<DistributedFS::FDGuard>(FD_SAN_OVERFLOW_END, false);
    auto result = raf->CloseSync();
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(result.IsSuccess());
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_010";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_011
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_011";
 
    auto fdsanMock = FdsanMock::GetMock();
    EXPECT_CALL(*fdsanMock, fdsan_close_with_tag(testing::_, testing::_))
        .WillOnce(testing::SetErrnoAndReturn(EBADFD, -1));
    raf->rafEntity->fd = make_unique<DistributedFS::FDGuard>(1, false);
    auto result = raf->CloseSync();

    EXPECT_FALSE(result.IsSuccess());
 
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_011";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test