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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "file_entity.h"
#include "fs_randomaccessfile.h"
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
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
protected:
unique_ptr<RandomAccessFileEntity> rafEntity;
    unique_ptr<FsRandomAccessFile> raf;
};

void FsRandomAccessFileMockTest::SetUpTestCase(void)
{
    uvMock = make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsRandomAccessFileMockTest::TearDownTestCase(void)
{
    Uvfs::ins = nullptr;
    uvMock = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsRandomAccessFileMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = make_unique<RandomAccessFileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    rafEntity->fd = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity->filePointer = 0;
    raf = make_unique<FsRandomAccessFile>(move(rafEntity));
}

void FsRandomAccessFileMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_ReadSync_001
 * @tc.desc: Test function of ReadSync() interface for is failed for options is nullopt.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_001";

    ArrayBuffer buffer(malloc(100), 100);
    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(-1));
    auto result = raf->ReadSync(buffer, nullopt);
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
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_ReadSync_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_ReadSync_002";

    ArrayBuffer buffer(malloc(100), 100);
    ReadOptions options;
    options.offset = 10;
    options.length = 10;
    raf->rafEntity->filePointer = 20;

    EXPECT_CALL(*uvMock, uv_fs_read(_, _, _, _, _, _, _)).WillOnce(Return(0));
    auto result = raf->ReadSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), true);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_ReadSync_002";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_003
 * @tc.desc: Test function of WriteSync() interface for is failed for options is nullopt.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_003";

    string data = "test data";
    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));
    auto result = raf->WriteSync(data, nullopt);
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
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_004";

    string data = "test data";
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));
    auto result = raf->WriteSync(data, options);
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_004";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_WriteSync_005
 * @tc.desc: Test function of WriteSync() interface for is failed for uv_fs_write() return -1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_005";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));
    auto result = raf->WriteSync(buffer, options);
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
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_WriteSync_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_WriteSync_006";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.length = 4;
    options.offset = 0;

    EXPECT_CALL(*uvMock, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(0));
    auto result = raf->WriteSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), true);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_WriteSync_006";
}

/**
 * @tc.name: FsRandomAccessFileMockTest_CloseSync_007
 * @tc.desc: Test function of CloseSync() interface for is failed for uv_fs_write() return -1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 
*/
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_007";

    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(-1));
    auto result = raf->CloseSync();
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
HWTEST_F(FsRandomAccessFileMockTest, FsRandomAccessFileMockTest_CloseSync_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileMockTest_CloseSync_008";

    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
    auto result = raf->CloseSync();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileMockTest_CloseSync_008";
}

}