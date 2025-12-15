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

#include "write_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class WriteCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/WriteCoreTest";
};

void WriteCoreTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "WriteCoreTest");
}

void WriteCoreTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WriteCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void WriteCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: WriteCoreTest_DoWrite_001
 * @tc.desc: Test function of WriteCore::DoWrite(StringBuffer) interface for FAILURE when fd < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite_001";

    int32_t fd = -1;
    string buffer;

    auto res = WriteCore::DoWrite(fd, buffer);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite_001";
}

/**
 * @tc.name: WriteCoreTest_DoWrite_002
 * @tc.desc: Test function of WriteCore::DoWrite(ArrayBuffer) interface for FAILURE when fd < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite_002";

    int32_t fd = -1;
    ArrayBuffer buffer(nullptr, 1);

    auto res = WriteCore::DoWrite(fd, buffer);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite_002";
}

/**
 * @tc.name: WriteCoreTest_DoWrite_003
 * @tc.desc: Test function of WriteCore::DoWrite(StringBuffer) interface for FAILURE when offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite_003";

    int32_t fd = 1;
    string buffer;
    std::optional<WriteOptions> options = std::make_optional<WriteOptions>(WriteOptions());
    options->offset = std::make_optional<int64_t>(-1);

    auto res = WriteCore::DoWrite(fd, buffer, options);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite_003";
}

/**
 * @tc.name: WriteCoreTest_DoWrite_004
 * @tc.desc: Test function of WriteCore::DoWrite(ArrayBuffer) interface for FAILURE when GetActualLen fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite_004";

    int32_t fd = 1;
    ArrayBuffer buffer(nullptr, 1);

    std::optional<WriteOptions> options = std::make_optional<WriteOptions>(WriteOptions());
    options->length = std::make_optional<int64_t>(-1);

    auto res = WriteCore::DoWrite(fd, buffer, options);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test