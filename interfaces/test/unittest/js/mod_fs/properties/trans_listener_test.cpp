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
#include <filesystem>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "copy_core.h"
#include "rmdir_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

string g_path = "/data/test/TransListenerCoreTest.txt";
string g_distPath = "/data/storage/el2/distributedfiles/";
const string FILE_MANAGER_AUTHORITY = "docs";
const string MEDIA_AUTHORITY = "media";

class IProgressListenerTest : public IProgressListener {
public:
    void InvokeListener(uint64_t progressSize, uint64_t totalSize) const override {}
};

class TransListenerCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TransListenerCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    if (access(g_distPath.c_str(), F_OK)) {
        if (mkdir(g_distPath.c_str(), mode) < 0) {
            GTEST_LOG_(INFO) << "SetUpTestCase Mkdir failed errno = " << errno;
            EXPECT_TRUE(false);
        }
    }

    int32_t fd = open(g_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);
}

void TransListenerCoreTest::TearDownTestCase(void)
{
    auto result = rmdir(g_path.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "TearDownTestCase Rmdir failed! ret: " << result << ", errno: " << errno;
    }

    error_code ec;
    bool success = filesystem::remove_all(g_distPath, ec);
    if (!success || ec) {
        GTEST_LOG_(ERROR) << "TearDownTestCase Rmdir failed error = " << ec.message();
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TransListenerCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TransListenerCoreTest_CreateDfsCopyPath_001
 * @tc.desc: Test function of TransListenerCore::CreateDfsCopyPath interface for FALSE.
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
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_001";

    string path = "/data/test/TransListenerCoreTest_HandleCopyFailure_001.txt";
    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";
    CopyEvent event;
    event.errorCode = 0;
    int result = TransListenerCore::HandleCopyFailure(event, info, path, "");
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_001";
}

/**
 * @tc.name: TransListenerCoreTest_HandleCopyFailure_002
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_002";

    string path = "/data/test/TransListenerCoreTest_HandleCopyFailure_002.txt";
    int32_t fd = open(path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";
    CopyEvent event;
    event.errorCode = SOFTBUS_TRANS_FILE_EXISTED;
    int result = TransListenerCore::HandleCopyFailure(event, info, path, "");
    EXPECT_EQ(result, EEXIST);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_002";
}

/**
 * @tc.name: TransListenerCoreTest_HandleCopyFailure_003
 * @tc.desc: Test function of TransListenerCore::HandleCopyFailure interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_HandleCopyFailure_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_HandleCopyFailure_003";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = MEDIA_AUTHORITY;
    CopyEvent event;
    event.errorCode = DFS_CANCEL_SUCCESS;
    int result = TransListenerCore::HandleCopyFailure(event, info, "", "");
    EXPECT_EQ(result, ECANCELED);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_HandleCopyFailure_003";
}

/**
 * @tc.name: TransListenerCoreTest_WaitForCopyResult_001
 * @tc.desc: Test function of TransListenerCore::WaitForCopyResult interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_WaitForCopyResult_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_WaitForCopyResult_001";

    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    transListener->copyEvent_.copyResult = FAILED;
    int result = TransListenerCore::WaitForCopyResult(transListener.get());
    EXPECT_EQ(result, 2);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_WaitForCopyResult_001";
}

/**
 * @tc.name: TransListenerCoreTest_WaitForCopyResult_002
 * @tc.desc: Test function of TransListenerCore::WaitForCopyResult interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_WaitForCopyResult_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_WaitForCopyResult_002";

    int result = TransListenerCore::WaitForCopyResult(nullptr);
    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_WaitForCopyResult_002";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_001
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_001";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = MEDIA_AUTHORITY;
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    string disSandboxPath = "disSandboxPath";

    int result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_001";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_002
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_002";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";

    string disSandboxPath = "disSandboxPath";
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    int result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_002";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_003
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_003";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";
    info.sandboxPath = "abc";

    string disSandboxPath = "disSandboxPath";
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    int result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_003";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_004
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_004";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";
    info.sandboxPath = "/data/test/PrepareCopySession_004.txt";

    string disSandboxPath = "disSandboxPath";
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    int result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_004";
}

/**
 * @tc.name: TransListenerCoreTest_PrepareCopySession_005
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_PrepareCopySession_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_PrepareCopySession_005";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = "abc";
    info.sandboxPath = "/data/test/PrepareCopySession_004.txt";
    int32_t fd = open(info.sandboxPath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        EXPECT_TRUE(false);
    }
    close(fd);

    string disSandboxPath = "disSandboxPath";
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    int result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_PrepareCopySession_005";
}

/**
 * @tc.name: TransListenerCoreTest_CopyToSandBox_001
 * @tc.desc: Test function of TransListenerCore::CopyToSandBox interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyToSandBox_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyToSandBox_001";

    string disSandboxPath = "disSandboxPath";
    string sandboxPath = "sandboxPath";
    string currentId = "currentId";

    int result = TransListenerCore::CopyToSandBox("srcUri", disSandboxPath, sandboxPath, currentId);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyToSandBox_001";
}

/**
 * @tc.name: TransListenerCoreTest_CopyToSandBox_002
 * @tc.desc: Test function of TransListenerCore::CopyToSandBox interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyToSandBox_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyToSandBox_002";

    string disSandboxPath = g_path;
    string sandboxPath = "/data/test";
    string currentId = "currentId";

    int result = TransListenerCore::CopyToSandBox("srcUri", disSandboxPath, sandboxPath, currentId);
    EXPECT_EQ(result, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyToSandBox_002";
}

/**
 * @tc.name: TransListenerCoreTest_GetFileName_001
 * @tc.desc: Test function of TransListenerCore::GetFileName interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetFileName_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetFileName_001";

    string path = "abc";

    auto result = TransListenerCore::GetFileName(path);
    EXPECT_EQ(result, "");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetFileName_001";
}

/**
 * @tc.name: TransListenerCoreTest_GetFileName_002
 * @tc.desc: Test function of TransListenerCore::GetFileName interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetFileName_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetFileName_002";

    auto result = TransListenerCore::GetFileName(g_path);
    EXPECT_EQ(result, "/TransListenerCoreTest.txt");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetFileName_002";
}

/**
 * @tc.name: TransListenerCoreTest_GetNetworkIdFromUri_001
 * @tc.desc: Test function of TransListenerCore::GetNetworkIdFromUri interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_GetNetworkIdFromUri_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_GetNetworkIdFromUri_001";

    string uri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    auto result = TransListenerCore::GetNetworkIdFromUri(uri);
    EXPECT_EQ(result, "AD125AD1CF");

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_GetNetworkIdFromUri_001";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_001
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FALSE.
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
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_002";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(make_shared<IProgressListenerTest>()));
    entry->callback = nullptr;
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_002";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_003
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_003";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(make_shared<IProgressListenerTest>()));
    entry->callback->listener = nullptr;
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_003";
}

/**
 * @tc.name: TransListenerCoreTest_CallbackComplete_004
 * @tc.desc: Test function of TransListenerCore::CallbackComplete interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CallbackComplete_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CallbackComplete_004";

    auto entry = make_shared<FsUvEntry>(make_shared<FsCallbackObject>(make_shared<IProgressListenerTest>()));
    TransListenerCore::CallbackComplete(entry);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CallbackComplete_004";
}

/**
 * @tc.name: TransListenerCoreTest_OnFileReceive_001
 * @tc.desc: Test function of TransListenerCore::OnFileReceive interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFileReceive_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFileReceive_001";

    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    transListener->callback_ = nullptr;
    auto res = transListener->OnFileReceive(0, 0);
    EXPECT_EQ(res, ENOMEM);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFileReceive_001";
}

/**
 * @tc.name: TransListenerCoreTest_OnFileReceive_002
 * @tc.desc: Test function of TransListenerCore::OnFileReceive interface for SUCC.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFileReceive_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFileReceive_002";

    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    transListener->callback_ = make_shared<FsCallbackObject>(make_shared<IProgressListenerTest>());
    auto res = transListener->OnFileReceive(0, 0);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFileReceive_002";
}

/**
 * @tc.name: TransListenerCoreTest_OnFinished_001
 * @tc.desc: Test function of TransListenerCore::OnFinished interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFinished_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFinished_001";

    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    auto res = transListener->OnFinished("sessionName");
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFinished_001";
}

/**
 * @tc.name: TransListenerCoreTest_OnFailed_001
 * @tc.desc: Test function of TransListenerCore::OnFailed interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_OnFailed_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_OnFailed_001";

    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    auto res = transListener->OnFailed("sessionName", 0);
    EXPECT_EQ(res, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_OnFailed_001";
}

/**
 * @tc.name: TransListenerCoreTest_CopyFileFromSoftBus_001
 * @tc.desc: Test function of TransListenerCore::CopyFileFromSoftBus interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreTest, TransListenerCoreTest_CopyFileFromSoftBus_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreTest-begin TransListenerCoreTest_CopyFileFromSoftBus_001";

    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";
    shared_ptr<TransListenerCore> transListener = make_shared<TransListenerCore>();
    shared_ptr<FsFileInfos> infos = make_shared<FsFileInfos>();
    transListener->copyEvent_.copyResult = FAILED;

    auto res = transListener->CopyFileFromSoftBus(srcUri, "destUri", infos, nullptr);
    EXPECT_EQ(res, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreTest-end TransListenerCoreTest_CopyFileFromSoftBus_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test