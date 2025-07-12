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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/xattr.h>

#include "system_mock.h"
#include "xattr_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class XattrCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<SystemMock> sys = nullptr;
};

filesystem::path XattrCoreMockTest::tempFilePath;

void XattrCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = "/data/local/tmp/xattr_test_file.txt";
    ofstream tempfile(tempFilePath);
    tempfile << "Test content\n123\n456";
    tempfile.close();
    sys = make_shared<SystemMock>();
    System::ins = sys;
}

void XattrCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove(tempFilePath);
    System::ins = nullptr;
    sys = nullptr;
}

void XattrCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void XattrCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: XattrCoreMockTest_DoSetXattr_001
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreMockTest, XattrCoreMockTest_DoSetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreMockTest-begin XattrCoreMockTest_DoSetXattr_001";

    string path = tempFilePath.string();
    string key = "test_key";
    string value = "test_value";

    EXPECT_CALL(*sys, setxattr(_, _, _, _, _)).WillOnce(SetErrnoAndReturn(EIO, -1));
    auto ret = XattrCore::DoSetXattr(path, key, value);
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "XattrCoreMockTest-end XattrCoreMockTest_DoSetXattr_001";
}

/**
 * @tc.name: XattrCoreMockTest_DoSetXattr_002
 * @tc.desc: Test function of XattrCore::DoSetXattr interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreMockTest, XattrCoreMockTest_DoSetXattr_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreMockTest-begin XattrCoreMockTest_DoSetXattr_002";

    string path = tempFilePath.string();
    string key = "test_key";
    string value = "test_value";

    EXPECT_CALL(*sys, setxattr(_, _, _, _, _)).WillOnce(Return(0));
    auto ret = XattrCore::DoSetXattr(path, key, value);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "XattrCoreMockTest-end XattrCoreMockTest_DoSetXattr_002";
}

/**
 * @tc.name: XattrCoreMockTest_DoGetXattr_001
 * @tc.desc: Test function of XattrCore::DoGetXattr interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreMockTest, XattrCoreMockTest_DoGetXattr_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreMockTest-begin XattrCoreMockTest_DoGetXattr_001";

    string path = tempFilePath.string();
    string key = "test_key";

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillRepeatedly(SetErrnoAndReturn(EIO, -1));
    auto ret = XattrCore::DoGetXattr(path, key);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "XattrCoreMockTest-end XattrCoreMockTest_DoGetXattr_001";
}

/**
 * @tc.name: XattrCoreMockTest_DoGetXattr_002
 * @tc.desc: Test function of XattrCore::DoGetXattr interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreMockTest, XattrCoreMockTest_DoGetXattr_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreMockTest-begin XattrCoreMockTest_DoGetXattr_002";

    string path = tempFilePath.string();
    string key = "test_key";

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(1)).WillOnce(SetErrnoAndReturn(EIO, -1));
    auto ret = XattrCore::DoGetXattr(path, key);
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "XattrCoreMockTest-end XattrCoreMockTest_DoGetXattr_002";
}

/**
 * @tc.name: XattrCoreMockTest_DoGetXattr_003
 * @tc.desc: Test function of XattrCore::DoGetXattr interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(XattrCoreMockTest, XattrCoreMockTest_DoGetXattr_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrCoreMockTest-begin XattrCoreMockTest_DoGetXattr_003";

    string path = tempFilePath.string();
    string key = "test_key";

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(1)).WillOnce(Return(1));
    auto ret = XattrCore::DoGetXattr(path, key);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "XattrCoreMockTest-end XattrCoreMockTest_DoGetXattr_003";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test