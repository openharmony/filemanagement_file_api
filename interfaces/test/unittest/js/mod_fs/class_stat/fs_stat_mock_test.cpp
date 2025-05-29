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
#include "../properties/mock/system_mock.h"
#include "fs_stat_entity.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
    static inline shared_ptr<SystemMock> sys = nullptr;
};

void FsStatMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    sys = std::make_shared<SystemMock>();
    System::ins = sys;
}

void FsStatMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    System::ins = nullptr;
    sys = nullptr;
}

void FsStatMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsStatMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

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

    std::unique_ptr<StatEntity> statEntity;
    std::unique_ptr<FsStat> fsStat;
    statEntity = std::make_unique<StatEntity>();
    statEntity->fileInfo_ = std::make_unique<FileInfo>();
    statEntity->fileInfo_->isPath = true;
    statEntity->fileInfo_->path = std::make_unique<char[]>(100);
    strcpy(statEntity->fileInfo_->path.get(), "/test/path");
    fsStat = std::make_unique<FsStat>(std::move(statEntity));

    EXPECT_CALL(*sys, getxattr(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(fsStat->GetLocation(), 1);

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

    std::unique_ptr<StatEntity> statEntity;
    std::unique_ptr<FsStat> fsStat;
    statEntity = std::make_unique<StatEntity>();
    statEntity->fileInfo_ = std::make_unique<FileInfo>();
    statEntity->fileInfo_->isPath = false;
    const int fdValue = 3;
    const bool isClosed = false;
    statEntity->fileInfo_->fdg = std::make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    fsStat = std::make_unique<FsStat>(std::move(statEntity));

    EXPECT_CALL(*sys, fgetxattr(_, _, _, _)).WillOnce(Return(1));
    EXPECT_EQ(fsStat->GetLocation(), 1);

    GTEST_LOG_(INFO) << "FsStatMockTes-end FsStatMockTest_GetLocation_002";
}

}