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

#include "copy_file_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyFileCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void CopyFileCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "CopyFileCoreTest");
}

void CopyFileCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void CopyFileCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyFileCoreTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyFileCoreTest_DoCopyFile_001
 * @tc.desc: Test function of CopyFileCore::DoCopyFile interface for FAILURE when mode is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreTest, CopyFileCoreTest_DoCopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreTest-begin CopyFileCoreTest_DoCopyFile_001";

    auto [succSrc, src] = GenerateFileInfoFromPath("fakePath/CopyFileCoreTest_DoCopyFile_001_src.txt");
    ASSERT_TRUE(succSrc);
    auto [succDest, dest] = GenerateFileInfoFromPath("fakePath/CopyFileCoreTest_DoCopyFile_001_src.txt");
    ASSERT_TRUE(succDest);

    optional<int32_t> mode = std::make_optional(1); // only support 0

    auto res = CopyFileCore::DoCopyFile(src, dest, mode);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "CopyFileCoreTest-end CopyFileCoreTest_DoCopyFile_001";
}

/**
 * @tc.name: CopyFileCoreTest_DoCopyFile_002
 * @tc.desc: Test function of CopyFileCore::DoCopyFile interface for FAILURE when paths are all empty.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreTest, CopyFileCoreTest_DoCopyFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreTest-begin CopyFileCoreTest_DoCopyFile_002";

    auto [succSrc, src] = GenerateFileInfoFromPath("");
    ASSERT_TRUE(succSrc);
    auto [succDest, dest] = GenerateFileInfoFromPath("");
    ASSERT_TRUE(succDest);

    auto res = CopyFileCore::DoCopyFile(src, dest);

    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "CopyFileCoreTest-end CopyFileCoreTest_DoCopyFile_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test