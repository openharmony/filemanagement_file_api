/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"
#include "task_signal_listener.h"
#include "task_signal.h"

using namespace OHOS;
using namespace OHOS::Storage::DistributedFile;

class MockDistributedFileDaemonManager : public DistributedFileDaemonManager {
public:
    MOCK_METHOD(int32_t, CancelCopyTask, (const std::string &sessionName), (override));

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

    int32_t PrepareSession(const std::string &srcUri, const std::string &dstUri, const std::string &srcDeviceId,
        const sptr<IRemoteObject> &listener, HmdfsInfo &info) override
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

    static MockDistributedFileDaemonManager &GetInstance()
    {
        static MockDistributedFileDaemonManager instance;
        return instance;
    }
};

static MockDistributedFileDaemonManager &g_mockDistributedFileDaemonManager =
    MockDistributedFileDaemonManager::GetInstance();

#ifdef ENABLE_DISTRIBUTED_FILE_MOCK
DistributedFileDaemonManager &DistributedFileDaemonManager::GetInstance()
{
    return MockDistributedFileDaemonManager::GetInstance();
}
#endif

class MockTaskSignalListener : public OHOS::DistributedFS::ModuleTaskSignal::TaskSignalListener {
public:
    MOCK_METHOD(void, OnCancel, (), (override));
};

namespace OHOS {
namespace DistributedFS {
namespace ModuleTaskSignal {
namespace Test {
using namespace std;
using namespace OHOS::DistributedFS::ModuleTaskSignal;

class TaskSignalTest : public testing::Test {
public:
    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}

    void SetUp() override
    {
        taskSignal_ = std::make_unique<TaskSignal>();
    }

    void TearDown() override
    {
        taskSignal_.reset();
    }

    MockDistributedFileDaemonManager &GetMock()
    {
        return g_mockDistributedFileDaemonManager;
    }

protected:
    std::unique_ptr<TaskSignal> taskSignal_;
};

/**
 * @tc.name: TaskSignalTest_Cancel_001
 * @tc.desc: Test function of TaskSignal::Cancel interface for SUCCESS when remoteTask_ is false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_Cancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_Cancel_001";
    // Prepare test condition
    taskSignal_->remoteTask_.store(false);
    // Do testing
    auto result = taskSignal_->Cancel();
    // Verify results
    EXPECT_EQ(result, 0);
    EXPECT_TRUE(taskSignal_->needCancel_.load());
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_Cancel_001";
}

/**
 * @tc.name: TaskSignalTest_Cancel_002
 * @tc.desc: Test function of TaskSignal::Cancel interface for SUCCESS when remoteTask_ is false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_Cancel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_Cancel_002";
    // Prepare test condition
    taskSignal_->remoteTask_.store(true);
    taskSignal_->sessionName_ = "TaskSignalTest_Cancel_002";
    // Set mock behaviors
    int expectedCode = 0;
    EXPECT_CALL(GetMock(), CancelCopyTask(testing::_)).WillOnce(testing::Return(expectedCode));
    // Do testing
    auto result = taskSignal_->Cancel();
    // Verify results
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_Cancel_002";
}

/**
 * @tc.name: TaskSignalTest_Cancel_003
 * @tc.desc: Test function of TaskSignal::Cancel interface for FAILURE when sessionName_ is empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_Cancel_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_Cancel_003";
    // Prepare test parameters
    taskSignal_->remoteTask_.store(true);
    taskSignal_->sessionName_ = "";
    // Do testing
    auto result = taskSignal_->Cancel();
    // Verify results
    EXPECT_EQ(result, -3);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_Cancel_003";
}

/**
 * @tc.name: TaskSignalTest_Cancel_004
 * @tc.desc: Test function of TaskSignal::Cancel interface for FAILURE when CancelCopyTask fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_Cancel_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_Cancel_004";
    // Prepare test parameters
    taskSignal_->remoteTask_.store(true);
    taskSignal_->sessionName_ = "TaskSignalTest_Cancel_004";
    // Set mock behaviors
    int expectedCode = -1;
    EXPECT_CALL(GetMock(), CancelCopyTask(testing::_)).WillOnce(testing::Return(expectedCode));
    // Do testing
    auto result = taskSignal_->Cancel();
    // Verify results
    EXPECT_EQ(result, expectedCode);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_Cancel_004";
}

/**
 * @tc.name: TaskSignalTest_IsCanceled_001
 * @tc.desc: Test function of TaskSignal::IsCanceled interface for case when needCancel_ is true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_IsCanceled_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_IsCanceled_001";
    // Prepare test condition
    taskSignal_->needCancel_.store(true);
    // Do testing
    bool result = taskSignal_->IsCanceled();
    // Verify results
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_IsCanceled_001";
}

/**
 * @tc.name: TaskSignalTest_IsCanceled_002
 * @tc.desc: Test function of TaskSignal::IsCanceled interface for case when remoteTask_ is true.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_IsCanceled_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_IsCanceled_002";
    // Prepare test condition
    taskSignal_->remoteTask_.store(true);
    // Do testing
    bool result = taskSignal_->IsCanceled();
    // Verify results
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_IsCanceled_002";
}

/**
 * @tc.name: TaskSignalTest_IsCanceled_003
 * @tc.desc: Test function of TaskSignal::IsCanceled interface for case when both needCancel_ and remoteTask_ are false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_IsCanceled_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_IsCanceled_003";
    // Prepare test condition
    taskSignal_->needCancel_.store(false);
    taskSignal_->remoteTask_.store(false);
    // Do testing
    bool result = taskSignal_->IsCanceled();
    // Verify results
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_IsCanceled_003";
}

/**
 * @tc.name: TaskSignalTest_SetTaskSignalListener_001
 * @tc.desc: Test function of TaskSignal::SetTaskSignalListener interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_SetTaskSignalListener_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_SetTaskSignalListener_001";
    // Prepare test parameters
    MockTaskSignalListener mockListener;
    // Do testing
    taskSignal_->SetTaskSignalListener(&mockListener);
    // Verify results
    EXPECT_EQ(taskSignal_->signalListener_, &mockListener);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_SetTaskSignalListener_001";
}

/**
 * @tc.name: TaskSignalTest_SetTaskSignalListener_002
 * @tc.desc: Test function of TaskSignal::SetTaskSignalListener interface for FAILURE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_SetTaskSignalListener_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_SetTaskSignalListener_002";
    // Prepare test parameters
    MockTaskSignalListener expectedListener;
    // Prepre test condition
    MockTaskSignalListener mockListener;
    taskSignal_->signalListener_ = &mockListener;
    // Do testing
    taskSignal_->SetTaskSignalListener(&expectedListener);
    // Verify results
    EXPECT_EQ(taskSignal_->signalListener_, &mockListener);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_SetTaskSignalListener_002";
}

/**
 * @tc.name: TaskSignalTest_OnCancel_001
 * @tc.desc: Test function of TaskSignal::OnCancel interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_OnCancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_OnCancel_001";
    // Set mock behaviors
    MockTaskSignalListener mockListener;
    taskSignal_->signalListener_ = &mockListener;
    EXPECT_CALL(mockListener, OnCancel()).Times(1);
    // Do testing
    taskSignal_->OnCancel();
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_OnCancel_001";
}

/**
 * @tc.name: TaskSignalTest_CheckCancelIfNeed_001
 * @tc.desc: Test function of TaskSignal::CheckCancelIfNeed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_CheckCancelIfNeed_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_CheckCancelIfNeed_001";
    // Set mock behaviors
    taskSignal_->needCancel_.store(true);
    MockTaskSignalListener mockListener;
    taskSignal_->signalListener_ = &mockListener;
    EXPECT_CALL(mockListener, OnCancel()).Times(1);
    // Do testing
    bool result = taskSignal_->CheckCancelIfNeed("TaskSignalTest_CheckCancelIfNeed_001");
    // Verify results
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_CheckCancelIfNeed_001";
}

/**
 * @tc.name: TaskSignalTest_CheckCancelIfNeed_002
 * @tc.desc: Test function of TaskSignal::CheckCancelIfNeed interface for FAILURE when needCancel_ is false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_CheckCancelIfNeed_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_CheckCancelIfNeed_002";
    // Prepare test condition
    taskSignal_->needCancel_.store(false);
    // Do testing
    bool result = taskSignal_->CheckCancelIfNeed("TaskSignalTest_CheckCancelIfNeed_002");
    // Verify results
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_CheckCancelIfNeed_002";
}

/**
 * @tc.name: TaskSignalTest_MarkRemoteTask_001
 * @tc.desc: Test function of TaskSignal::MarkRemoteTask interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_MarkRemoteTask_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_MarkRemoteTask_001";
    // Do testing
    taskSignal_->MarkRemoteTask();
    // Verify results
    EXPECT_TRUE(taskSignal_->remoteTask_.load());
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_MarkRemoteTask_001";
}

/**
 * @tc.name: TaskSignalTest_SetFileInfoOfRemoteTask_001
 * @tc.desc: Test function of TaskSignal::SetFileInfoOfRemoteTask interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TaskSignalTest, TaskSignalTest_SetFileInfoOfRemoteTask_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin TaskSignalTest_SetFileInfoOfRemoteTask_001";
    // Prepare test parameters
    std::string sessionName = "testSession";
    std::string filePath = "testFilePath";
    // Do testing
    taskSignal_->SetFileInfoOfRemoteTask(sessionName, filePath);
    // Verify results
    EXPECT_EQ(taskSignal_->sessionName_, sessionName);
    EXPECT_EQ(taskSignal_->filePath_, filePath);
    GTEST_LOG_(INFO) << "TaskSignalTest-end TaskSignalTest_SetFileInfoOfRemoteTask_001";
}

} // namespace Test
} // namespace ModuleTaskSignal
} // namespace DistributedFS
} // namespace OHOS