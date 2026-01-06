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

#include "mkdtemp_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MkdtempCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string tempDir = FileUtils::defaultTempDir + "/MkdtempCoreMockTest";
};

void MkdtempCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MkdtempCoreMockTest");
}

void MkdtempCoreMockTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MkdtempCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(tempDir, true));
    UvFsMock::EnableMock();
}

void MkdtempCoreMockTest::TearDown()
{
    UvFsMock::DisableMock();
    ASSERT_TRUE(FileUtils::RemoveAll(tempDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MkdtempCoreMockTest_DoMkdtemp_001
 * @tc.desc: Test function of MkdtempCore::DoMkdtemp interface for SUCCESS when executing real invocation.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdtempCoreMockTest, MkdtempCoreMockTest_DoMkdtemp_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-begin MkdtempCoreMockTest_DoMkdtemp_001";

    auto path = tempDir + "/MkdtempCoreMockTest_DoMkdtemp_001_XXXXXX";
    UvFsMock::DisableMock();

    auto ret = MkdtempCore::DoMkdtemp(path);

    ASSERT_TRUE(ret.IsSuccess());
    auto tempPath = ret.GetData().value();
    auto prefix = tempDir + "/MkdtempCoreMockTest_DoMkdtemp_001_";
    EXPECT_NE(tempPath, path);
    EXPECT_EQ(tempPath.find(prefix, 0), 0);

    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-end MkdtempCoreMockTest_DoMkdtemp_001";
}

/**
 * @tc.name: MkdtempCoreMockTest_DoMkdtemp_002
 * @tc.desc: Test function of MkdtempCore::DoMkdtemp interface for FAILURE when uv_fs_mkdtemp fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdtempCoreMockTest, MkdtempCoreMockTest_DoMkdtemp_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-begin MkdtempCoreMockTest_DoMkdtemp_002";

    string path = tempDir + "/MkdtempCoreMockTest_DoMkdtemp_002_XXXXXX";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_mkdtemp(_, _, _, _)).WillOnce(Return(-1));

    auto ret = MkdtempCore::DoMkdtemp(path);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-end MkdtempCoreMockTest_DoMkdtemp_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test