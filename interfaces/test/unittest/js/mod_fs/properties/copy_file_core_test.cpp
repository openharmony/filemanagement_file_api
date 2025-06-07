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

#include "copy_file_core.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyFileCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CopyFileCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CopyFileCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyFileCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyFileCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_001
 * @tc.desc: Test function of CopyFileCore::ValidMode interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_001";

    FileInfo src;
    FileInfo dest;
    optional<int32_t> mode = std::make_optional(1);

    auto res = CopyFileCore::DoCopyFile(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_001";
}

/**
 * @tc.name: CopyFileCoreMockTest_DoCopyFile_002
 * @tc.desc: Test function of CopyFileCore::ValidMode interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreMockTest, CopyFileCoreMockTest_DoCopyFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-begin CopyFileCoreMockTest_DoCopyFile_002";

    FileInfo src;
    FileInfo dest;
    src.isPath = true;
    dest.isPath = true;
    src.path = std::make_unique<char[]>(1);
    dest.path = std::make_unique<char[]>(1);

    auto res = CopyFileCore::DoCopyFile(src, dest);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreMockTest-end CopyFileCoreMockTest_DoCopyFile_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test