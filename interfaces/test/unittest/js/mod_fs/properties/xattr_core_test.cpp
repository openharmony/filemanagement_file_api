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

#include "xattr_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>
#include <sys/xattr.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static constexpr size_t MAX_XATTR_SIZE = 4096;

class XattrCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/XattrCoreTest";
};

void XattrCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "XattrCoreTest");
}

void XattrCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void XattrCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void XattrCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: XattrCoreTest_DoSetXattr_001
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for FAILURE when the xattr value is too long and over
 * MAX_XATTR_SIZE limit.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoSetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoSetXattr_001";

    auto path = testDir + "/XattrCoreTest_DoSetXattr_001.txt";
    std::string key = "test_key";
    std::string value(MAX_XATTR_SIZE + 1, 'a'); // over limit

    auto ret = XattrCore::DoSetXattr(path, key, value);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoSetXattr_001";
}

/**
 * @tc.name: XattrCoreTest_DoSetXattr_002
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for FAILURE when the xattr key is too long and over
 * MAX_XATTR_SIZE limit.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoSetXattr_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoSetXattr_002";

    auto path = testDir + "/XattrCoreTest_DoSetXattr_002.txt";
    std::string key(MAX_XATTR_SIZE + 1, 'a');
    std::string value = "test_value";

    auto ret = XattrCore::DoSetXattr(path, key, value);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoSetXattr_002";
}

/**
 * @tc.name: XattrCoreTest_DoGetXattr_001
 * @tc.desc: Test function of XattrCore::DoGetXattr interface for SUCCESS when the file is not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoGetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoGetXattr_001";

    auto path = testDir + "/XattrCoreTest_DoGetXattr_001_non_exitent.txt";
    string key = "test_key";

    auto ret = XattrCore::DoGetXattr(path, key);

    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value(), "");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoGetXattr_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test