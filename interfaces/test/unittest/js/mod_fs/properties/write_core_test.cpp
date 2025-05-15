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
#include "mock/uv_fs_mock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class WriteCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WriteCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void WriteCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void WriteCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void WriteCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: WriteCoreTest_DoWrite1_001
 * @tc.desc: Test function of WriteCore::DoWrite1 interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite1_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite1_001";
    int32_t fd = -1;
    string buffer;
    auto res = WriteCore::DoWrite(fd, buffer);

    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite1_001";
}

/**
 * @tc.name: WriteCoreTest_DoWrite1_002
 * @tc.desc: Test function of WriteCore::DoWrite3 interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite1_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite1_002";
    int32_t fd = 1;
    string buffer;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = WriteCore::DoWrite(fd, buffer);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite1_002";
}

/**
 * @tc.name: WriteCoreTest_DoWrite1_003
 * @tc.desc: Test function of WriteCore::DoWrite3 interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite1_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite1_003";
    int32_t fd = 1;
    string buffer;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(1));

    auto res = WriteCore::DoWrite(fd, buffer);
    EXPECT_EQ(res.IsSuccess(), true);
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite1_003";
}

/**
 * @tc.name: WriteCoreTest_DoWrite2_001
 * @tc.desc: Test function of WriteCore::DoWrite2 interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite2_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite2_001";
    int32_t fd = -1;
    string buffer;

    auto res = WriteCore::DoWrite(fd, buffer);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite2_001";
}

/**
 * @tc.name: WriteCoreTest_DoWrite2_002
 * @tc.desc: Test function of WriteCore::DoWrite2 interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WriteCoreTest, WriteCoreTest_DoWrite2_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteCoreTest-begin WriteCoreTest_DoWrite2_002";
    int32_t fd = -1;
    string buffer;
    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;

    EXPECT_CALL(*uv, uv_fs_write(_, _, _, _, _, _, _)).WillOnce(Return(-1));

    auto res = WriteCore::DoWrite(fd, buffer);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "WriteCoreTest-end WriteCoreTest_DoWrite2_002";
}


} // namespace OHOS::FileManagement::ModuleFileIO::Test