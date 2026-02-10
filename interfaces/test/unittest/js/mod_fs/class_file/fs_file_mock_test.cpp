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

#include "fs_file.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "file_entity.h"
#include "sys_file_mock.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsFileMockTest");
    SysFileMock::EnableMock();
    UvFsMock::EnableMock();
}

void FsFileMockTest::TearDownTestSuite()
{
    SysFileMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsFileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsFileMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsFileMockTest_GetPath_001
 * @tc.desc: Test function of FsFile::GetPath interface for FAILURE when uv_fs_realpath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetPath_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_GetPath_001.txt";
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto res = fsFile.GetPath();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetPath_001";
}

/**
 * @tc.name: FsFileMockTest_GetPath_002
 * @tc.desc: Test function of FsFile::GetPath interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetPath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetPath_002";

    string filepath = "fakePath/FsFileMockTest_GetPath_002.txt";
    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = filepath;
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));
    uv_fs_t mock_req = {0};
    mock_req.ptr = static_cast<void*>(filepath.data());

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(mock_req), testing::Return(0)));

    auto res = fsFile.GetPath();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, filepath);

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetPath_002";
}

/**
 * @tc.name: FsFileMockTest_GetName_001
 * @tc.desc: Test function of FsFile::GetName interface for FAILURE when uv_fs_realpath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_GetName_001.txt";
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto res = fsFile.GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_001";
}

/**
 * @tc.name: FsFileMockTest_GetName_002
 * @tc.desc: Test function of FsFile::GetName interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetName_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetName_002";

    string filepath = "fakePath/FsFileMockTest_GetName_002.txt";
    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = filepath;
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));
    uv_fs_t mock_req = {0};
    mock_req.ptr = static_cast<void*>(filepath.data());

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(mock_req), testing::Return(0)));

    auto res = fsFile.GetName();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, "FsFileMockTest_GetName_002.txt");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetName_002";
}

/**
 * @tc.name: FsFileMockTest_GetParent_001
 * @tc.desc: Test function of FsFile::GetParent interface for FAILURE when uv_fs_realpath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetParent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetParent_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_GetParent_001.txt";
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto res = fsFile.GetParent();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetParent_001";
}

/**
 * @tc.name: FsFileMockTest_GetParent_002
 * @tc.desc: Test function of FsFile::GetParent interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_GetParent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_GetParent_002";

    string filepath = "fakePath/FsFileMockTest_GetParent_002.txt";
    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = filepath;
    entity->uri_ = "";
    FsFile fsFile(std::move(entity));
    uv_fs_t mock_req = {0};
    mock_req.ptr = static_cast<void*>(filepath.data());

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(mock_req), testing::Return(0)));

    auto res = fsFile.GetParent();

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    ASSERT_TRUE(res.IsSuccess());
    auto path = res.GetData().value();
    EXPECT_EQ(path, "fakePath");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_GetParent_002";
}

/**
 * @tc.name: FsFileMockTest_Lock_001
 * @tc.desc: Test function of FsFile::GetParent interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_Lock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_Lock_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_Lock_001.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(Return(0));

    auto res = fsFile.Lock(true);

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_Lock_001";
}

/**
 * @tc.name: FsFileMockTest_Lock_002
 * @tc.desc: Test function of FsFile::Lock interface for FAILURE when flock fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_Lock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_Lock_002";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_Lock_002.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto res = fsFile.Lock(false);

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_Lock_002";
}

/**
 * @tc.name: FsFileMockTest_TryLock_001
 * @tc.desc: Test function of FsFile::TryLock interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_TryLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_TryLock_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_TryLock_001.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(Return(0));

    auto res = fsFile.TryLock(true);

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_TryLock_001";
}

/**
 * @tc.name: FsFileMockTest_TryLock_002
 * @tc.desc: Test function of FsFile::TryLock interface for FAILURE when flock fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_TryLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_TryLock_002";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_TryLock_002.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto res = fsFile.TryLock(false);

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_TryLock_002";
}

/**
 * @tc.name: FsFileMockTest_UnLock_001
 * @tc.desc: Test function of FsFile::UnLock interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_UnLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_UnLock_001";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_UnLock_001.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(Return(0));

    auto res = fsFile.UnLock();

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_UnLock_001";
}

/**
 * @tc.name: FsFileMockTest_UnLock_002
 * @tc.desc: Test function of FsFile::UnLock interface for FAILURE when flock fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsFileMockTest, FsFileMockTest_UnLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsFileMockTest-begin FsFileMockTest_UnLock_002";

    auto expectedFd = 10;
    auto entity = std::make_unique<FileEntity>();
    entity->fd_ = std::make_unique<DistributedFS::FDGuard>(expectedFd, false);
    entity->path_ = "fakePath/FsFileMockTest_UnLock_002.txt";
    FsFile fsFile(std::move(entity));

    auto fileMock = SysFileMock::GetMock();
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto res = fsFile.UnLock();

    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsFileMockTest-end FsFileMockTest_UnLock_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test