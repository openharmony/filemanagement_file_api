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

#include "utimes_core.h"
#include "mock/uv_fs_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UtimesCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtimesCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UtimesCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UtimesCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UtimesCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: UtimesCoreTest_DoUtimes_001
 * @tc.desc: Test function of UtimesCore::DoUtimes interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UtimesCoreTest, UtimesCoreTest_DoUtimes_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UtimesCoreTest-begin UtimesCoreTest_DoUtimes_001";
    string path;
    double mtime = -1;
    auto res = UtimesCore::DoUtimes(path, mtime);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UtimesCoreTest-end UtimesCoreTest_DoUtimes_001";
}

/**
 * @tc.name: UtimesCoreTest_DoUtimes_002
 * @tc.desc: Test function of UtimesCore::DoUtimes interface for Failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UtimesCoreTest, UtimesCoreTest_DoUtimes_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UtimesCoreTest-begin UtimesCoreTest_DoUtimes_002";
    string path;
    double mtime = 1;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = UtimesCore::DoUtimes(path, mtime);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UtimesCoreTest-end UtimesCoreTest_DoUtimes_002";
}

/**
 * @tc.name: UtimesCoreTest_DoUtimes_003
 * @tc.desc: Test function of UtimesCore::DoUtimes interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UtimesCoreTest, UtimesCoreTest_DoUtimes_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UtimesCoreTest-begin UtimesCoreTest_DoUtimes_003";
    string path;
    double mtime = 1;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uv, uv_fs_utime(_, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = UtimesCore::DoUtimes(path, mtime);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "UtimesCoreTest-end UtimesCoreTest_DoUtimes_003";
}

/**
 * @tc.name: UtimesCoreTest_DoUtimes_004
 * @tc.desc: Test function of UtimesCore::DoUtimes interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(UtimesCoreTest, UtimesCoreTest_DoUtimes_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UtimesCoreTest-begin UtimesCoreTest_DoUtimes_004";
    string path;
    double mtime = 1;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uv, uv_fs_utime(_, _, _, _, _, _)).WillOnce(Return(1));

    auto res = UtimesCore::DoUtimes(path, mtime);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "UtimesCoreTest-end UtimesCoreTest_DoUtimes_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
