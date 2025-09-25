/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "fs_stat.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fs_stat_entity.h"
#include "securec.h"
#include "system_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsStatMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FsStatMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "FsStatMockTest");
    SystemMock::EnableMock();
}

void FsStatMockTest::TearDownTestCase(void)
{
    SystemMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsStatMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsStatMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

inline const int32_t EXPECTED_FD = 1;

/**
 * @tc.name: FsStatMockTest_GetLocation_001
 * @tc.desc: Test function of GetLocation() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatMockTest, FsStatMockTest_GetLocation_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatMockTes-begin FsStatMockTest_GetLocation_001";

    unique_ptr<StatEntity> statEntity;
    unique_ptr<FsStat> fsStat;
    statEntity = make_unique<StatEntity>();
    statEntity->fileInfo_ = make_unique<FileInfo>();
    statEntity->fileInfo_->isPath = true;
    int length = 100;
    string testPath = "/test/stat_path";
    statEntity->fileInfo_->path = make_unique<char[]>(length);
    strncpy_s(statEntity->fileInfo_->path.get(), length, testPath.c_str(), testPath.size());
    statEntity->fileInfo_->path.get()[99] = '\0';
    fsStat = make_unique<FsStat>(move(statEntity));

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(fsStat->GetLocation(), 1);
    testing::Mock::VerifyAndClearExpectations(sys.get());

    GTEST_LOG_(INFO) << "FsStatMockTes-end FsStatMockTest_GetLocation_001";
}

/**
 * @tc.name: FsStatMockTest_GetLocation_002
 * @tc.desc: Test function of GetLocation() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatMockTest, FsStatMockTest_GetLocation_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatMockTes-begin FsStatMockTest_GetLocation_002";

    unique_ptr<StatEntity> statEntity;
    unique_ptr<FsStat> fsStat;
    statEntity = make_unique<StatEntity>();
    statEntity->fileInfo_ = make_unique<FileInfo>();
    statEntity->fileInfo_->isPath = false;
    const int fdValue = EXPECTED_FD;
    const bool isClosed = false;
    statEntity->fileInfo_->fdg = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    fsStat = make_unique<FsStat>(move(statEntity));

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, fgetxattr(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(fsStat->GetLocation(), 1);
    testing::Mock::VerifyAndClearExpectations(sys.get());

    GTEST_LOG_(INFO) << "FsStatMockTes-end FsStatMockTest_GetLocation_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test