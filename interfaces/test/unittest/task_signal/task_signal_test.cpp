/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "task_signal.h"

#include <cerrno>
#include <condition_variable>
#include <gtest/gtest.h>
#include <string>

#include "filemgmt_libhilog.h"
#include "task_signal_listener.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;
using namespace OHOS::DistributedFS::ModuleTaskSignal;
using namespace FileManagement;

class TaskSignalTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};

    static std::condition_variable taskListenerCv_;
    static std::mutex taskListenerCallbackLock_;
    static std::string canceledFilePath_;
};

std::condition_variable TaskSignalTest::taskListenerCv_;
std::mutex TaskSignalTest::taskListenerCallbackLock_;
std::string TaskSignalTest::canceledFilePath_;

class TaskSignalListenerTestImpl : public TaskSignalListener {
public:
    ~TaskSignalListenerTestImpl() = default;
    void OnCancel()
    {
        string path = "aaa";
        TaskSignalTest::canceledFilePath_ = path;
        TaskSignalTest::taskListenerCv_.notify_one();
        HILOGI("OnCancel in. path = %{public}s", path.c_str());
    }
};

/**
 * @tc.name: Task_Signal_Cancel_0000
 * @tc.desc: Test function of Cancel() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: NAN
 */
HWTEST_F(TaskSignalTest, Task_Signal_Cancel_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin Task_Signal_Cancel_0000";
    auto signal = std::make_shared<TaskSignal>();
    auto ret = signal->Cancel();
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: Task_Signal_IsCancel_0000
 * @tc.desc: Test function of IsCanceled() interface
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: NAN
 */
HWTEST_F(TaskSignalTest, Task_Signal_IsCancel_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin Task_Signal_IsCancel_0000";
    auto signal = std::make_shared<TaskSignal>();
    auto isCanceled = signal->IsCanceled();
    EXPECT_TRUE(!isCanceled);

    auto ret = signal->Cancel();
    EXPECT_TRUE(ret == 0);
    isCanceled = signal->IsCanceled();
    EXPECT_TRUE(isCanceled);
}

/**
 * @tc.name: Task_Signal_SetTaskSignalListener_0000
 * @tc.desc: Test function of SetTaskSignalListener() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: NAN
 */
HWTEST_F(TaskSignalTest, Task_Signal_SetTaskSignalListener_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin Task_Signal_SetTaskSignalListener_0000";
    std::unique_lock<std::mutex> lock(TaskSignalTest::taskListenerCallbackLock_);
    auto signal = std::make_shared<TaskSignal>();
    auto listener = std::make_shared<TaskSignalListenerTestImpl>();
    signal->SetTaskSignalListener(listener.get());
    std::string filePath = "aaa";
    listener->OnCancel();
    signal->CheckCancelIfNeed(filePath);
    EXPECT_EQ(TaskSignalTest::canceledFilePath_, filePath);
}

/**
 * @tc.name: Task_Signal_MarkRemoteTask_0000
 * @tc.desc: Test function of MarkRemoteTask() and SetFileInfoOfRemoteTask() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: NAN
 */
HWTEST_F(TaskSignalTest, Task_Signal_MarkRemoteTask_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin Task_Signal_MarkRemoteTask_0000";
    auto signal = std::make_shared<TaskSignal>();
    signal->MarkRemoteTask();
    std::string sessionName = "DistributedDevice0";
    std::string filePath = "aaa";
    signal->SetFileInfoOfRemoteTask(sessionName, filePath);
    auto ret = signal->Cancel();
    EXPECT_EQ(ret, 4);
}

/**
 * @tc.name: Task_Signal_MarkDfsTask_0000
 * @tc.desc: Test function of MarkDfsTask() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: NAN
 */
HWTEST_F(TaskSignalTest, Task_Signal_MarkDfsTask_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSignalTest-begin Task_Signal_MarkDfsTask_0000";
    auto signal = std::make_shared<TaskSignal>();
    signal->MarkDfsTask();
    std::string srcPath = "/data/test/srcPath.txt";
    std::string localUri = "/data/test/test.txt";
    std::string dstUri = "/data/test/test.txt";
    signal->SetCopyTaskUri(localUri, dstUri, srcPath);
    auto ret = signal->Cancel();
    EXPECT_EQ(ret, 0);
}
} // namespace DistributedFS
} // namespace OHOS