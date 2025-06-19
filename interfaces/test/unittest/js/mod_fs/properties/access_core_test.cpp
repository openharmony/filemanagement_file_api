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
#include <sys/xattr.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class AccessCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    const string CLOUDDISK_FILE_PREFIX = "/data/storage/el2/cloud";
    const string DISTRIBUTED_FILE_PREFIX = "/data/storage/el2/distributedfiles";
    const string CLOUD_FILE_LOCATION = "user.cloud.location";
    const string POSITION_LOCAL = "1";
    const string POSITION_BOTH = "2";
};

void AccessCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AccessCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AccessCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AccessCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

// 递归创建多级目录的辅助函数
bool CreateDirectoryRecursive(const std::string& path) {
    if (path.empty()) {
        return false;
    }

    size_t pos = 0;
    std::string dir;
    if (path[0] == '/') {
        dir += '/';
        pos++;
    }

    while ((pos = path.find('/', pos)) != std::string::npos) {
        dir = path.substr(0, pos++);
        if (dir.empty()) continue;
        if (mkdir(dir.c_str(), 0755) == -1) {
            if (errno != EEXIST) {
                return false;
            }
        }
    }

    if (mkdir(path.c_str(), 0755) == -1 && errno != EEXIST) {
        return false;
    }
    return true;
}

/**
 * @tc.name: AccessCoreTest_DoAccess_001
 * @tc.desc: Test function of AccessCore::DoAccess interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_001";

    std::string path;
    std::optional<AccessModeType> mode;

    auto res = AccessCore::DoAccess(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_001";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_002
 * @tc.desc: Test function of AccessCore::DoAccess interface for ERROR.
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
 * @tc.desc: Test function of AccessCore::DoAccess interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_003";

    std::string path = "test";
    std::optional<AccessModeType> mode = std::make_optional<AccessModeType>(AccessModeType::ERROR);

    auto res = AccessCore::DoAccess(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_003";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_004
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_004";

    std::string path = CLOUDDISK_FILE_PREFIX;
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    auto res = AccessCore::DoAccess(path, mode, flag);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_004";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_005
 * @tc.desc: Test function of AccessCore::DoAccess interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_005";

    std::string path = CLOUDDISK_FILE_PREFIX;
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    ASSERT_TRUE(CreateDirectoryRecursive(path));
    auto re = setxattr(path.c_str(), CLOUD_FILE_LOCATION.c_str(), POSITION_LOCAL.c_str(), POSITION_LOCAL.size(), 0);
    ASSERT_NE(re, -1);

    auto res = AccessCore::DoAccess(path, mode, flag);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_005";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_006
 * @tc.desc: Test function of AccessCore::DoAccess interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_006";

    std::string path = "test";
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    auto res = AccessCore::DoAccess(path, mode, flag);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_006";
}

/**
 * @tc.name: AccessCoreTest_DoAccess_007
 * @tc.desc: Test function of AccessCore::DoAccess interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(AccessCoreTest, AccessCoreTest_DoAccess_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessCoreTest-begin AccessCoreTest_DoAccess_007";

    std::string path = CLOUDDISK_FILE_PREFIX;
    AccessModeType mode = AccessModeType::EXIST;
    AccessFlag flag = LOCAL_FLAG;

    ASSERT_TRUE(CreateDirectoryRecursive(path));
    auto re = setxattr(path.c_str(), CLOUD_FILE_LOCATION.c_str(), POSITION_BOTH.c_str(), POSITION_BOTH.size(), 0);
    ASSERT_NE(re, -1);

    auto res = AccessCore::DoAccess(path, mode, flag);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "AccessCoreTest-end AccessCoreTest_DoAccess_007";
}

} // OHOS::FileManagement::ModuleFileIO::Test