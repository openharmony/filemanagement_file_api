/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "file_n_exporter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "file_entity.h"
#include "libn_mock.h"
#include "sys_file_mock.h"
#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FileMockTest");
    LibnMock::EnableMock();
    SysFileMock::EnableMock();
    UvFsMock::EnableMock();
}

void FileMockTest::TearDownTestSuite()
{
    LibnMock::DisableMock();
    SysFileMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

 /**
  * @tc.name: FileMockTest_GetPath_001
  * @tc.desc: Test function of FsFile::GetPath interface for FAILURE when uv_fs_realpath fails.
  * @tc.size: MEDIUM
  * @tc.type: FUNC
  * @tc.level Level 1
  */
HWTEST_F(FileMockTest, FileMockTest_GetPath_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetPath_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    FileEntity fileEntity;
    fileEntity.fd_ = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileEntity.path_ = "fackPath/FileMockTest_GetPath_001.txt";
    fileEntity.uri_ = "";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&fileEntity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&fileEntity)),
         testing::Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = FileNExporter::GetPath(env, info);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetPath_001";
}

  /**
  * @tc.name: FileMockTest_GetPath_002
  * @tc.desc: Test function of FsFile::GetPath interface for FAILURE when InitArgs fails.
  * @tc.size: MEDIUM
  * @tc.type: FUNC
  * @tc.level Level 1
  */
HWTEST_F(FileMockTest, FileMockTest_GetPath_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetPath_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));

    auto result = FileNExporter::GetPath(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetPath_002";
}

/**
 * @tc.name: FileMockTest_GetName_001
 * @tc.desc: Test function of FsFile::GetName interface for FAILURE when uv_fs_realpath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_GetName_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetName_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    FileEntity fileEntity;
    fileEntity.fd_ = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileEntity.path_ = "fackPath/FileMockTest_GetPath_001.txt";
    fileEntity.uri_ = "";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&fileEntity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&fileEntity)),
         testing::Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = FileNExporter::GetName(env, info);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetName_001";
}

/**
 * @tc.name: FileMockTest_GetName_002
 * @tc.desc: Test function of FsFile::GetName interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_GetName_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetName_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));

    auto result = FileNExporter::GetName(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetName_002";
}

/**
 * @tc.name: FileMockTest_GetParent_001
 * @tc.desc: Test function of FsFile::GetParent interface for FAILURE when uv_fs_realpath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_GetParent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetParent_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    FileEntity fileEntity;
    fileEntity.fd_ = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileEntity.path_ = "fackPath/FileMockTest_GetParent_001.txt";
    fileEntity.uri_ = "";
    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&fileEntity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&fileEntity)),
         testing::Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_realpath(_, _, _, _)).WillOnce(Return(-1));

    auto result = FileNExporter::GetParent(env, info);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetParent_001";
}

/**
 * @tc.name: FileMockTest_GetParent_002
 * @tc.desc: Test function of FsFile::GetParent interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_GetParent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_GetParent_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));

    auto result = FileNExporter::GetParent(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_GetParent_002";
}

/**
 * @tc.name: FileMockTest_TryLock_001
 * @tc.desc: Test function of FsFile::TryLock interface for FAILURE when flock fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_TryLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_Lock_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto filepath = "fackPath/FileMockTest_Lock_001.txt";
    FileEntity fileEntity;
    fileEntity.fd_ = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileEntity.path_ = filepath;
    fileEntity.uri_ = "";
    auto libnMock = LibnMock::GetMock();
    auto fileMock = SysFileMock::GetMock();
    auto uvMock = UvFsMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), _)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&fileEntity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&fileEntity)),
         testing::Return(napi_ok)));
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto result = FileNExporter::TryLock(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_TryLock_001";
}

/**
 * @tc.name: FileMockTest_TryLock_002
 * @tc.desc: Test function of FsFile::TryLock interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_TryLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_TryLock_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), _)).WillOnce(testing::Return(false));

    auto result = FileNExporter::TryLock(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_TryLock_002";
}

/**
 * @tc.name: FileMockTest_UnLock_001
 * @tc.desc: Test function of FsFile::UnLock interface for FAILURE when flock fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_UnLock_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_UnLock_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto filepath = "fackPath/FileMockTest_UnLock_001.txt";
    FileEntity fileEntity;
    fileEntity.fd_ = std::make_unique<DistributedFS::FDGuard>(1, false);
    fileEntity.path_ = filepath;
    fileEntity.uri_ = "";
    auto libnMock = LibnMock::GetMock();
    auto fileMock = SysFileMock::GetMock();
    auto uvMock = UvFsMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(reinterpret_cast<napi_value>(&fileEntity)));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&fileEntity)),
         testing::Return(napi_ok)));
    EXPECT_CALL(*fileMock, flock(_, _)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto result = FileNExporter::UnLock(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(fileMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_UnLock_001";
}

/**
 * @tc.name: FileMockTest_UnLock_002
 * @tc.desc: Test function of FsFile::UnLock interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FileMockTest, FileMockTest_UnLock_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileMockTest-begin FileMockTest_UnLock_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();

    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));

    auto result = FileNExporter::UnLock(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FileMockTest-end FileMockTest_UnLock_002";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS