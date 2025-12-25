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

#include "fs_task_signal.h"

#include <gtest/gtest.h>

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
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void FsTaskSignalTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
}

void FsTaskSignalTest::TearDownTestSuite(void)
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsTaskSignalTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsTaskSignalTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_001
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_001";

    FsTaskSignal fsTaskSignal;
    shared_ptr<TaskSignal> taskSignal = nullptr;
    shared_ptr<TaskSignalListener> signalListener = nullptr;

    auto res = fsTaskSignal.Constructor(taskSignal, signalListener);

    EXPECT_EQ(taskSignal, nullptr);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_001";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_002
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_002";

    FsTaskSignal fsTaskSignal;
    shared_ptr<TaskSignal> taskSignal = std::make_shared<TaskSignal>();
    shared_ptr<TaskSignalListener> signalListener = nullptr;

    auto res = fsTaskSignal.Constructor(taskSignal, signalListener);

    EXPECT_NE(taskSignal, nullptr);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_002";
}

/**
 * @tc.name: FsTaskSignalTest_Constructor_003
 * @tc.desc: Test function of FsTaskSignal::Constructor interface for True.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Constructor_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Constructor_003";

    FsTaskSignal fsTaskSignal;
    shared_ptr<TaskSignal> taskSignal = std::make_shared<TaskSignal>();
    shared_ptr<TaskSignalListener> signalListener = std::make_shared<Assistant>();

    auto res = fsTaskSignal.Constructor(taskSignal, signalListener);

    EXPECT_NE(signalListener, nullptr);
    EXPECT_EQ(res.IsSuccess(), true);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Constructor_003";
}

/**
 * @tc.name: FsTaskSignalTest_Cancel_001
 * @tc.desc: Test function of FsTaskSignal::Cancel interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Cancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Cancel_001";

    FsTaskSignal fsTaskSignal;

    auto res = fsTaskSignal.Cancel();

    EXPECT_NE(fsTaskSignal.taskSignal_, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Cancel_001";
}

/**
 * @tc.name: FsTaskSignalTest_Cancel_002
 * @tc.desc: Test function of FsTaskSignal::Cancel interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_Cancel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_Cancel_002";

    FsTaskSignal fsTaskSignal;
    fsTaskSignal.taskSignal_ = std::make_shared<TaskSignal>();

    auto res = fsTaskSignal.Cancel();

    EXPECT_NE(fsTaskSignal.taskSignal_, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_Cancel_002";
}

/**
 * @tc.name: FsTaskSignalTest_OnCancel_001
 * @tc.desc: Test function of FsTaskSignal::OnCancel interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_OnCancel_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_OnCancel_001";

    FsTaskSignal fsTaskSignal;

    auto res = fsTaskSignal.OnCancel();

    EXPECT_NE(fsTaskSignal.taskSignal_, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_OnCancel_001";
}

/**
 * @tc.name: FsTaskSignalTest_OnCancel_002
 * @tc.desc: Test function of FsTaskSignal::OnCancel interface for False.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsTaskSignalTest, FsTaskSignalTest_OnCancel_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin FsTaskSignalTest_OnCancel_002";

    FsTaskSignal fsTaskSignal;
    fsTaskSignal.taskSignal_ = std::make_shared<TaskSignal>();

    auto res = fsTaskSignal.OnCancel();

    EXPECT_NE(fsTaskSignal.taskSignal_, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end FsTaskSignalTest_OnCancel_002";
}

} // OHOS::FileManagement::ModuleFileIO::Test
