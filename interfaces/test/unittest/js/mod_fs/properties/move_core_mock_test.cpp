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

#include "move_core.h"
#include "uv_fs_mock.h"

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MoveCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<UvfsMock> uvMock = nullptr;
};

void MoveCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void MoveCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void MoveCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MoveCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

extern "C" {
    const char* uv_err_name(int err)
    {
        return "EXDEV";
    }
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

    std::string src;
    std::string dest;

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_000";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_001
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_001";

    std::string src;
    std::string dest;

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_001";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_002
 * @tc.desc: Test function of MoveCore::DoMove interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_002";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);
    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_002";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_003
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_003";

    std::string src;
    std::string dest;

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_003";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_004
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_004";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1)).WillOnce(Return(0));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_004";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_005
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_005";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_THROW_ERR);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_005";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_006
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_006";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_006";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_007
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_007";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_007";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_008
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_008";

    std::string src;
    std::string dest;

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest);
    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_008";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_009
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_009";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_009";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_0010
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_0010";

    std::string src;
    std::string dest = "file.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    int fd = open(dest.c_str(), O_RDWR | O_CREAT, 0666);
    ASSERT_NE(fd, -1);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);
    close(fd);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0010";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_0011
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_0011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_0011";

    std::string src;
    std::string dest;
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0011";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_0012
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_0012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_0012";

    std::string src = "file.txt";
    std::string dest = "dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    int fd = open(src.c_str(), O_RDWR | O_CREAT, 0666);
    ASSERT_NE(fd, -1);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1)).WillOnce(Return(-1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    ASSERT_EQ(unlink(src.c_str()), 0);
    close(fd);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0012";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_0013
 * @tc.desc: Test function of MoveCore::DoMove interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_0013";

    std::string src = "file.txt";
    std::string dest = "dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    int fd = open(src.c_str(), O_RDWR | O_CREAT, 0666);
    ASSERT_NE(fd, -1);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), false);

    ASSERT_EQ(unlink(src.c_str()), 0);
    close(fd);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0013";
}

/**
 * @tc.name: MoveCoreMockTest_DoMove_0014
 * @tc.desc: Test function of MoveCore::DoMove interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MoveCoreMockTest, MoveCoreMockTest_DoMove_0014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MoveCoreMockTest-begin MoveCoreMockTest_DoMove_0013";

    std::string src = "file.txt";
    std::string dest = "dest.txt";
    optional<int> mode = std::make_optional<int>(MODE_FORCE_MOVE);

    int fd = open(src.c_str(), O_RDWR | O_CREAT, 0666);
    ASSERT_NE(fd, -1);

    EXPECT_CALL(*uvMock, uv_fs_access(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_rename(_, _, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(1));

    auto res = MoveCore::DoMove(src, dest, mode);
    EXPECT_EQ(res.IsSuccess(), true);

    ASSERT_EQ(unlink(src.c_str()), 0);
    close(fd);

    GTEST_LOG_(INFO) << "MoveCoreMockTest-end MoveCoreMockTest_DoMove_0013";
}


} // namespace OHOS::FileManagement::ModuleFileIO::Test