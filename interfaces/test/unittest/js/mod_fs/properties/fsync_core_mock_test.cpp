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

#include "fsync_core.h"
#include "uv_fs_mock.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsyncCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock>  uvfs = nullptr;
};

void FsyncCoreMockTest::SetUpTestCase(void)
{
    uvfs = std::make_shared<UvfsMock>();
    Uvfs::ins = uvfs;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsyncCoreMockTest::TearDownTestCase(void)
{
    Uvfs::ins = nullptr;
    uvfs = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsyncCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsyncCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsyncCoreMockTest_DoFsync_001
 * @tc.desc: Test function of RenameCore::DoFsync interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncCoreMockTest, FsyncCoreMockTest_DoFsync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncCoreMockTest-begin FsyncCoreMockTest_DoFsync_001";

    EXPECT_CALL(*uvfs, uv_fs_fsync(_, _, _, _)).WillOnce(Return(-1));

    auto res = FsyncCore::DoFsync(1);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsyncCoreMockTest-end FsyncCoreMockTest_DoFsync_001";
}

/**
 * @tc.name: FsyncCoreMockTest_DoFsync_002
 * @tc.desc: Test function of RenameCore::DoFsync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsyncCoreMockTest, FsyncCoreMockTest_DoFsync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsyncCoreMockTest-begin FsyncCoreMockTest_DoFsync_002";

    EXPECT_CALL(*uvfs, uv_fs_fsync(_, _, _, _)).WillOnce(Return(1));

    auto res = FsyncCore::DoFsync(1);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsyncCoreMockTest-end FsyncCoreMockTest_DoFsync_002";
}


} // namespace OHOS::FileManagement::ModuleFileIO::Test