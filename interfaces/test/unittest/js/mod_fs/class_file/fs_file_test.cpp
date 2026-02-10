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

#include "fs_file.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "file_entity.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsFileTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsFileTest";
};

void FsFileTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsFileTest");
}

void FsFileTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsFileTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsFileTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileTest_Constructor_001
 * @tc.desc: Test function of FsFile::Constructor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_Constructor_001";

    auto result = FsFile::Constructor();

    ASSERT_TRUE(result.IsSuccess());
    std::unique_ptr<FsFile> file(result.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(file, nullptr);
    ASSERT_NE(file->fileEntity, nullptr);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_Constructor_001";
}

/**
 * @tc.name: FsFileTest_GetFD_001
 * @tc.desc: Test function of FsFile::GetFD interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetFD_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetFD_001";

    auto expectedFd = 10;
    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    FsFile file(std::move(fileEntity));

    auto result = file.GetFD();

    ASSERT_TRUE(result.IsSuccess());
    auto fd = result.GetData().value();
    EXPECT_EQ(fd, expectedFd);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetFD_001";
}

/**
 * @tc.name: FsFileTest_GetFD_002
 * @tc.desc: Test function of FsFile::GetFD interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetFD_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetFD_002";

    FsFile file(nullptr);

    auto result = file.GetFD();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetFD_002";
}

/**
 * @tc.name: FsFileTest_GetFD_003
 * @tc.desc: Test function of FsFile::GetFD interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetFD_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetFD_003";

    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.GetFD();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetFD_003";
}

/**
 * @tc.name: FsFileTest_GetPath_001
 * @tc.desc: Test function of FsFile::GetPath interface for SUCCESS when path is uri.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetPath_001";

    auto path = testDir + "/FsFileTest_GetPath_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto expectedFd = 10;
    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    fileEntity->uri_ = "file://" + path;

    FsFile file(std::move(fileEntity));

    auto result = file.GetPath();

    ASSERT_TRUE(result.IsSuccess());
    auto filePath = result.GetData().value();
    EXPECT_EQ(filePath, path);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetPath_001";
}

/**
 * @tc.name: FsFileTest_GetPath_002
 * @tc.desc: Test function of FsFile::GetPath interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetPath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetPath_002";

    FsFile file(nullptr);

    auto result = file.GetPath();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetPath_002";
}

/**
 * @tc.name: FsFileTest_GetPath_003
 * @tc.desc: Test function of FsFile::GetPath interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetPath_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetPath_003";

    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.GetPath();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetPath_003";
}

/**
 * @tc.name: FsFileTest_GetName_001
 * @tc.desc: Test function of FsFile::GetName interface for SUCCESS when path is uri.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetName_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetName_001";

    auto path = testDir + "/FsFileTest_GetName_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto expectedFd = 10;
    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    fileEntity->uri_ = "file://" + path;

    FsFile file(std::move(fileEntity));

    auto result = file.GetName();

    ASSERT_TRUE(result.IsSuccess());
    auto filename = result.GetData().value();
    EXPECT_EQ(filename, "FsFileTest_GetName_001.txt");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetName_001";
}

/**
 * @tc.name: FsFileTest_GetName_002
 * @tc.desc: Test function of FsFile::GetName interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetName_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetName_002";

    FsFile file(nullptr);

    auto result = file.GetName();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetName_002";
}

/**
 * @tc.name: FsFileTest_GetName_003
 * @tc.desc: Test function of FsFile::GetName interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetName_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetName_003";

    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.GetName();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetName_003";
}

/**
 * @tc.name: FsFileTest_GetParent_001
 * @tc.desc: Test function of FsFile::GetParent interface for SUCCESS when path is uri.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetParent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetParent_001";

    auto path = testDir + "/FsFileTest_GetParent_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto expectedFd = 10;
    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    fileEntity->uri_ = "file://" + path;

    FsFile file(std::move(fileEntity));

    auto result = file.GetParent();

    ASSERT_TRUE(result.IsSuccess());
    auto parent = result.GetData().value();
    EXPECT_EQ(parent, testDir);

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetParent_001";
}

/**
 * @tc.name: FsFileTest_GetParent_002
 * @tc.desc: Test function of FsFile::GetParent interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetParent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetParent_002";

    FsFile file(nullptr);

    auto result = file.GetParent();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetParent_002";
}

/**
 * @tc.name: FsFileTest_GetParent_003
 * @tc.desc: Test function of FsFile::GetParent interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_GetParent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_GetParent_003";

    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.GetParent();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_GetParent_003";
}

/**
 * @tc.name: FsFileTest_Lock_001
 * @tc.desc: Test function of FsFile::Lock interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_Lock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_Lock_001";

    FsFile file(nullptr);

    auto result = file.Lock(true);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_Lock_001";
}

/**
 * @tc.name: FsFileTest_Lock_002
 * @tc.desc: Test function of FsFile::Lock interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_Lock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_Lock_002";

    auto fileEntity = make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.Lock(true);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_Lock_002";
}

/**
 * @tc.name: FsFileTest_TryLock_001
 * @tc.desc: Test function of FsFile::TryLock interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_TryLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_TryLock_001";

    FsFile file(nullptr);

    auto result = file.TryLock(true);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_TryLock_001";
}

/**
 * @tc.name: FsFileTest_TryLock_002
 * @tc.desc: Test function of FsFile::TryLock interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_TryLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_TryLock_002";

    auto fileEntity = std::make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.TryLock(false);

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_TryLock_002";
}

/**
 * @tc.name: FsFileTest_UnLock_001
 * @tc.desc: Test function of FsFile::UnLock interface for FAILURE when fileEntity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_UnLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_UnLock_001";

    FsFile file(nullptr);

    auto result = file.UnLock();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_UnLock_001";
}

/**
 * @tc.name: FsFileTest_UnLock_002
 * @tc.desc: Test function of FsFile::UnLock interface for FAILURE when fileEntity->fd_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileTest, FsFileTest_UnLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileTest-begin FsFileTest_UnLock_002";

    auto fileEntity = std::make_unique<FileEntity>();
    fileEntity->fd_ = nullptr;
    FsFile file(std::move(fileEntity));

    auto result = file.UnLock();

    EXPECT_FALSE(result.IsSuccess());
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "FsFileTest-end FsFileTest_UnLock_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test