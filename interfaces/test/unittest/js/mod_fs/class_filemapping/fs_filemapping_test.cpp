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

#include "fs_filemapping.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "mmap_core.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsFileMappingTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

protected:
    static void CreateTestMapping();
    static void CleanupTestMapping();
    
    static FsFileMapping *testMapping_;
    static int testFd_;

private:
    const string testDir = FileUtils::testRootDir + "/FsFileMappingTest";
};

FsFileMapping *FsFileMappingTest::testMapping_ = nullptr;
int FsFileMappingTest::testFd_ = -1;

void FsFileMappingTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsFileMappingTest");
}

void FsFileMappingTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsFileMappingTest::CreateTestMapping()
{
    if (testMapping_ != nullptr) {
        return;
    }
    
    string testDir = FileUtils::testRootDir + "/FsFileMappingTest_Static";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    
    string content = "Test content for file mapping operations";
    string filepath = testDir + "/test_mapping.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);
    testFd_ = fd;
    
    auto result = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, content.size());
    ASSERT_TRUE(result.IsSuccess());
    testMapping_ = result.GetData().value();
}

void FsFileMappingTest::CleanupTestMapping()
{
    if (testMapping_ != nullptr) {
        delete testMapping_;
        testMapping_ = nullptr;
    }
    if (testFd_ >= 0) {
        close(testFd_);
        testFd_ = -1;
    }
}

void FsFileMappingTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    CreateTestMapping();
}

void FsFileMappingTest::TearDown()
{
    CleanupTestMapping();
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileMappingTest_CheckValid_001
 * @tc.desc: Test function of FsFileMapping::CheckValid interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_CheckValid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_CheckValid_001";

    ASSERT_NE(testMapping_, nullptr);
    EXPECT_TRUE(testMapping_->CheckValid());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_CheckValid_001";
}

