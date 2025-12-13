/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "trans_listener_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "copy_core.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static const string FILE_MANAGER_AUTHORITY = "docs";
static const string MEDIA_AUTHORITY = "media";
static const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles";

class IProgressListenerTest : public IProgressListener {
public:
    void InvokeListener(uint64_t progressSize, uint64_t totalSize) const override {}
};

class TransListenerCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/TransListenerCoreTest";
};

void TransListenerCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "TransListenerCoreTest");
}

void TransListenerCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(DISTRIBUTED_PATH, true));
}

void TransListenerCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    ASSERT_TRUE(FileUtils::RemoveAll(DISTRIBUTED_PATH));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TransListenerCoreTest_CreateDfsCopyPath_001
 * @tc.desc: Test function of TransListenerCore::CreateDfsCopyPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CreateDfsCopyPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CreateDfsCopyPath_001";

    string result = TransListenerCore::CreateDfsCopyPath();
    EXPECT_FALSE(result.empty());

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CreateDfsCopyPath_001";
}

/**
 * @tc.name: TransListenerCoreTest_HandleCopyFailure_001
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for FAILURE when event.errorCode is not in
 * the softbusErr2ErrCodeTable.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_001";

    auto disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_HandleCopyFailure_001";
    auto currentId = "fakeCurrentId";
    ASSERT_TRUE(FileUtils::CreateDirectories(disSandboxPath));

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";
    CopyEvent event;
    event.errorCode = 0;

    int result = TransListenerCore::HandleCopyFailure(event, info, disSandboxPath, currentId);

    EXPECT_EQ(result, EIO);
    ASSERT_FALSE(FileUtils::Exists(disSandboxPath));

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_001";
}

/**
 * @tc.name: TransListenerCoreTest_HandleCopyFailure_002
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for FAILURE when event.errorCode is
 * SOFTBUS_TRANS_FILE_EXISTED, which is in the softbusErr2ErrCodeTable.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_002";

    auto disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_HandleCopyFailure_002";
    auto currentId = "fakeCurrentId";
    ASSERT_TRUE(FileUtils::CreateDirectories(disSandboxPath));

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";
    CopyEvent event;
    event.errorCode = SOFTBUS_TRANS_FILE_EXISTED;

    int result = TransListenerCore::HandleCopyFailure(event, info, disSandboxPath, currentId);

    EXPECT_EQ(result, EEXIST);
    ASSERT_FALSE(FileUtils::Exists(disSandboxPath));

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_002";
}

/**
 * @tc.name: TransListenerCoreTest_HandleCopyFailure_003
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_003";

    auto disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_HandleCopyFailure_003";
    auto currentId = "fakeCurrentId";
    ASSERT_TRUE(FileUtils::CreateDirectories(disSandboxPath));

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = MEDIA_AUTHORITY;
    CopyEvent event;
    event.errorCode = DFS_CANCEL_SUCCESS;

    int result = TransListenerCore::HandleCopyFailure(event, info, disSandboxPath, currentId);

    EXPECT_EQ(result, ECANCELED);
    ASSERT_TRUE(FileUtils::Exists(disSandboxPath));

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_003";
}

/**
 * @tc.name: TransListenerCoreTest_WaitForCopyResult_001
 * @tc.desc: Test function of TransListenerCore::WaitForCopyResult interface for SUCCESS when copyResult is FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_WaitForCopyResult_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_WaitForCopyResult_001";

    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    transListener->copyEvent_.copyResult = FAILED;
    int result = TransListenerCore::WaitForCopyResult(transListener.get());
    EXPECT_EQ(result, FAILED);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_WaitForCopyResult_001";
}

/**
 * @tc.name: TransListenerCoreTest_WaitForCopyResult_002
 * @tc.desc: Test function of TransListenerCore::WaitForCopyResult interface for FAILURE when transListener is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_WaitForCopyResult_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_WaitForCopyResult_002";

    int result = TransListenerCore::WaitForCopyResult(nullptr);
    EXPECT_EQ(result, FAILED);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_WaitForCopyResult_002";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_001
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FAILURE when PrepareSession fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_001";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = MEDIA_AUTHORITY;

    string disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_PrepareCopySession_001";
    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    string destUri = testDir + "/TransListenerCoreTest_PrepareCopySession_001";
    ASSERT_TRUE(FileUtils::CreateDirectories(destUri));

    int result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_001";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_002
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FAILURE when creating disSandboxPath
 * fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_002";

    ASSERT_TRUE(FileUtils::RemoveAll(DISTRIBUTED_PATH)); // mock the DISTRIBUTED_PATH directory not existent
    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";

    string disSandboxPath;
    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    string destUri = testDir + "/TransListenerCoreTest_PrepareCopySession_002";

    int result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);

    EXPECT_EQ(result, ENOENT);
    EXPECT_EQ(disSandboxPath.find(DISTRIBUTED_PATH), 0);
    EXPECT_GT(disSandboxPath.compare(DISTRIBUTED_PATH), 0);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_002";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_003
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FAILURE when info.sandboxPath is
 * invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_003";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";
    info.sandboxPath = "invalidSandboxPath";

    string disSandboxPath;
    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    string destUri = testDir + "/TransListenerCoreTest_PrepareCopySession_003";

    int result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);

    EXPECT_EQ(result, EIO);
    EXPECT_EQ(disSandboxPath.find(DISTRIBUTED_PATH), 0);
    EXPECT_GT(disSandboxPath.compare(DISTRIBUTED_PATH), 0);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_003";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_004
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FAILURE when info.sandboxPath not
 * exists and PrepareCopySession fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_004";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";
    info.sandboxPath = testDir + "/TransListenerCoreTest_PrepareCopySession_004/non_existent/";

    string disSandboxPath;
    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    string destUri = testDir + "/TransListenerCoreTest_PrepareCopySession_004";

    int result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);

    EXPECT_EQ(result, EIO);
    EXPECT_EQ(disSandboxPath.find(DISTRIBUTED_PATH), 0);
    EXPECT_GT(disSandboxPath.compare(DISTRIBUTED_PATH), 0);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_004";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_005
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FAILURE when info.sandboxPath is
 * exists but PrepareCopySession fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_005";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "fakeAuthority";
    info.sandboxPath = testDir + "/TransListenerCoreTest_PrepareCopySession_005/sandboxFile.txt";
    ASSERT_TRUE(FileUtils::CreateFile(info.sandboxPath, "content"));

    string disSandboxPath;
    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    string destUri = testDir + "/TransListenerCoreTest_PrepareCopySession_005";

    int result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);

    EXPECT_EQ(result, EIO);
    EXPECT_EQ(disSandboxPath.find(DISTRIBUTED_PATH), 0);
    EXPECT_GT(disSandboxPath.compare(DISTRIBUTED_PATH), 0);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_005";
}

/**
 * @tc.name: TransListenerCoreTest_CopyToSandBox_001
 * @tc.desc: Test function of TransListenerCore::CopyToSandBox interface for SUCCESS when copying a directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyToSandBox_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyToSandBox_001";

    string disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_CopyToSandBox_001";
    string sandboxPath = testDir + "/TransListenerCoreTest_CopyToSandBox_001";
    string currentId = "fakeCurrentId";
    string srcSubDir = disSandboxPath + "/subDir";
    string srcSubFile = disSandboxPath + "/subFile.txt";
    ASSERT_TRUE(FileUtils::CreateDirectories(disSandboxPath));
    ASSERT_TRUE(FileUtils::CreateDirectories(sandboxPath));
    ASSERT_TRUE(FileUtils::CreateDirectories(srcSubDir));
    ASSERT_TRUE(FileUtils::CreateFile(srcSubFile, "content"));

    string destSubDir = sandboxPath + "/subDir";
    string destSubFile = sandboxPath + "/subFile.txt";

    int result = TransListenerCore::CopyToSandBox("fakeSrcUri", disSandboxPath, sandboxPath, currentId);

    EXPECT_EQ(result, ERRNO_NOERR);
    EXPECT_TRUE(FileUtils::Exists(destSubDir));
    EXPECT_TRUE(FileUtils::Exists(destSubFile));

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyToSandBox_001";
}

/**
 * @tc.name: TransListenerCoreTest_CopyToSandBox_002
 * @tc.desc: Test function of TransListenerCore::CopyToSandBox interface for SUCCESS when copying a file.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyToSandBox_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyToSandBox_002";

    string disSandboxPath = DISTRIBUTED_PATH + "/TransListenerCoreTest_CopyToSandBox_002";
    string sandboxPath = testDir + "/TransListenerCoreTest_CopyToSandBox_002.txt";
    string currentId = "fakeCurrentId";
    string srcFile = disSandboxPath + "/subFile.txt";
    string srcUri = "file://" + srcFile;
    ASSERT_TRUE(FileUtils::CreateFile(srcFile, "content"));

    int result = TransListenerCore::CopyToSandBox(srcUri, disSandboxPath, sandboxPath, currentId);

    EXPECT_EQ(result, ERRNO_NOERR);
    EXPECT_EQ(FileUtils::ReadTextFileContent(sandboxPath), std::make_tuple(true, "content"));

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyToSandBox_002";
}

/**
 * @tc.name: TransListenerCoreTest_GetFileName_001
 * @tc.desc: Test function of TransListenerCore::GetFileName interface for FAILURE when path is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetFileName_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetFileName_001";

    string path = "invalidPath";

    auto result = TransListenerCore::GetFileName(path);
    EXPECT_EQ(result, "");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetFileName_001";
}

/**
 * @tc.name: TransListenerCoreTest_GetFileName_002
 * @tc.desc: Test function of TransListenerCore::GetFileName interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetFileName_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetFileName_002";

    auto path = testDir + "/TransListenerCoreTest_GetFileName_002.txt";
    auto result = TransListenerCore::GetFileName(path);
    EXPECT_EQ(result, "/TransListenerCoreTest_GetFileName_002.txt");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetFileName_002";
}

/**
 * @tc.name: TransListenerCoreTest_GetNetworkIdFromUri_001
 * @tc.desc: Test function of TransListenerCore::GetNetworkIdFromUri interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetNetworkIdFromUri_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetNetworkIdFromUri_001";

    string uri = "file://translistener.preparecopysession?networkid=AD125AD1CF";

    auto result = TransListenerCore::GetNetworkIdFromUri(uri);
    EXPECT_EQ(result, "AD125AD1CF");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetNetworkIdFromUri_001";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_001
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FAILURE when entry is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_001";

    TransListenerCore::CallbackComplete(nullptr);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_001";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_002
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FAILURE when entry->callback is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_002";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(std::make_shared<IProgressListenerTest>()));
    entry->callback = nullptr;
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_002";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_003
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FAILURE when entry->callback->listener
 * is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_003";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(std::make_shared<IProgressListenerTest>()));
    entry->callback->listener = nullptr;
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_003";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_004
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_004";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(std::make_shared<IProgressListenerTest>()));
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_004";
}

/**
 * @tc.name: TransListenerCoreTest_OnFileReceive_001
 * @tc.desc: Test function of TransListenerCore::OnFileReceive interface for FAILURE when transListener->callback_ is
 * nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFileReceive_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFileReceive_001";

    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    transListener->callback_ = nullptr;
    auto res = transListener->OnFileReceive(0, 0);
    EXPECT_EQ(res, ENOMEM);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFileReceive_001";
}

/**
 * @tc.name: TransListenerCoreTest_OnFileReceive_002
 * @tc.desc: Test function of TransListenerCore::OnFileReceive interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFileReceive_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFileReceive_002";

    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    transListener->callback_ = make_shared<FsCallbackObject>(std::make_shared<IProgressListenerTest>());
    auto res = transListener->OnFileReceive(0, 0);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFileReceive_002";
}

/**
 * @tc.name: TransListenerCoreTest_OnFinished_001
 * @tc.desc: Test function of TransListenerCore::OnFinished interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFinished_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFinished_001";

    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    auto res = transListener->OnFinished("sessionName");
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFinished_001";
}

/**
 * @tc.name: TransListenerCoreTest_OnFailed_001
 * @tc.desc: Test function of TransListenerCore::OnFailed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFailed_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFailed_001";

    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    auto res = transListener->OnFailed("sessionName", 0);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFailed_001";
}

/**
 * @tc.name: TransListenerCoreTest_CopyFileFromSoftBus_001
 * @tc.desc: Test function of TransListenerCore::CopyFileFromSoftBus interface for FAILURE when PrepareCopySession fails
 * (invalid file Path).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyFileFromSoftBus_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyFileFromSoftBus_001";

    string srcUri = "file://translistener.preparecopysession?networkid=AD125AD1CF";
    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    std::shared_ptr<FsFileInfos> infos = std::make_shared<FsFileInfos>();

    auto res = transListener->CopyFileFromSoftBus(srcUri, "fakeDestUri", infos, nullptr);
    EXPECT_EQ(res, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyFileFromSoftBus_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test