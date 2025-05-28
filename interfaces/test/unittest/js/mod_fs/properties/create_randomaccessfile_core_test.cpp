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

#include "create_randomaccessfile_core.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CreateRandomAccessFileCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CreateRandomAccessFileCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CreateRandomAccessFileCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CreateRandomAccessFileCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CreateRandomAccessFileCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001";

    string path = "/test/path.txt";
    int32_t mode = -5;
    optional<RandomAccessFileOptions> options = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, options);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_001";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002";

    string path = "/test/path.txt";
    int32_t mode = 0;
    RandomAccessFileOptions opts;
    opts.start = -1;
    opts.end = 100;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, opts);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_002";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003";

    string path = "/test/path.txt";
    int32_t mode = 0;
    RandomAccessFileOptions opts;
    opts.start = 10;
    opts.end = -1;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, opts);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_003";
}

/**'
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004";

    int fd = -1;
    optional<RandomAccessFileOptions> opts = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(fd, opts);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_004";
}

/**'
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005";

    string path = "";
    int32_t mode = 0;
    optional<RandomAccessFileOptions> options = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(path, mode, options);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_005";
}

/**
 * @tc.name: CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006
 * @tc.desc: Test function of CreateRandomAccessFileCore::DoCreateRandomAccessFile interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateRandomAccessFileCoreTest, CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Test-begin CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006";

    int fd = 3;
    optional<RandomAccessFileOptions> opts = nullopt;

    auto res = CreateRandomAccessFileCore::DoCreateRandomAccessFile(fd, opts);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "Test-end CreateRandomAccessFileCoreTest_DoCreateRandomAccessFile_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test