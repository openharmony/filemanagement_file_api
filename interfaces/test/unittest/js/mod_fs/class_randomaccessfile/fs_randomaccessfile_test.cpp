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
#include "file_entity.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsRandomAccessFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    std::unique_ptr<RandomAccessFileEntity> rafEntity;
    std::unique_ptr<FsRandomAccessFile> raf;
};

void FsRandomAccessFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsRandomAccessFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsRandomAccessFileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = std::make_unique<RandomAccessFileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    rafEntity->fd = std::make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity->filePointer = 0;
    raf = std::make_unique<FsRandomAccessFile>(std::move(rafEntity));
}

void FsRandomAccessFileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

// 测试Constructor
/**
 * @tc.name: FsRandomAccessFileTest_Constructor_001
 * @tc.desc: Test function of Constructor() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_Constructor_001";

    auto result = FsRandomAccessFile::Constructor();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_Constructor_001";
}

// 测试GetFD
/**
 * @tc.name: FsRandomAccessFileTest_GetFD_002
 * @tc.desc: Test function of GetFD() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFD_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_GetFD_002";

    auto result = raf->GetFD();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_GetFD_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFD_003
 * @tc.desc: Test function of GetFD() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFD_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_GetFD_003";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    auto result = raf->GetFD();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_GetFD_003";
}

// GetFPointer
/**
 * @tc.name: FsRandomAccessFileTest_GetFPointer_004
 * @tc.desc: Test function of GetFPointer() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFPointer_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_GetFPointer_004";

    raf->rafEntity->filePointer = 100;
    auto result = raf->GetFPointer();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_GetFPointer_004";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFPointer_005
 * @tc.desc: Test function of GetFPointer() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFPointer_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_GetFPointer_005";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    auto result = raf->GetFPointer();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_GetFPointer_005";
}

// SetFilePointerSync
/**
 * @tc.name: FsRandomAccessFileTest_SetFilePointerSync_006
 * @tc.desc: Test function of SetFilePointerSync() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_SetFilePointerSync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_SetFilePointerSync_006";

    auto result = raf->SetFilePointerSync(50);
    EXPECT_EQ(result.IsSuccess(), true);
    EXPECT_EQ(raf->rafEntity->filePointer, 50);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_SetFilePointerSync_006";
}

/**
 * @tc.name: FsRandomAccessFileTest_SetFilePointerSync_007
 * @tc.desc: Test function of SetFilePointerSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_SetFilePointerSync_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_SetFilePointerSync_007";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    auto result = raf->SetFilePointerSync(50);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_SetFilePointerSync_007";
}

// ReadSync
/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_008
 * @tc.desc: Test function of ReadSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_ReadSync_008";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    ArrayBuffer buffer(malloc(100), 100);

    auto result = raf->ReadSync(buffer, std::nullopt);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_ReadSync_008";
}

/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_009
 * @tc.desc: Test function of ReadSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_ReadSync_009";

    ArrayBuffer buffer(malloc(100), 100);
    ReadOptions options;
    options.offset = -5;
    
    auto result = raf->ReadSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_ReadSync_009";
}

/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_010
 * @tc.desc: Test function of ReadSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_ReadSync_010";

    ArrayBuffer buffer(malloc(100), 100);
    ReadOptions options;
    options.length = -1;
    
    auto result = raf->ReadSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_ReadSync_010";
}

// WriteSync
/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_011
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_011";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    std::string data = "test data";
    auto result = raf->WriteSync(data, std::nullopt);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_011";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_012
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_012";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    ArrayBuffer buffer(malloc(100), 100);
    auto result = raf->WriteSync(buffer, std::nullopt);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_012";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_013
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_013";

    std::string data = "test data";
    WriteOptions options;
    options.offset = -5;

    auto result = raf->WriteSync(data, options);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_013";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_014
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_014";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.offset = -5;

    auto result = raf->WriteSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_014";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_015
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_015";

    std::string data = "test data";
    WriteOptions options;
    options.length = -5;

    auto result = raf->WriteSync(data, options);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_015";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_016
 * @tc.desc: Test function of WriteSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_WriteSync_016";

    ArrayBuffer buffer(malloc(100), 100);
    WriteOptions options;
    options.length = -5;

    auto result = raf->WriteSync(buffer, options);
    EXPECT_EQ(result.IsSuccess(), false);
    free(buffer.buf);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_WriteSync_016";
}

// CloseSync
/**
 * @tc.name: FsRandomAccessFileTest_CloseSync_017
 * @tc.desc: Test function of CloseSync() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_CloseSync_017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-begin FsRandomAccessFileTest_CloseSync_017";

    raf = std::make_unique<FsRandomAccessFile>(nullptr);
    auto result = raf->CloseSync();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsRandomAccessFileMockTest-end FsRandomAccessFileTest_CloseSync_017";
}

}