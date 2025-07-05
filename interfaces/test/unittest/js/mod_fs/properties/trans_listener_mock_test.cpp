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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "copy_core.h"
#include "unistd_mock.h"

using namespace OHOS;
using namespace OHOS::Storage::DistributedFile;

class MockDistributedFileDaemonManager final : public DistributedFileDaemonManager {
public:
    MOCK_METHOD(int32_t, PrepareSession,
        (const std::string &srcUri, const std::string &dstUri, const std::string &srcDeviceId,
            const sptr<IRemoteObject> &listener, HmdfsInfo &info),
        (override));

    int32_t OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override
    {
        return 0;
    }
    int32_t CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override
    {
        return 0;
    }
    int32_t OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj) override
    {
        return 0;
    }
    int32_t CloseP2PConnectionEx(const std::string &networkId) override
    {
        return 0;
    }
    int32_t CancelCopyTask(const std::string &sessionName) override
    {
        return 0;
    }
    int32_t CancelCopyTask(const std::string &srcUri, const std::string &dstUri)
    {
        return 0;
    }
    int32_t GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus)
    {
        return 0;
    }
    int32_t UpdateDfsSwitchStatus(int32_t switchStatus)
    {
        return 0;
    }
    int32_t GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
    {
        return 0;
    }
    int32_t PushAsset(
        int32_t userId, const sptr<AssetObj> &assetObj, const sptr<IAssetSendCallback> &sendCallback) override
    {
        return 0;
    }
    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }
    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }
    int32_t GetSize(const std::string &uri, bool isSrcUri, uint64_t &size) override
    {
        return 0;
    }
    int32_t IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory) override
    {
        return 0;
    }
    int32_t Copy(const std::string &srcUri, const std::string &destUri, ProcessCallback processCallback) override
    {
        return 0;
    }
    int32_t Cancel(const std::string &srcUri, const std::string &destUri) override
    {
        return 0;
    }
    int32_t Cancel() override
    {
        return 0;
    }
    int32_t CancelCopyTask(const std::string &srcUri, const std::string &dstUri) override
    {
        return 0;
    }
    int32_t GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus) override
    {
        return 0;
    }
    int32_t UpdateDfsSwitchStatus(int32_t switchStatus) override
    {
        return 0;
    }
    int32_t GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList) override
    {
        return 0;
    }
    int32_t RegisterFileDfsListener(const std::string &instanceId, const sptr<IFileDfsListener> &listener) override
    {
        return 0;
    }
    int32_t UnregisterFileDfsListener(const std::string &instanceId) override
    {
        return 0;
    }
    int32_t IsSameAccountDevice(const std::string &networkId, bool &isSameAccount) override
    {
        return 0;
    }

    MockDistributedFileDaemonManager() = default;
    ~MockDistributedFileDaemonManager() = default;

    static std::shared_ptr<MockDistributedFileDaemonManager> GetMock();
    static void DisableMock();

private:
    static thread_local std::shared_ptr<MockDistributedFileDaemonManager> managerMock;
};

thread_local std::shared_ptr<MockDistributedFileDaemonManager> MockDistributedFileDaemonManager::managerMock = nullptr;

std::shared_ptr<MockDistributedFileDaemonManager> MockDistributedFileDaemonManager::GetMock()
{
    if (managerMock == nullptr) {
        managerMock = std::make_shared<MockDistributedFileDaemonManager>();
    }
    return managerMock;
}

void MockDistributedFileDaemonManager::DisableMock()
{
    managerMock = nullptr;
}

class MockTaskSignalListener : public OHOS::DistributedFS::ModuleTaskSignal::TaskSignalListener {
public:
    MOCK_METHOD(void, OnCancel, (), (override));
};

#ifdef ENABLE_DISTRIBUTED_FILE_MOCK
DistributedFileDaemonManager &DistributedFileDaemonManager::GetInstance()
{
    return *MockDistributedFileDaemonManager::GetMock();
}
#endif

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

string g_path = "/data/test/TransListenerCoreMockTest.txt";
const string FILE_MANAGER_AUTHORITY = "docs";
const string MEDIA_AUTHORITY = "media";

class IProgressListenerTest : public IProgressListener {
public:
    void InvokeListener(uint64_t progressSize, uint64_t totalSize) const override {}
};

class TransListenerCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TransListenerCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t fd = open(g_path.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        ASSERT_TRUE(false);
    }
    close(fd);
    UnistdMock::EnableMock();
}

void TransListenerCoreMockTest::TearDownTestCase(void)
{
    rmdir(g_path.c_str());
    MockDistributedFileDaemonManager::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TransListenerCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TransListenerCoreMockTest_PrepareCopySession_001
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreMockTest, TransListenerCoreMockTest_PrepareCopySession_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-begin TransListenerCoreMockTest_PrepareCopySession_001";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = FILE_MANAGER_AUTHORITY;
    info.authority = MEDIA_AUTHORITY;
    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";

    string disSandboxPath = "disSandboxPath";
    auto unistdMock = UnistdMock::GetMock();
    auto managerMock = MockDistributedFileDaemonManager::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*managerMock, PrepareSession(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(ERRNO_NOERR));
    auto result = TransListenerCore::PrepareCopySession(srcUri, "destUri", nullptr, info, disSandboxPath);
    EXPECT_EQ(result, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-end TransListenerCoreMockTest_PrepareCopySession_001";
}

/**
 * @tc.name: TransListenerCoreMockTest_CopyFileFromSoftBus_001
 * @tc.desc: Test function of TransListenerCore::CopyFileFromSoftBus interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreMockTest, TransListenerCoreMockTest_CopyFileFromSoftBus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-begin TransListenerCoreMockTest_CopyFileFromSoftBus_001";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = FILE_MANAGER_AUTHORITY;

    string srcUri = "http://translistener.preparecopysession?networkid=AD125AD1CF";
    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    std::shared_ptr<FsFileInfos> infos = std::make_shared<FsFileInfos>();
    auto unistdMock = UnistdMock::GetMock();
    auto managerMock = MockDistributedFileDaemonManager::GetMock();

    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*managerMock, PrepareSession(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(ERRNO_NOERR));

    auto res = transListener->CopyFileFromSoftBus(srcUri, "destUri", infos, nullptr);
    EXPECT_EQ(res, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-end TransListenerCoreMockTest_CopyFileFromSoftBus_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
