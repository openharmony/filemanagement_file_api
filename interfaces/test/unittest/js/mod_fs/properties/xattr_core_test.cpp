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

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>
#include <sys/xattr.h>

#include "xattr_core.h"

#define MAX_LEN 4096

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class XattrCoreTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path XattrCoreTest::tempFilePath;

void XattrCoreTest::SetUpTestCase(void)
{
    tempFilePath = "/data/local/tmp/xattr_test_file.txt";
    ofstream tempfile(tempFilePath);
    tempfile << "Test content\n123\n456";
    tempfile.close();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void XattrCoreTest::TearDownTestCase(void)
{
    filesystem::remove(tempFilePath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void XattrCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void XattrCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: XattrCoreTest_DoSetXattr_001
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoSetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoSetXattr_001";

    string path = tempFilePath.string();
    std::string key = "test_key";
    std::string value(MAX_LEN + 1, 'a');

    auto ret = XattrCore::DoSetXattr(path, key, value);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    int errCode = err.GetErrNo();
    EXPECT_EQ(errCode, 13900020);
    auto msg = err.GetErrMsg();
    EXPECT_EQ(msg, "Invalid argument");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoSetXattr_001";
}

/**
 * @tc.name: XattrCoreTest_DoSetXattr_002
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoSetXattr_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoSetXattr_002";

    string path = tempFilePath.string();
    std::string key(MAX_LEN + 1, 'a');
    std::string value = "test_value";

    auto ret = XattrCore::DoSetXattr(path, key, value);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    int errCode = err.GetErrNo();
    EXPECT_EQ(errCode, 13900020);
    auto msg = err.GetErrMsg();
    EXPECT_EQ(msg, "Invalid argument");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoSetXattr_002";
}

/**
 * @tc.name: XattrCoreTest_DoGetXattr_001
 * @tc.desc: Test function of XattrCore::DoGetXattr interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreTest, XattrCoreTest_DoGetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreTest-begin XattrCoreTest_DoGetXattr_001";

    std::string path = "/data/local/tmp/nonexistent_file";
    string key = "test_key";

    auto ret = XattrCore::DoGetXattr(path, key);

    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value(), "");

    GTEST_LOG_(INFO) << "XattrCoreTest-end XattrCoreTest_DoGetXattr_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test