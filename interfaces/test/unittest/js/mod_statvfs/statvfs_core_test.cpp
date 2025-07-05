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

#include "statvfs_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/statvfs.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class StatvFsCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StatvFsCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t fd = open("/data/test/statvfs_test.txt", O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
        ASSERT_TRUE(false);
    }
    close(fd);
}

void StatvFsCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    rmdir("/data/test/statvfs_test.txt");
}

void StatvFsCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatvFsCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetFreeSize_001
 * @tc.desc: Test function of DoGetFreeSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetFreeSize_001";

    struct statvfs diskInfo;
    diskInfo.f_bsize = 2;
    diskInfo.f_bfree = 1;

    auto result = ModuleStatvfs::StatvfsCore::DoGetFreeSize("/data/test/statvfs_test.txt");
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetFreeSize_001";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetFreeSize_002
 * @tc.desc: Test function of DoGetFreeSize interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetFreeSize_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetFreeSize_002";

    auto result = ModuleStatvfs::StatvfsCore::DoGetFreeSize("/test/path");
    EXPECT_EQ(result.IsSuccess(), false);
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetFreeSize_002";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_003
 * @tc.desc: Test function of DoGetTotalSize interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_003";

    struct statvfs diskInfo;
    diskInfo.f_bsize = 2;
    diskInfo.f_blocks = 1;

    auto result = ModuleStatvfs::StatvfsCore::DoGetTotalSize("/data/test/statvfs_test.txt");
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_003";
}

/**
 * @tc.name: StatvFsCoreTest_DoGetTotalSize_004
 * @tc.desc: Test function of DoGetTotalSize interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatvFsCoreTest, StatvFsCoreTest_DoGetTotalSize_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatvFsCoreTest-begin StatvFsCoreTest_DoGetTotalSize_004";

    auto result = ModuleStatvfs::StatvfsCore::DoGetTotalSize("/test/path");
    EXPECT_EQ(result.IsSuccess(), false);
    auto err = result.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "StatvFsCoreTest-end StatvFsCoreTest_DoGetTotalSize_004";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
