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

#include "rmdir_core.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RmdirCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RmdirCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void RmdirCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RmdirCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void RmdirCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_001
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_001";
    std::string fpath;
    auto res = RmdirentCore::DoRmdirent(fpath);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_001";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_002
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_002";
    std::string fpath = "invalid?path";
    auto res = RmdirentCore::DoRmdirent(fpath);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_002";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_003
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_003";
    std::string fpath = "/dir";
    auto res = RmdirentCore::DoRmdirent(fpath);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_003";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_004
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_004";

    std::filesystem::create_directories("test_dir");
    std::ofstream("test_dir/test_file.txt") << "test";

    std::filesystem::permissions("test_dir", 
        std::filesystem::perms::owner_write | std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::replace);

    auto res = RmdirentCore::DoRmdirent("test_dir");
    EXPECT_EQ(res.IsSuccess(), true);

    try {
        std::filesystem::permissions("test_dir",
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::replace);
    } catch (...) {}
    std::filesystem::remove_all("test_dir");

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_004";
}

/**
 * @tc.name: RmdirCoreTest_DoRmdirent_005
 * @tc.desc: Test function of RmdirCore::DoRmdirent interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RmdirCoreTest, RmdirCoreTest_DoRmdirent_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmdirCoreTest-begin RmdirCoreTest_DoRmdirent_005";

    std::filesystem::create_directories("test_dir");
    std::ofstream("test_dir/test_file.txt") << "test";

    auto res = RmdirentCore::DoRmdirent("test_dir");
    EXPECT_EQ(res.IsSuccess(), true);

    try {
        std::filesystem::permissions("test_dir",
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::replace);
        } catch (...) {}
    std::filesystem::remove_all("test_dir");

    GTEST_LOG_(INFO) << "RmdirCoreTest-end RmdirCoreTest_DoRmdirent_005";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test