/**
 * @tc.name: FsFileMappingTest_IsReadOnly_001
 * @tc.desc: Test function of FsFileMapping::IsReadOnly interface for READ_WRITE mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_IsReadOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_IsReadOnly_001";

    ASSERT_NE(testMapping_, nullptr);
    EXPECT_FALSE(testMapping_->IsReadOnly());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_IsReadOnly_001";
}

/**
 * @tc.name: FsFileMappingTest_SetPosition_001
 * @tc.desc: Test function of FsFileMapping::SetPosition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_SetPosition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_SetPosition_001";

    ASSERT_NE(testMapping_, nullptr);
    size_t newPos = 10;
    auto result = testMapping_->SetPosition(newPos);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(testMapping_->GetEntity()->position, newPos);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_SetPosition_001";
}

/**
 * @tc.name: FsFileMappingTest_SetPosition_002
 * @tc.desc: Test function of FsFileMapping::SetPosition interface for FAILURE when position > limit.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_SetPosition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_SetPosition_002";

    ASSERT_NE(testMapping_, nullptr);
    size_t invalidPos = testMapping_->GetEntity()->limit + 100;
    auto result = testMapping_->SetPosition(invalidPos);
    
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_SetPosition_002";
}

/**
 * @tc.name: FsFileMappingTest_GetPosition_001
 * @tc.desc: Test function of FsFileMapping::GetPosition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_GetPosition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_GetPosition_001";

    ASSERT_NE(testMapping_, nullptr);
    auto result = testMapping_->GetPosition();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), testMapping_->GetEntity()->position);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_GetPosition_001";
}

/**
 * @tc.name: FsFileMappingTest_Capacity_001
 * @tc.desc: Test function of FsFileMapping::Capacity interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Capacity_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Capacity_001";

    ASSERT_NE(testMapping_, nullptr);
    auto result = testMapping_->Capacity();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), testMapping_->GetEntity()->capacity);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Capacity_001";
}

/**
 * @tc.name: FsFileMappingTest_SetLimit_001
 * @tc.desc: Test function of FsFileMapping::SetLimit interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_SetLimit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_SetLimit_001";

    ASSERT_NE(testMapping_, nullptr);
    size_t newLimit = testMapping_->GetEntity()->capacity / 2;
    auto result = testMapping_->SetLimit(newLimit);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(testMapping_->GetEntity()->limit, newLimit);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_SetLimit_001";
}

/**
 * @tc.name: FsFileMappingTest_SetLimit_002
 * @tc.desc: Test function of FsFileMapping::SetLimit interface for FAILURE when limit > capacity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_SetLimit_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_SetLimit_002";

    ASSERT_NE(testMapping_, nullptr);
    size_t invalidLimit = testMapping_->GetEntity()->capacity + 100;
    auto result = testMapping_->SetLimit(invalidLimit);
    
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_SetLimit_002";
}

/**
 * @tc.name: FsFileMappingTest_SetLimit_003
 * @tc.desc: Test function of FsFileMapping::SetLimit interface for SUCCESS when position > new limit.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_SetLimit_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_SetLimit_003";

    ASSERT_NE(testMapping_, nullptr);
    testMapping_->SetPosition(20);
    size_t newLimit = 10;
    auto result = testMapping_->SetLimit(newLimit);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(testMapping_->GetEntity()->limit, newLimit);
    EXPECT_EQ(testMapping_->GetEntity()->position, newLimit);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_SetLimit_003";
}

/**
 * @tc.name: FsFileMappingTest_GetLimit_001
 * @tc.desc: Test function of FsFileMapping::GetLimit interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_GetLimit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_GetLimit_001";

    ASSERT_NE(testMapping_, nullptr);
    auto result = testMapping_->GetLimit();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), testMapping_->GetEntity()->limit);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_GetLimit_001";
}

/**
 * @tc.name: FsFileMappingTest_Flip_001
 * @tc.desc: Test function of FsFileMapping::Flip interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Flip_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Flip_001";

    ASSERT_NE(testMapping_, nullptr);
    size_t oldPosition = 15;
    testMapping_->SetPosition(oldPosition);
    
    auto result = testMapping_->Flip();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(testMapping_->GetEntity()->limit, oldPosition);
    EXPECT_EQ(testMapping_->GetEntity()->position, 0);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Flip_001";
}

/**
 * @tc.name: FsFileMappingTest_Remaining_001
 * @tc.desc: Test function of FsFileMapping::Remaining interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Remaining_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Remaining_001";

    ASSERT_NE(testMapping_, nullptr);
    testMapping_->SetPosition(5);
    testMapping_->SetLimit(20);
    
    auto result = testMapping_->Remaining();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), 15);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Remaining_001";
}

/**
 * @tc.name: FsFileMappingTest_Read_001
 * @tc.desc: Test function of FsFileMapping::Read interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Read_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Read_001";

    ASSERT_NE(testMapping_, nullptr);
    testMapping_->SetPosition(0);
    
    char buffer[100] = {0};
    auto result = testMapping_->Read(buffer, sizeof(buffer), 10);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_GT(result.GetData().value(), 0);
    EXPECT_EQ(testMapping_->GetEntity()->position, result.GetData().value());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Read_001";
}

/**
 * @tc.name: FsFileMappingTest_Read_002
 * @tc.desc: Test function of FsFileMapping::Read interface for FAILURE when buffer is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Read_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Read_002";

    ASSERT_NE(testMapping_, nullptr);
    
    auto result = testMapping_->Read(nullptr, 100, 10);
    
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Read_002";
}

/**
 * @tc.name: FsFileMappingTest_ReadFrom_001
 * @tc.desc: Test function of FsFileMapping::ReadFrom interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_ReadFrom_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_ReadFrom_001";

    ASSERT_NE(testMapping_, nullptr);
    
    char buffer[100] = {0};
    auto result = testMapping_->ReadFrom(0, buffer, sizeof(buffer), 10);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_GT(result.GetData().value(), 0);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_ReadFrom_001";
}

/**
 * @tc.name: FsFileMappingTest_Write_001
 * @tc.desc: Test function of FsFileMapping::Write interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Write_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Write_001";

    ASSERT_NE(testMapping_, nullptr);
    testMapping_->SetPosition(0);
    
    const char *data = "Hello";
    size_t dataLen = strlen(data);
    auto result = testMapping_->Write(data, dataLen, dataLen);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), dataLen);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Write_001";
}

/**
 * @tc.name: FsFileMappingTest_Write_002
 * @tc.desc: Test function of FsFileMapping::Write interface for FAILURE when data is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Write_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Write_002";

    ASSERT_NE(testMapping_, nullptr);
    
    auto result = testMapping_->Write(nullptr, 100, 10);
    
    EXPECT_FALSE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Write_002";
}

/**
 * @tc.name: FsFileMappingTest_WriteTo_001
 * @tc.desc: Test function of FsFileMapping::WriteTo interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_WriteTo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_WriteTo_001";

    ASSERT_NE(testMapping_, nullptr);
    
    const char *data = "Test";
    size_t dataLen = strlen(data);
    auto result = testMapping_->WriteTo(0, data, dataLen, dataLen);
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(result.GetData().value(), dataLen);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_WriteTo_001";
}

/**
 * @tc.name: FsFileMappingTest_Msync_001
 * @tc.desc: Test function of FsFileMapping::Msync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Msync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Msync_001";

    ASSERT_NE(testMapping_, nullptr);
    
    auto result = testMapping_->Msync(0, testMapping_->GetEntity()->capacity);
    
    EXPECT_TRUE(result.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Msync_001";
}

/**
 * @tc.name: FsFileMappingTest_Unmap_001
 * @tc.desc: Test function of FsFileMapping::Unmap interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Unmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Unmap_001";

    string content = "Test content";
    string filepath = testDir + "/test_unmap.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    auto mapResult = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, content.size());
    ASSERT_TRUE(mapResult.IsSuccess());
    auto mapping = mapResult.GetData().value();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Unmap();
    
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(mapping->CheckValid());

    delete mapping;
    close(fd);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Unmap_001";
}

/**
 * @tc.name: FsFileMappingTest_Unmap_002
 * @tc.desc: Test function of FsFileMapping::Unmap interface for idempotent behavior.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Unmap_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Unmap_002";

    string content = "Test content";
    string filepath = testDir + "/test_unmap2.txt";
    auto [fd, success] = FileUtils::CreateFileWithContent(filepath, content);
    ASSERT_TRUE(success);

    auto mapResult = MmapCore::DoMmap(fd, MappingMode::READ_WRITE, 0, content.size());
    ASSERT_TRUE(mapResult.IsSuccess());
    auto mapping = mapResult.GetData().value();
    ASSERT_NE(mapping, nullptr);

    auto result1 = mapping->Unmap();
    EXPECT_TRUE(result1.IsSuccess());
    
    auto result2 = mapping->Unmap();
    EXPECT_TRUE(result2.IsSuccess());

    delete mapping;
    close(fd);

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Unmap_002";
}

/**
 * @tc.name: FsFileMappingTest_Constructor_001
 * @tc.desc: Test function of FsFileMapping::Constructor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingTest, FsFileMappingTest_Constructor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingTest-begin FsFileMappingTest_Constructor_001";

    char mockBuffer[1024] = {0};
    struct FileMappingParams params;
    params.mapAddr = mockBuffer + 100;
    params.rawMapAddr = mockBuffer;
    params.capacity = 924;
    params.rawCapacity = 1024;
    params.adjustment = 100;
    params.mode = MappingMode::READ_WRITE;
    params.fd = 10;
    params.offset = 0;

    auto result = FsFileMapping::Constructor(params);
    
    ASSERT_TRUE(result.IsSuccess());
    auto mapping = result.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_EQ(mapping->GetEntity()->capacity, params.capacity);
    EXPECT_EQ(mapping->GetEntity()->rawCapacity, params.rawCapacity);
    EXPECT_EQ(mapping->GetEntity()->adjustment, params.adjustment);
    EXPECT_EQ(mapping->GetEntity()->mode, params.mode);

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingTest-end FsFileMappingTest_Constructor_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
