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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "mmap_mock.h"
#include "securec.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

#define BUFFER_LENGTH 1024
#define CAPACITY_LENGTH 924
#define ADJUSTMENT_LENGTH (BUFFER_LENGTH - CAPACITY_LENGTH)
#define MOCK_FD 10

class FsFileMappingMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

protected:
    static FsFileMapping *CreateTestMapping();
    static FsFileMapping *DoUnmapAfterCreate();
};

void FsFileMappingMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsFileMappingMockTest");
    MmapMock::EnableMock();
}

void FsFileMappingMockTest::TearDownTestSuite()
{
    MmapMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsFileMappingMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsFileMappingMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

FsFileMapping *FsFileMappingMockTest::CreateTestMapping()
{
    char *mockBuffer = new char[BUFFER_LENGTH];
    memset_s(mockBuffer, BUFFER_LENGTH, 0, BUFFER_LENGTH);
    struct FileMappingParams params;
    params.mapAddr = mockBuffer + ADJUSTMENT_LENGTH;
    params.rawMapAddr = mockBuffer;
    params.capacity = CAPACITY_LENGTH;
    params.rawCapacity = BUFFER_LENGTH;
    params.adjustment = ADJUSTMENT_LENGTH;
    params.mode = MappingMode::READ_WRITE;
    params.fd = MOCK_FD;
    params.offset = 0;

    auto result = FsFileMapping::Constructor(params);
    if (result.IsSuccess()) {
        return result.GetData().value();
    }
    return nullptr;
}

FsFileMapping *FsFileMappingMockTest::DoUnmapAfterCreate()
{
    auto mapping = CreateTestMapping();
    if (!mapping) {
        return nullptr;
    }

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, munmap(_, _)).WillOnce(Return(0));

    auto unmapResult = mapping->Unmap();
    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_TRUE(unmapResult.IsSuccess());

    return mapping;
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_001
 * @tc.desc: Test function of FsFileMapping::Msync interface for FAILURE when msync fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_001";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, msync(_, _, _)).WillOnce(Return(-1));

    auto result = mapping->Msync(0, mapping->GetEntity()->capacity);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_002
 * @tc.desc: Test function of FsFileMapping::Msync interface for FAILURE when sysconf fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_002";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(-1));

    auto result = mapping->Msync(0, mapping->GetEntity()->capacity);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_002";
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_003
 * @tc.desc: Test function of FsFileMapping::Msync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_003";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, sysconf(_SC_PAGESIZE)).WillOnce(Return(4096));
    EXPECT_CALL(*mmapMock, msync(_, _, _)).WillOnce(Return(0));

    auto result = mapping->Msync(0, mapping->GetEntity()->capacity);

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_TRUE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_003";
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_004
 * @tc.desc: Test function of FsFileMapping::Msync interface for FAILURE when offset overflow.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_004";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Msync(SIZE_MAX, 100);

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_004";
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_005
 * @tc.desc: Test function of FsFileMapping::Msync interface for FAILURE when offset exceeds raw capacity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_005";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Msync(mapping->GetEntity()->rawCapacity + 100, 100);

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_005";
}

