/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "sys_xattr_mock.h"
#include "ut_fs_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsStatMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsStatMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsStatMockTest");
    SysXattrMock::EnableMock();
}

void FsStatMockTest::TearDownTestSuite()
{
    SysXattrMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsStatMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsStatMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsStatMockTest_GetLocation_001
 * @tc.desc: Test function of FsStat::GetLocation interface for SUCCESS when fileInfo is path.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatMockTest, FsStatMockTest_GetLocation_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatMockTes-begin FsStatMockTest_GetLocation_001";

    auto path = "fakePath/FsStatMockTest_GetLocation_001.txt";
    auto [succ, fileInfo] = GenerateFileInfoFromPath(path);
    ASSERT_TRUE(succ);

    auto entity = std::make_unique<StatEntity>();
    entity->fileInfo_ = std::shared_ptr<FileInfo>(&fileInfo, [](FileInfo *) {});
    FsStat fsStat(std::move(entity));

    auto xattrMock = SysXattrMock::GetMock();
    Location expectedLocation = CLOUD;
    EXPECT_CALL(*xattrMock, getxattr(_, _, _, _))
        .WillOnce(DoAll(Invoke([&expectedLocation](const char *path, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = static_cast<char>(expectedLocation + '0');
        }),
            Return(1)));

    auto location = fsStat.GetLocation();

    fsStat.entity->fileInfo_ = nullptr;
    testing::Mock::VerifyAndClearExpectations(xattrMock.get());
    EXPECT_EQ(location, static_cast<int32_t>(expectedLocation));

    GTEST_LOG_(INFO) << "FsStatMockTes-end FsStatMockTest_GetLocation_001";
}

/**
 * @tc.name: FsStatMockTest_GetLocation_002
 * @tc.desc: Test function of FsStat::GetLocation interface for SUCCESS when fileInfo is FD.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatMockTest, FsStatMockTest_GetLocation_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatMockTes-begin FsStatMockTest_GetLocation_002";

    auto fileInfo = std::make_shared<FileInfo>();
    fileInfo->fdg = make_unique<DistributedFS::FDGuard>(1, false);
    auto entity = std::make_unique<StatEntity>();
    entity->fileInfo_ = fileInfo;
    FsStat fsStat(std::move(entity));

    auto xattrMock = SysXattrMock::GetMock();
    Location expectedLocation = CLOUD;
    EXPECT_CALL(*xattrMock, fgetxattr(_, _, _, _))
        .WillOnce(DoAll(Invoke([&expectedLocation](int fd, const char *name, void *value, size_t size) {
            *static_cast<char *>(value) = static_cast<char>(expectedLocation + '0');
        }),
            Return(1)));

    auto location = fsStat.GetLocation();

    testing::Mock::VerifyAndClearExpectations(xattrMock.get());
    EXPECT_EQ(location, static_cast<int32_t>(expectedLocation));

    GTEST_LOG_(INFO) << "FsStatMockTes-end FsStatMockTest_GetLocation_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test