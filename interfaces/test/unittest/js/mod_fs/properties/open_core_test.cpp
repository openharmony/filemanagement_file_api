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

#include <gtest/gtest.h>

#include "open_core.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class OpenCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OpenCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void OpenCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void OpenCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void OpenCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_001
 * @tc.desc: Test function of OpenCore::DoOpen interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_001";

    string path = "/test/open_test.txt";
    int32_t mode = -1;
    
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_001";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_002
 * @tc.desc: Test function of OpenCore::DoOpen interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_002";

    string path = "/test/open_test.txt";
    int32_t mode = 3;
    
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_002";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_003
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_003";

    string path = "file://media/open_test.jpg";
    int32_t mode = 0;
    
    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_003";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_004
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_004";

    string path = "file://docs/open_test.pdf";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_004";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_005
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_005";

    string path = "content://com.example.provider/open_test.txt";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_005";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_006
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_006";

    string path = "datashare://media/open_test.jpg";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_006";
}

/**
 * @tc.name: OpenCoreTest_DoOpen_007
 * @tc.desc: Test function of OpenCore::DoOpen interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(OpenCoreTest, OpenCoreTest_DoOpen_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenCoreTest-begin OpenCoreTest_DoOpen_007";

    string path = "invalid://path/dir_test";
    int32_t mode = 0;

    auto res = OpenCore::DoOpen(path, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "OpenCoreTest-end OpenCoreTest_DoOpen_007";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test