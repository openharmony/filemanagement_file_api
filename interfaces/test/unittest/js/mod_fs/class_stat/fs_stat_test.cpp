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

#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsStatTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FsStatTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FsStatTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FsStatTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsStatTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsStatTest_Constructor_001
 * @tc.desc: Test FsStat::Constructor for success case
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_Constructor_001";
    
    auto stat = FsStat::Constructor();
    EXPECT_NE(stat, nullptr);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_Constructor_001";
}

/**
 * @tc.name: FsStatTest_IsBlockDevice_001
 * @tc.desc: Test FsStat::IsBlockDevice for directory
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsBlockDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsBlockDevice_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFBLK;
    EXPECT_TRUE(stat->IsBlockDevice());
    EXPECT_FALSE(stat->IsFile());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsBlockDevice_001";
}

/**
 * @tc.name: FsStatTest_IsCharacterDevice_001
 * @tc.desc: Test FsStat::IsCharacterDevice for directory
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsCharacterDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsCharacterDevice_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFCHR;
    EXPECT_TRUE(stat->IsCharacterDevice());
    EXPECT_FALSE(stat->IsFile());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsCharacterDevice_001";
}

/**
 * @tc.name: FsStatTest_IsDirectory_001
 * @tc.desc: Test FsStat::IsDirectory for directory
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsDirectory_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFDIR | 0755;
    EXPECT_TRUE(stat->IsDirectory());
    EXPECT_FALSE(stat->IsFile());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsDirectory_001";
}

/**
 * @tc.name: FsStatTest_IsFIFO_001
 * @tc.desc: Test FsStat::IsFIFO for directory
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsFIFO_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsFIFO_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFIFO;
    EXPECT_TRUE(stat->IsFIFO());
    EXPECT_FALSE(stat->IsFile());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsFIFO_001";
}

/**
 * @tc.name: FsStatTest_IsFile_001
 * @tc.desc: Test FsStat::IsFile for regular file
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsFile_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFREG | 0644;
    EXPECT_TRUE(stat->IsFile());
    EXPECT_FALSE(stat->IsDirectory());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsFile_001";
}

/**
 * @tc.name: FsStatTest_IsSocket_001
 * @tc.desc: Test FsStat::IsSocket for symbolic link
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsSocket_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsSocket_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFSOCK;
    EXPECT_TRUE(stat->IsSocket());
    EXPECT_FALSE(stat->IsDirectory());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsSocket_001";
}

/**
 * @tc.name: FsStatTest_IsSymbolicLink_001
 * @tc.desc: Test FsStat::IsSymbolicLink for symbolic link
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsSymbolicLink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsSymbolicLink_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFLNK | 0777;
    EXPECT_TRUE(stat->IsSymbolicLink());
    EXPECT_FALSE(stat->IsDirectory());
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsSymbolicLink_001";
}

/**
 * @tc.name: FsStatTest_GetIno_001
 * @tc.desc: Test FsStat::GetIno for valid inode number
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetIno_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetIno_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_ino = 123456789;
    EXPECT_EQ(stat->GetIno(), 123456789);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetIno_001";
}

/**
 * @tc.name: FsStatTest_GetMode_001
 * @tc.desc: Test FsStat::GetMode for permission bits
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMode_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMode_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mode = S_IFREG | 0755;
    EXPECT_EQ(stat->GetMode(), 0755);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMode_001";
}

/**
 * @tc.name: FsStatTest_GetUid_001
 * @tc.desc: Test FsStat::GetUid for user ID
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetUid_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetUid_001";

    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_uid = 1000;
    EXPECT_EQ(stat->GetUid(), 1000);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetUid_001";
}

/**
 * @tc.name: FsStatTest_GetGid_001
 * @tc.desc: Test FsStat::GetGid for group ID
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetGid_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetGid_001";

    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_gid = 1000;
    EXPECT_EQ(stat->GetGid(), 1000);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetGid_001";
}

/**
 * @tc.name: FsStatTest_GetSize_001
 * @tc.desc: Test FsStat::GetSize for file size
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetSize_001";

    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_size = 123456789;
    EXPECT_EQ(stat->GetSize(), 123456789);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetSize_001";
}

/**
 * @tc.name: FsStatTest_GetAtime_001
 * @tc.desc: Test FsStat::GetAtime for access time in seconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetAtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetAtime_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_atim.tv_sec = 1630473600;
    stat->entity->stat_.st_atim.tv_nsec = 500000000;
    
    EXPECT_EQ(stat->GetAtime(), 1630473600);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetAtime_001";
}

/**
 * @tc.name: FsStatTest_GetMtime_001
 * @tc.desc: Test FsStat::GetMtime for modification time in seconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMtime_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mtim.tv_sec = 1630473601;
    stat->entity->stat_.st_mtim.tv_nsec = 500000000;
    
    EXPECT_EQ(stat->GetMtime(), 1630473601);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMtime_001";
}

/**
 * @tc.name: FsStatTest_GetCtime_001
 * @tc.desc: Test FsStat::GetCtime for change time in seconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetCtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetCtime_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_ctim.tv_sec = 1630473602;
    stat->entity->stat_.st_ctim.tv_nsec = 500000000;

    EXPECT_EQ(stat->GetCtime(), 1630473602);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetCtime_001";
}

/**
 * @tc.name: FsStatTest_GetAtimeNs_001
 * @tc.desc: Test FsStat::GetAtimeNs for access time in nanoseconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetAtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetAtimeNs_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_atim.tv_sec = 1630473600;
    stat->entity->stat_.st_atim.tv_nsec = 500000000;

    int64_t expected = 1630473600LL * 1000000000 + 500000000;
    EXPECT_EQ(stat->GetAtimeNs(), expected);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetAtimeNs_001";
}

/**
 * @tc.name: FsStatTest_GetMtimeNs_001
 * @tc.desc: Test FsStat::GetMtimeNs for modification time in nanoseconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMtimeNs_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_mtim.tv_sec = 1630473601;
    stat->entity->stat_.st_mtim.tv_nsec = 500000000;

    int64_t expected = 1630473601LL * 1000000000 + 500000000;
    EXPECT_EQ(stat->GetMtimeNs(), expected);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMtimeNs_001";
}

/**
 * @tc.name: FsStatTest_GetCtimeNs_001
 * @tc.desc: Test FsStat::GetCtimeNs for change time in nanoseconds
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetCtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetCtimeNs_001";
    
    auto stat = FsStat::Constructor();
    ASSERT_NE(stat, nullptr);

    stat->entity->stat_.st_ctim.tv_sec = 1630473602;
    stat->entity->stat_.st_ctim.tv_nsec = 500000000;

    int64_t expected = 1630473602LL * 1000000000 + 500000000;
    EXPECT_EQ(stat->GetCtimeNs(), expected);
    delete stat;
    
    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetCtimeNs_001";
}

}