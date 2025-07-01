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

class CopyFileCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CopyFileCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CopyFileCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CopyFileCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyFileCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyFileCoreTest_DoCopyFile_001
 * @tc.desc: Test function of CopyFileCore::ValidMode interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreTest, CopyFileCoreTest_DoCopyFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreTest-begin CopyFileCoreTest_DoCopyFile_001";

    FileInfo src;
    FileInfo dest;
    optional<int32_t> mode = std::make_optional(1);

    auto res = CopyFileCore::DoCopyFile(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreTest-end CopyFileCoreTest_DoCopyFile_001";
}

/**
 * @tc.name: CopyFileCoreTest_DoCopyFile_002
 * @tc.desc: Test function of CopyFileCore::ValidMode interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyFileCoreTest, CopyFileCoreTest_DoCopyFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyFileCoreTest-begin CopyFileCoreTest_DoCopyFile_002";

    FileInfo src;
    FileInfo dest;
    src.isPath = true;
    dest.isPath = true;
    src.path = std::make_unique<char[]>(1);
    dest.path = std::make_unique<char[]>(1);

    auto res = CopyFileCore::DoCopyFile(src, dest);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "CopyFileCoreTest-end CopyFileCoreTest_DoCopyFile_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test