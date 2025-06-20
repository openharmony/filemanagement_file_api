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

#include "file_entity.h"
#include "fs_file.h"

#include <gtest/gtest.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::unique_ptr<FileEntity> fileEntity;
    std::unique_ptr<FsFile> fsFile;
};

void FsFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsFileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    
    fileEntity = std::make_unique<FileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    fileEntity->fd_ = std::make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    fileEntity->path_ = "/data/test/file_test.txt";
    fileEntity->uri_ = "";
    fsFile = std::make_unique<FsFile>(std::move(fileEntity));
}

void FsFileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileTest_Constructor_001
 * @tc.desc: Test function of FsFile::Constructor() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_Constructor_001";

    auto result = FsFile::Constructor();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_Constructor_001";
}

/**
 * @tc.name: FsFileTest_GetFD_002
 * @tc.desc: Test function of GetFD() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetFD_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetFD_002";

    auto result = fsFile->GetFD();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetFD_002";
}

/**
 * @tc.name: FsFileTest_GetFD_003
 * @tc.desc: Test function of GetFD() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetFD_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetFD_003";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->GetFD();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetFD_003";
}

/**
 * @tc.name: FsFileTest_GetPath_004
 * @tc.desc: Test function of GetPath() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetPath_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetPath_004";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->GetPath();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetPath_004";
}

/**
 * @tc.name: FsFileTest_GetName_005
 * @tc.desc: Test function of GetName() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetName_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetName_005";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->GetName();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetName_005";
}

/**
 * @tc.name: FsFileTest_GetParent_006
 * @tc.desc: Test function of GetParent() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetParent_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetParent_006";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->GetParent();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetParent_006";
}

/**
 * @tc.name: FsFileTest_Lock_007
 * @tc.desc: Test function of Lock() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_Lock_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_Lock_007";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->Lock(true);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_Lock_007";
}

/**
 * @tc.name: FsFileTest_TryLock_008
 * @tc.desc: Test function of TryLock() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_TryLock_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_TryLock_008";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->TryLock(true);
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_TryLock_008";
}

/**
 * @tc.name: FsFileTest_UnLock_009
 * @tc.desc: Test function of UnLock() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_UnLock_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_UnLock_009";

    fsFile = std::make_unique<FsFile>(nullptr);
    auto result = fsFile->UnLock();
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_UnLock_009";
}

/**
 * @tc.name: FsFileTest_GetName_010
 * @tc.desc: Test function of GetName() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetName_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetName_010";

    fsFile->fileEntity->uri_ = "file://storage/file_test.txt";
    auto result = fsFile->GetName();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetName_010";
}

/**
 * @tc.name: FsFileTest_GetParent_011
 * @tc.desc: Test function of GetParent() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetParent_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetParent_011";

    fsFile->fileEntity->uri_ = "file://storage/file_test.txt";
    auto result = fsFile->GetParent();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetParent_011";
}


/**
 * @tc.name: FsFileTest_GetPath_012
 * @tc.desc: Test function of GetPath() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetPath_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetPath_012";

    fsFile->fileEntity->uri_ = "file://storage/file_test.txt";
    auto result = fsFile->GetPath();
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetPath_012";
}

}