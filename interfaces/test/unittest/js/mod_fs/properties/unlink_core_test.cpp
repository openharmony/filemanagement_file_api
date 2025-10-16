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
#include <sys/stat.h>
#include <sys/types.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static string g_dirPath = "/data/storage/el2/UninkCoreTestDir";
static string g_filePath = "/data/storage/el2/UninkCoreTestFile.txt";

class UnlinkCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UnlinkCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UnlinkCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UnlinkCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UnlinkCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_001
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_001";

    auto res = UnlinkCore::DoUnlink(g_dirPath);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_001";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_002
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_002";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    auto result = mkdir(g_dirPath.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "UnlinkCoreTest_DoUnlink_002 mkdir failed! ret: " << result;
    }

    auto res = UnlinkCore::DoUnlink(g_dirPath);

    result = rmdir(g_dirPath.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "UnlinkCoreTest_DoUnlink_002 rmdir failed! ret: " << result;
    }

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_002";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_003
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_003";

    auto res = UnlinkCore::DoUnlink(g_filePath);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_003";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_004
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for Successed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_004";
    auto file = open(g_filePath.c_str(), O_CREAT | O_RDWR);
    if (file < 0) {
        GTEST_LOG_(ERROR) << "UnlinkCoreTest_DoUnlink_004 create failed! ret: " << file;
        EXPECT_GT(file, 0);
    }
    close(file);

    auto res = UnlinkCore::DoUnlink(g_filePath);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_004";
}

/**
 * @tc.name: UnlinkCoreTest_DoUnlink_005
 * @tc.desc: Test function of UnlinkCore::DoUnlink interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UnlinkCoreTest, UnlinkCoreTest_DoUnlink_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkCoreTest-begin UnlinkCoreTest_DoUnlink_005";

    auto res = UnlinkCore::DoUnlink("hello world");
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UnlinkCoreTest-end UnlinkCoreTest_DoUnlink_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
