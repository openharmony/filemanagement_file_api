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


#include "symlink_core.h"
#include "uv_fs_mock.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SymlinkCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

void SymlinkCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void SymlinkCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void SymlinkCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SymlinkCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SymlinkCoreMockTest_DoSymlink_001
 * @tc.desc: Test function of SymlinkCore::DoSymlink interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SymlinkCoreMockTest, SymlinkCoreMockTest_DoSymlink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SymlinkCore-begin SymlinkCoreMockTest_DoSymlink_001";

    string target;
    string srcPath;

    EXPECT_CALL(*uvMock, uv_fs_symlink(_, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = SymlinkCore::DoSymlink(target, srcPath);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "SymlinkCore-end SymlinkCoreMockTest_DoSymlink_001";
}

/**
 * @tc.name: SymlinkCoreMockTest_DoSymlink_002
 * @tc.desc: Test function of SymlinkCore::DoSymlink interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SymlinkCoreMockTest, SymlinkCoreMockTest_DoSymlink_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SymlinkCore-begin SymlinkCoreMockTest_DoSymlink_002";

    string target;
    string srcPath;

    EXPECT_CALL(*uvMock, uv_fs_symlink(_, _, _, _, _, _)).WillOnce(Return(1));

    auto res = SymlinkCore::DoSymlink(target, srcPath);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "SymlinkCore-end SymlinkCoreMockTest_DoSymlink_002";
}

} // OHOS::FileManagement::ModuleFileIO::Test
