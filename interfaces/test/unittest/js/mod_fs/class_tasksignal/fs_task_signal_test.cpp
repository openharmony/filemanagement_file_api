/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "fs_task_signal.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class Assistant : public TaskSignalListener {
public:
    void OnCancel() {}
};

class FsTaskSignalTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsTaskSignalTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsTaskSignalTest");
}

void FsTaskSignalTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsTaskSignalTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsTaskSignalTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_001
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for FAILURE when taskSignal param is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_001";

    shared_ptr<TaskSignal> taskSignal = nullptr;
    shared_ptr<TaskSignalListener> signalListener = nullptr;

    auto res = FsTaskSignal::Constructor(taskSignal, signalListener);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_001";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_002
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for FAILURE when signalListener param is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_002";

    shared_ptr<TaskSignal> taskSignal = std::make_shared<TaskSignal>();
    shared_ptr<TaskSignalListener> signalListener = nullptr;

    auto res = FsTaskSignal::Constructor(taskSignal, signalListener);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_002";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_003
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_003";

    shared_ptr<TaskSignal> taskSignal = std::make_shared<TaskSignal>();
    shared_ptr<TaskSignalListener> signalListener = std::make_shared<Assistant>();

    auto res = FsTaskSignal::Constructor(taskSignal, signalListener);

    ASSERT_TRUE(res.IsSuccess());
    std::unique_ptr<FsTaskSignal> fsTaskSignal = std::move(res.GetData().value());
    ASSERT_NE(fsTaskSignal, nullptr);
    ASSERT_NE(fsTaskSignal->taskSignal_, nullptr);
    ASSERT_NE(fsTaskSignal->signalListener_, nullptr);

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_003";
}

/**
 * @tc.name: FsTaskSignalTest_Cancel_001
 * @tc.desc: Test function of FsTaskSignal::Cancel interface for FAILURE when taskSignal_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Cancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Cancel_001";

    FsTaskSignal fsTaskSignal;
    fsTaskSignal.taskSignal_ = nullptr;

    auto res = fsTaskSignal.Cancel();

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Cancel_001";
}

/**
 * @tc.name: FsTaskSignalTest_Cancel_002
 * @tc.desc: Test function of FsTaskSignal::Cancel interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Cancel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Cancel_002";

    FsTaskSignal fsTaskSignal;

    auto res = fsTaskSignal.Cancel();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Cancel_002";
}

/**
 * @tc.name: FsTaskSignalTest_OnCancel_001
 * @tc.desc: Test function of FsTaskSignal::OnCancel interface for FAILURE when taskSignal_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_OnCancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_OnCancel_001";

    FsTaskSignal fsTaskSignal;
    fsTaskSignal.taskSignal_ = nullptr;

    auto res = fsTaskSignal.OnCancel();

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_OnCancel_001";
}

/**
 * @tc.name: FsTaskSignalTest_OnCancel_002
 * @tc.desc: Test function of FsTaskSignal::OnCancel interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_OnCancel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_OnCancel_002";

    FsTaskSignal fsTaskSignal;

    auto res = fsTaskSignal.OnCancel();

    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_OnCancel_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
