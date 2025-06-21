/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lstat_core.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class LstatCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

void LstatCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    uvMock = make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void LstatCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void LstatCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void LstatCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LstatCoreMockTest_DoLstat_001
 * @tc.desc: Test function of LstatCore::DoLstat interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(LstatCoreMockTest, LstatCoreMockTest_DoLstat_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LstatCoreMockTest-begin LstatCoreMockTest_DoLstat_001";
    
    EXPECT_CALL(*uvMock, uv_fs_lstat(_, _, _, _)).WillOnce(Return(-1));

    auto res = LstatCore::DoLstat("/data/test/lstat.txt");
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "LstatCoreMockTest-end LstatCoreMockTest_DoLstat_001";
}

}