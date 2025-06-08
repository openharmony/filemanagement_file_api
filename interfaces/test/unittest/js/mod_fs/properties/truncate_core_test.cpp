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

#include "truncate_core.h"

#include <gtest/gtest.h>


namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class TruncateCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TruncateCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void TruncateCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TruncateCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TruncateCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_001
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_001";
    FileInfo fileInfo;
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(-1);

    auto res = TruncateCore::DoTruncate(fileInfo);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_001";
}

/**
 * @tc.name: TruncateCoreTest_DoTruncate_002
 * @tc.desc: Test function of RmdirCore::DoTruncate interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TruncateCoreTest, TruncateCoreTest_DoTruncate_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TruncateCoreTest-begin TruncateCoreTest_DoTruncate_002";
    FileInfo fileInfo;
    std::optional<int64_t> len = std::make_optional(static_cast<int64_t>(-1));
    fileInfo.fdg = std::make_unique<DistributedFS::FDGuard>(1);

    auto res = TruncateCore::DoTruncate(fileInfo, len);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "TruncateCoreTest-end TruncateCoreTest_DoTruncate_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test