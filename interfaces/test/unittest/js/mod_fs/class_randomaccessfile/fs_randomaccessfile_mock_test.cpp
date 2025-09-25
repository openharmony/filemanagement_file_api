/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "file_entity.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsRandomAccessFileMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::unique_ptr<RandomAccessFileEntity> rafEntity;
    std::unique_ptr<FsRandomAccessFile> raf;
};

void FsRandomAccessFileMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "FsRandomAccessFileMockTest");
    UvFsMock::EnableMock();
}

void FsRandomAccessFileMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsRandomAccessFileMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = std::make_unique<RandomAccessFileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    rafEntity->fd = std::make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity->filePointer = 0;
    raf = std::make_unique<FsRandomAccessFile>(std::move(rafEntity));
}

void FsRandomAccessFileMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_ReadSync_001
 * @tc.desc: Test function of ReadSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_001";

    ArrayBuffer buffer(malloc(100), 100);
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->ReadSync(buffer, std::nullopt);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_ReadSync_001";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_ReadSync_002
 * @tc.desc: Test function of ReadSync() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_002";

    ArrayBuffer buffer(malloc(100), 100);
    ReadOptions options;
    options.offset = 10;
    options.length = 10;
    raf->rafEntity->filePointer = 20;
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto result = raf->ReadSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_ReadSync_002";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_003
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_003";

    std::string data = "test data";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->WriteSync(data, std::nullopt);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_003";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_004
 * @tc.desc: Test function of WriteSync() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_004";

    std::string data = "test data";
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto result = raf->WriteSync(data, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_004";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_005
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_005";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto result = raf->WriteSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_005";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_006
 * @tc.desc: Test function of WriteSync() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_006";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));

    auto result = raf->WriteSync(buffer, options);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_006";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_007
 * @tc.desc: Test function of CloseSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_007";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(-1));

    auto result = raf->CloseSync();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_007";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_008
 * @tc.desc: Test function of CloseSync() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_008";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));

    auto result = raf->CloseSync();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_008";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test