/**
 * @tc.name: FsFileMappingMockTest_Unmap_001
 * @tc.desc: Test function of FsFileMapping::Unmap interface for FAILURE when munmap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Unmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Unmap_001";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, munmap(_, _)).WillOnce(Return(-1));

    auto result = mapping->Unmap();

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Unmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Unmap_002
 * @tc.desc: Test function of FsFileMapping::Unmap interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Unmap_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Unmap_002";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    auto mmapMock = MmapMock::GetMock();
    EXPECT_CALL(*mmapMock, munmap(_, _)).WillOnce(Return(0));

    auto result = mapping->Unmap();

    testing::Mock::VerifyAndClearExpectations(mmapMock.get());
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(mapping->CheckValid());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Unmap_002";
}

/**
 * @tc.name: FsFileMappingMockTest_Write_ReadOnly_001
 * @tc.desc: Test function of FsFileMapping::Write interface for FAILURE when mapping is read-only.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Write_ReadOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Write_ReadOnly_001";

    char mockBuffer[BUFFER_LENGTH] = {0};
    struct FileMappingParams params;
    params.mapAddr = mockBuffer + 100;
    params.rawMapAddr = mockBuffer;
    params.capacity = CAPACITY_LENGTH;
    params.rawCapacity = BUFFER_LENGTH;
    params.adjustment = 100;
    params.mode = MappingMode::READ_ONLY;
    params.fd = 10;
    params.offset = 0;

    auto constructorResult = FsFileMapping::Constructor(params);
    ASSERT_TRUE(constructorResult.IsSuccess());
    auto mapping = constructorResult.GetData().value();
    ASSERT_NE(mapping, nullptr);
    EXPECT_TRUE(mapping->IsReadOnly());

    const char *data = "test";
    auto result = mapping->Write(data, strlen(data), strlen(data));

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Write_ReadOnly_001";
}

/**
 * @tc.name: FsFileMappingMockTest_WriteTo_ReadOnly_001
 * @tc.desc: Test function of FsFileMapping::WriteTo interface for FAILURE when mapping is read-only.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_WriteTo_ReadOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_WriteTo_ReadOnly_001";

    char mockBuffer[BUFFER_LENGTH] = {0};
    struct FileMappingParams params;
    params.mapAddr = mockBuffer + 100;
    params.rawMapAddr = mockBuffer;
    params.capacity = CAPACITY_LENGTH;
    params.rawCapacity = BUFFER_LENGTH;
    params.adjustment = 100;
    params.mode = MappingMode::READ_ONLY;
    params.fd = 10;
    params.offset = 0;

    auto constructorResult = FsFileMapping::Constructor(params);
    ASSERT_TRUE(constructorResult.IsSuccess());
    auto mapping = constructorResult.GetData().value();
    ASSERT_NE(mapping, nullptr);

    const char *data = "test";
    auto result = mapping->WriteTo(0, data, strlen(data), strlen(data));

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_WriteTo_ReadOnly_001";
}

/**
 * @tc.name: FsFileMappingMockTest_ReadFrom_InvalidPosition_001
 * @tc.desc: Test function of FsFileMapping::ReadFrom interface for FAILURE when position > capacity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_ReadFrom_InvalidPosition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_ReadFrom_InvalidPosition_001";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    char buffer[100] = {0};
    auto result = mapping->ReadFrom(mapping->GetEntity()->capacity + 100, buffer, sizeof(buffer), 10);

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_ReadFrom_InvalidPosition_001";
}

/**
 * @tc.name: FsFileMappingMockTest_WriteTo_InvalidPosition_001
 * @tc.desc: Test function of FsFileMapping::WriteTo interface for FAILURE when position > capacity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_WriteTo_InvalidPosition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_WriteTo_InvalidPosition_001";

    auto mapping = CreateTestMapping();
    ASSERT_NE(mapping, nullptr);

    const char *data = "test";
    auto result = mapping->WriteTo(mapping->GetEntity()->capacity + 100, data, strlen(data), strlen(data));

    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_WriteTo_InvalidPosition_001";
}

/**
 * @tc.name: FsFileMappingMockTest_SetPosition_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::SetPosition interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_SetPosition_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_SetPosition_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->SetPosition(10);
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_SetPosition_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_GetPosition_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::GetPosition interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_GetPosition_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_GetPosition_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->GetPosition();
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_GetPosition_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Capacity_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Capacity interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Capacity_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Capacity_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Capacity();
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Capacity_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_SetLimit_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::SetLimit interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_SetLimit_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_SetLimit_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->SetLimit(100);
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_SetLimit_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_GetLimit_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::GetLimit interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_GetLimit_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_GetLimit_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->GetLimit();
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_GetLimit_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Flip_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Flip interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Flip_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Flip_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Flip();
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Flip_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Remaining_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Remaining interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Remaining_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Remaining_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Remaining();
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Remaining_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Read_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Read interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Read_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Read_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    char buffer[100] = {0};
    auto result = mapping->Read(buffer, sizeof(buffer), 10);
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Read_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Write_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Write interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Write_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Write_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    const char *data = "test";
    auto result = mapping->Write(data, strlen(data), strlen(data));
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Write_AfterUnmap_001";
}

/**
 * @tc.name: FsFileMappingMockTest_Msync_AfterUnmap_001
 * @tc.desc: Test function of FsFileMapping::Msync interface for FAILURE after unmap.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMappingMockTest, FsFileMappingMockTest_Msync_AfterUnmap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMappingMockTest-begin FsFileMappingMockTest_Msync_AfterUnmap_001";

    auto mapping = DoUnmapAfterCreate();
    ASSERT_NE(mapping, nullptr);

    auto result = mapping->Msync(0, 100);
    EXPECT_FALSE(result.IsSuccess());

    delete mapping;

    GTEST_LOG_(INFO) << "FsFileMappingMockTest-end FsFileMappingMockTest_Msync_AfterUnmap_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
