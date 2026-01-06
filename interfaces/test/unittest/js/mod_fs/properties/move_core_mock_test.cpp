/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "move_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveCoreMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/MoveCoreMockTest";
};

void MoveCoreMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MoveCoreMockTest");
    UvFsMock::EnableMock();
}

void MoveCoreMockTest::TearDownTestSuite()
{
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MoveCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void MoveCoreMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_000
 * @tc.desc: Test function of MoveCore::DoMove interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_000";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_000_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_000_dest.txt";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_000";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_001
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when uv_fs_rename fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_001";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_001_non_existent.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_001_dest.txt";
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-ENOENT));

    auto res = MoveCore::DoMove(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_001";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_002
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is MODE_FORCE_MOVE and uv_fs_rename
 * fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_002";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_002_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_002_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EIO));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_002";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_003
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when uv_fs_access fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_003";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_003_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_003_dest.txt";

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(-ENOENT));

    auto res = MoveCore::DoMove(src, dest);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_003";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_004
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is MODE_THROW_ERR and dest path already
 * exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_004";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_004_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_004_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).Times(2).WillOnce(Return(0)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900015);
    EXPECT_EQ(err.GetErrMsg(), "File exists");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_004";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_005
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is MODE_THROW_ERR and uv_fs_access
 * destPath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_005";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_005_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_005_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(-EIO));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_005";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_006
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is MODE_THROW_ERR and uv_fs_rename fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_006";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_006_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_006_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(-ENOENT));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900001);
    EXPECT_EQ(err.GetErrMsg(), "Operation not permitted");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_006";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_007
 * @tc.desc: Test function of MoveCore::DoMove interface for SUCCESS when mode is MODE_FORCE_MOVE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_007";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_007_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_007_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_007";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_008
 * @tc.desc: Test function of MoveCore::DoMove interface for SUCCESS when mode is MODE_THROW_ERR.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_008";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_008_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_008_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(-ENOENT));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_008";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_009
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when mode is MODE_FORCE_MOVE and uv_fs_stat src
 * fails under the cross-device condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_009";

    std::string src = "fakePath/MoveCoreMockTest_DoMove_009_src.txt";
    std::string dest = "fakePath/MoveCoreMockTest_DoMove_009_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-ENOENT));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_009";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_010
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when dest exists but src does not exist, and
 * filesystem::copy_file fails under the cross-device condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_010";

    std::string src = testDir + "/MoveCoreMockTest_DoMove_010_non_existent.txt";
    std::string dest = testDir + "/MoveCoreMockTest_DoMove_010_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    ASSERT_TRUE(FileUtils::CreateFile(dest, "content"));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_010";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_011
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when src and dest both do not exist, and
 * filesystem::copy_file fails under the cross-device condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_011";

    std::string src = testDir + "/MoveCoreMockTest_DoMove_011_src.txt";
    std::string dest = testDir + "/MoveCoreMockTest_DoMove_011_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_011";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_012
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when uv_fs_unlink src and dest fails under the
 * cross-device condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_012";

    std::string src = testDir + "/MoveCoreMockTest_DoMove_012_src.txt";
    std::string dest = testDir + "/MoveCoreMockTest_DoMove_012_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    ASSERT_TRUE(FileUtils::CreateFile(src, "content"));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1)).WillOnce(Return(-EIO));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_012";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_013
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILURE when uv_fs_unlink src fails under the cross-device
 * condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_013";

    std::string src = testDir + "/MoveCoreMockTest_DoMove_013_src.txt";
    std::string dest = testDir + "/MoveCoreMockTest_DoMove_013_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    ASSERT_TRUE(FileUtils::CreateFile(src, "content"));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-EIO)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_FALSE(res.IsSuccess());
    auto err = res.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_013";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_014
 * @tc.desc: Test function of MoveCore::DoMove interface for SUCCESS under the cross-device condition.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_014";

    std::string src = testDir + "/MoveCoreMockTest_DoMove_014_src.txt";
    std::string dest = testDir + "/MoveCoreMockTest_DoMove_014_dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    ASSERT_TRUE(FileUtils::CreateFile(src, "content"));

    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-EXDEV)); // Trigger cross-device condition
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_TRUE(res.IsSuccess());

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0014";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test