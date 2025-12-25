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

#include "access_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/xattr.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static const string CLOUDDISK_FILE_PREFIX = "/data/storage/el2/cloud";
static const string CLOUD_FILE_LOCATION = "user.cloud.location";
static const string POSITION_LOCAL = "1";
static const string POSITION_BOTH = "3";

class AccessCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string localTestDir = FileUtils::testRootDir + "/AccessCoreTest";
    const string cloudTestDir = CLOUDDISK_FILE_PREFIX + "/AccessCoreTest";
};

void AccessCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "AccessCoreTest");
}

void AccessCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void AccessCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(localTestDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(cloudTestDir, true));
}

void AccessCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(localTestDir));
    ASSERT_TRUE(FileUtils::RemoveAll(cloudTestDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_001
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS when checking whether the path exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_001";

    std::string path = localTestDir + "/AccessCoreTest_DoAccess_001.txt";

    auto res = AccessCore::DoAccess(path);
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_FALSE(exists);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_001";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_002
 * @tc.desc: Test function of AccessCore::DoAccess interface for FAILURE when path is empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_002";

    std::string path = "";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = DEFAULT_FLAG;

    auto res = AccessCore::DoAccess(path, mode, flag);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_002";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_003
 * @tc.desc: Test function of AccessCore::DoAccess interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_003";

    std::string path = localTestDir + "/AccessCoreTest_DoAccess_003";
    std::optional<AccessModeType> mode = std::make_optional<AccessModeType>(AccessModeType::ERROR);

    auto res = AccessCore::DoAccess(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_003";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_004
 * @tc.desc: Test function of AccessCore::DoAccess interface for FAILURE when get cloud file location fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_004";

    std::string path = cloudTestDir + "/AccessCoreTest_DoAccess_004";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));

    auto res = AccessCore::DoAccess(path, mode, flag);
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_FALSE(exists);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_004";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_005
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS when cloud file is only in local.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_005";

    std::string path = cloudTestDir + "/AccessCoreTest_DoAccess_005";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));
    auto re = setxattr(path.c_str(), CLOUD_FILE_LOCATION.c_str(), POSITION_LOCAL.c_str(), POSITION_LOCAL.size(), 0);
    ASSERT_NE(re, -1);

    auto res = AccessCore::DoAccess(path, mode, flag);
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_TRUE(exists);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_005";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_006
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS when local file not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_006";

    std::string path = localTestDir + "/AccessCoreTest_DoAccess_006";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    auto res = AccessCore::DoAccess(path, mode, flag);
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_FALSE(exists);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_006";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_007
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS when cloud file is both in local and cloud.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_007";

    std::string path = cloudTestDir + "/AccessCoreTest_DoAccess_007";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));
    auto re = setxattr(path.c_str(), CLOUD_FILE_LOCATION.c_str(), POSITION_BOTH.c_str(), POSITION_BOTH.size(), 0);
    ASSERT_NE(re, -1);

    auto res = AccessCore::DoAccess(path, mode, flag);
    ASSERT_TRUE(res.IsSuccess());
    bool exists = res.GetData().value();
    EXPECT_TRUE(exists);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_007";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test