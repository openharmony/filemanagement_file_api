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

#include "fs_file.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sys/prctl.h>

#include "file_entity.h"
#include "system_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsFileMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::unique_ptr<FileEntity> fileEntity;
    std::unique_ptr<FsFile> fsFile;
};

void FsFileMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "FsFileMockTest");
    SystemMock::EnableMock();
    UvFsMock::EnableMock();
}

void FsFileMockTest::TearDownTestCase(void)
{
    SystemMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsFileMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";

    fileEntity = std::make_unique<FileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    fileEntity->fd_ = std::make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    fileEntity->path_ = "/data/test/file_test.txt";
    fileEntity->uri_ = "";
    fsFile = std::make_unique<FsFile>(std::move(fileEntity));
}

void FsFileMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileMockTest_GetPath_001
 * @tc.desc: Test function of GetPath() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetPath_001";

    uv_fs_t mock_req;
    mock_req.ptr = const_cast<char *>("/data/test/file_test.txt");

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t *, uv_fs_t *req, const char *, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));
    auto result = fsFile->GetPath();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetPath_001";
}

/**
 * @tc.name: FsFileMockTest_GetPath_002
 * @tc.desc: Test function of GetPath() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetPath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetPath_002";

    fsFile->fileEntity->path_ = "/invalid/path";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = fsFile->GetPath();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetPath_002";
}

/**
 * @tc.name: FsFileMockTest_GetName_003
 * @tc.desc: Test function of GetName() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_003";

    fsFile->fileEntity->path_ = "/invalid/path";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = fsFile->GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_003";
}

/**
 * @tc.name: FsFileMockTest_GetName_004
 * @tc.desc: Test function of GetName() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_004";

    fsFile->fileEntity->path_ = "file_test.txt";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = fsFile->GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_004";
}

/**
 * @tc.name: FsFileMockTest_GetParent_005
 * @tc.desc: Test function of GetParent() interface for ERROR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetParent_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetParent_005";

    fsFile->fileEntity->path_ = "/invalid/path";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = fsFile->GetParent();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetParent_005";
}

/**
 * @tc.name: FsFileMockTest_GetName_006
 * @tc.desc: Test function of GetName() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_006";

    uv_fs_t mock_req;
    mock_req.ptr = const_cast<char *>("/file_test.txt");

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t *, uv_fs_t *req, const char *, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));

    auto result = fsFile->GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_006";
}

/**
 * @tc.name: FsFileMockTest_GetParent_007
 * @tc.desc: Test function of GetParent() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetParent_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetParent_007";

    uv_fs_t mock_req;
    mock_req.ptr = const_cast<char *>("/test/dir_test");

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t *, uv_fs_t *req, const char *, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));

    auto result = fsFile->GetParent();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetParent_007";
}

/**
 * @tc.name: FsFileMockTest_GetName_008
 * @tc.desc: Test function of GetName() interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_008";

    uv_fs_t mock_req;
    mock_req.ptr = const_cast<char *>("file_test.txt");

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t *, uv_fs_t *req, const char *, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));

    auto result = fsFile->GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_008";
}

/**
 * @tc.name: FsFileMockTest_GetParent_009
 * @tc.desc: Test function of GetParent() interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetParent_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetParent_009";

    uv_fs_t mock_req;
    mock_req.ptr = const_cast<char *>("dir_test");

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t *, uv_fs_t *req, const char *, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));

    auto result = fsFile->GetParent();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetParent_009";
}

/**
 * @tc.name: FsFileMockTest_Lock_010
 * @tc.desc: Test function of Lock() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_Lock_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_Lock_010";
    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(1));
    auto result = fsFile->Lock(true);
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_Lock_010";
}

/**
 * @tc.name: FsFileMockTest_Lock_011
 * @tc.desc: Test function of Lock() interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_Lock_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_Lock_011";

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(-1));
    auto result = fsFile->Lock(false);
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_Lock_011";
}

/**
 * @tc.name: FsFileMockTest_TryLock_012
 * @tc.desc: Test function of TryLock() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_TryLock_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_TryLock_012";

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(1));
    auto result = fsFile->TryLock(true);
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_TryLock_012";
}

/**
 * @tc.name: FsFileMockTest_TryLock_013
 * @tc.desc: Test function of TryLock() interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_TryLock_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_TryLock_013";

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(-1));
    auto result = fsFile->TryLock(false);
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_TryLock_013";
}

/**
 * @tc.name: FsFileMockTest_UnLock_014
 * @tc.desc: Test function of UnLock() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_UnLock_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_UnLock_014";

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(1));
    auto result = fsFile->UnLock();
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), true);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_UnLock_014";
}

/**
 * @tc.name: FsFileMockTest_UnLock_015
 * @tc.desc: Test function of UnLock() interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_UnLock_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_UnLock_015";

    auto sys = SystemMock::GetMock();
    EXPECT_CALL(*sys, flock(_, _)).WillOnce(Return(-1));
    auto result = fsFile->UnLock();
    testing::Mock::VerifyAndClearExpectations(sys.get());
    EXPECT_EQ(result.IsSuccess(), false);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_UnLock_015";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test