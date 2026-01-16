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

#include <gtest/gtest.h>
#include <sys/prctl.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsRandomAccessFileTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsRandomAccessFileTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsRandomAccessFileTest");
}

void FsRandomAccessFileTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsRandomAccessFileTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsRandomAccessFileTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsRandomAccessFileTest_Constructor_001
 * @tc.desc: Test function of FsRandomAccessFile::Constructor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_Constructor_001";

    auto result = FsRandomAccessFile::Constructor();
    ASSERT_TRUE(result.IsSuccess());
    std::unique_ptr<FsRandomAccessFile> raf(result.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(raf, nullptr);

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_Constructor_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFD_001
 * @tc.desc: Test function of FsRandomAccessFile::GetFD interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFD_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_GetFD_001";

    auto expectedFd = 10;
    auto rafEntity = make_unique<RandomAccessFileEntity>();
    rafEntity->fd = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.GetFD();

    ASSERT_TRUE(result.IsSuccess());
    auto fd = result.GetData().value();
    EXPECT_EQ(fd, expectedFd);

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_GetFD_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFD_002
 * @tc.desc: Test function of FsRandomAccessFile::GetFD interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFD_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_GetFD_002";

    FsRandomAccessFile raf(nullptr);

    auto result = raf.GetFD();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_GetFD_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFPointer_001
 * @tc.desc: Test function of FsRandomAccessFile::GetFPointer interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFPointer_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_GetFPointer_001";

    auto expectedFilePointer = 10;
    auto rafEntity = make_unique<RandomAccessFileEntity>();
    rafEntity->filePointer = expectedFilePointer;
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.GetFPointer();

    ASSERT_TRUE(result.IsSuccess());
    auto filePointer = result.GetData().value();
    EXPECT_EQ(filePointer, expectedFilePointer);

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_GetFPointer_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_GetFPointer_002
 * @tc.desc: Test function of FsRandomAccessFile::GetFPointer interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_GetFPointer_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_GetFPointer_002";

    FsRandomAccessFile raf(nullptr);

    auto result = raf.GetFPointer();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_GetFPointer_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_SetFilePointerSync_001
 * @tc.desc: Test function of FsRandomAccessFile::SetFilePointerSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_SetFilePointerSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_SetFilePointerSync_001";

    auto expectedFilePointer = 10;
    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.SetFilePointerSync(expectedFilePointer);

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(raf.rafEntity->filePointer, expectedFilePointer);

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_SetFilePointerSync_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_SetFilePointerSync_002
 * @tc.desc: Test function of FsRandomAccessFile::SetFilePointerSync interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_SetFilePointerSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_SetFilePointerSync_002";

    FsRandomAccessFile raf(nullptr);

    auto result = raf.SetFilePointerSync(0);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_SetFilePointerSync_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_001
 * @tc.desc: Test function of FsRandomAccessFile::ReadSync interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_ReadSync_001";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    FsRandomAccessFile raf(nullptr);

    auto result = raf.ReadSync(buffer, nullopt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_ReadSync_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_002
 * @tc.desc: Test function of FsRandomAccessFile::ReadSync interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_ReadSync_002";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    ReadOptions options;
    options.offset = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.ReadSync(buffer, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_ReadSync_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_ReadSync_003
 * @tc.desc: Test function of FsRandomAccessFile::ReadSync interface for FAILURE when length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_ReadSync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_ReadSync_003";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    ReadOptions options;
    options.length = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.ReadSync(buffer, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_ReadSync_003";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_001
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(string) interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_001";

    string data = "test data";
    FsRandomAccessFile raf(nullptr);

    auto result = raf.WriteSync(data, nullopt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_001";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_002
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(ArrayBuffer) interface for FAILURE when rafEntity is
 * nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_002";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    FsRandomAccessFile raf(nullptr);

    auto result = raf.WriteSync(buffer, nullopt);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_002";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_003
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(string) interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_003";

    string data = "test data";
    WriteOptions options;
    options.offset = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.WriteSync(data, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_003";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_004
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(ArrayBuffer) interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_004";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    WriteOptions options;
    options.offset = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.WriteSync(buffer, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_004";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_005
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(string) interface for FAILURE when length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_005";

    string data = "test data";
    WriteOptions options;
    options.length = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.WriteSync(data, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_005";
}

/**
 * @tc.name: FsRandomAccessFileTest_WriteSync_006
 * @tc.desc: Test function of FsRandomAccessFile::WriteSync(ArrayBuffer) interface for FAILURE when length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_WriteSync_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_WriteSync_006";

    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    WriteOptions options;
    options.length = -1;

    auto rafEntity = make_unique<RandomAccessFileEntity>();
    FsRandomAccessFile raf(std::move(rafEntity));

    auto result = raf.WriteSync(buffer, options);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_WriteSync_006";
}

/**
 * @tc.name: FsRandomAccessFileTest_CloseSync_001
 * @tc.desc: Test function of FsRandomAccessFile::CloseSync interface for FAILURE when rafEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

 */
HWTEST_F(FsRandomAccessFileTest, FsRandomAccessFileTest_CloseSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-begin FsRandomAccessFileTest_CloseSync_001";

    FsRandomAccessFile raf(nullptr);

    auto result = raf.CloseSync();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsRandomAccessFileTest-end FsRandomAccessFileTest_CloseSync_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test