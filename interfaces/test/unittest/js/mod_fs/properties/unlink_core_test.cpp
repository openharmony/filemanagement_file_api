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

#include "unlink_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UnlinkCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/UnlinkCoreTest";
};

void UnlinkCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "UnlinkCoreTest");
}

void UnlinkCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UnlinkCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void UnlinkCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_001
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for FAILURE when file is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_001";

    auto path = testDir + "/UnlinkCoreTest_DoUnlink_001.txt";

    auto res = UnlinkCore::DoUnlink(path);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_001";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_002
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for FAILURE when path is a directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_002";

    auto path = testDir + "/UnlinkCoreTest_DoUnlink_002";
    ASSERT_TRUE(FileUtils::CreateDirectories(path));

    auto res = UnlinkCore::DoUnlink(path);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900019);
    EXPECT_EQ(err.GetErrMsg(), "Is a directory");

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_002";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_003
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_003";

    auto path = testDir + "/UnlinkCoreTest_DoUnlink_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto res = UnlinkCore::DoUnlink(path);
    EXPECT_TRUE(res.IsSuccess());
    EXPECT_FALSE(FileUtils::Exists(path));

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
