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

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsStatTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void FsStatTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
}

void FsStatTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsStatTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsStatTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsStatTest_Constructor_001
 * @tc.desc: Test function of FsStat::Constructor interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_Constructor_001";

    std::unique_ptr<FsStat> fsStat(FsStat::Constructor()); // To smart ptr for auto memory release
    ASSERT_NE(fsStat, nullptr);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_Constructor_001";
}

/**
 * @tc.name: FsStatTest_IsBlockDevice_001
 * @tc.desc: Test function of FsStat::IsBlockDevice interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsBlockDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsBlockDevice_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFBLK;

    bool result = fsStat.IsBlockDevice();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsBlockDevice_001";
}

/**
 * @tc.name: FsStatTest_IsCharacterDevice_001
 * @tc.desc: Test function of FsStat::IsCharacterDevice interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsCharacterDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsCharacterDevice_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFCHR;

    bool result = fsStat.IsCharacterDevice();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsCharacterDevice_001";
}

/**
 * @tc.name: FsStatTest_IsDirectory_001
 * @tc.desc: Test function of FsStat::IsDirectory interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsDirectory_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFDIR | 0755;

    bool result = fsStat.IsDirectory();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsDirectory_001";
}

/**
 * @tc.name: FsStatTest_IsFIFO_001
 * @tc.desc: Test function of FsStat::IsFIFO interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsFIFO_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsFIFO_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFIFO;

    bool result = fsStat.IsFIFO();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsFIFO_001";
}

/**
 * @tc.name: FsStatTest_IsFile_001
 * @tc.desc: Test function of FsStat::IsFile interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsFile_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFREG | 0644;

    bool result = fsStat.IsFile();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsFile_001";
}

/**
 * @tc.name: FsStatTest_IsSocket_001
 * @tc.desc: Test function of FsStat::IsSocket interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsSocket_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsSocket_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFSOCK;

    bool result = fsStat.IsSocket();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsSocket_001";
}

/**
 * @tc.name: FsStatTest_IsSymbolicLink_001
 * @tc.desc: Test function of FsStat::IsSymbolicLink interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_IsSymbolicLink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_IsSymbolicLink_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFLNK | 0777;

    bool result = fsStat.IsSymbolicLink();

    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_IsSymbolicLink_001";
}

/**
 * @tc.name: FsStatTest_GetIno_001
 * @tc.desc: Test function of FsStat::GetIno interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetIno_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetIno_001";

    auto ino = 10;
    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_ino = ino;

    auto result = fsStat.GetIno();

    EXPECT_EQ(result, ino);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetIno_001";
}

/**
 * @tc.name: FsStatTest_GetMode_001
 * @tc.desc: Test function of FsStat::GetMode interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMode_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMode_001";

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mode = S_IFREG | 0755;

    auto result = fsStat.GetMode();

    EXPECT_EQ(result, 0755);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMode_001";
}

/**
 * @tc.name: FsStatTest_GetUid_001
 * @tc.desc: Test function of FsStat::GetUid interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetUid_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetUid_001";

    auto uid = 1000;
    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_uid = uid;

    auto result = fsStat.GetUid();

    EXPECT_EQ(result, uid);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetUid_001";
}

/**
 * @tc.name: FsStatTest_GetGid_001
 * @tc.desc: Test function of FsStat::GetGid interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetGid_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetGid_001";

    auto gid = 1000;
    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_gid = gid;

    auto result = fsStat.GetGid();

    EXPECT_EQ(result, gid);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetGid_001";
}

/**
 * @tc.name: FsStatTest_GetSize_001
 * @tc.desc: Test function of FsStat::GetSize interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetSize_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetSize_001";

    auto size = 10;
    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_size = size;

    auto result = fsStat.GetSize();

    EXPECT_EQ(result, size);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetSize_001";
}

/**
 * @tc.name: FsStatTest_GetAtime_001
 * @tc.desc: Test function of FsStat::GetAtime interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetAtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetAtime_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_atim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_atim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetAtime();

    EXPECT_EQ(result, now.tv_sec);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetAtime_001";
}

/**
 * @tc.name: FsStatTest_GetMtime_001
 * @tc.desc: Test function of FsStat::GetMtime interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMtime_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mtim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_mtim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetMtime();

    EXPECT_EQ(result, now.tv_sec);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMtime_001";
}

/**
 * @tc.name: FsStatTest_GetCtime_001
 * @tc.desc: Test function of FsStat::GetCtime interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetCtime_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetCtime_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_ctim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_ctim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetCtime();

    EXPECT_EQ(result, now.tv_sec);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetCtime_001";
}

/**
 * @tc.name: FsStatTest_GetAtimeNs_001
 * @tc.desc: Test function of FsStat::GetAtimeNs interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetAtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetAtimeNs_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_atim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_atim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetAtimeNs();

    auto nowNs = static_cast<uint64_t>(now.tv_sec * SECOND_TO_NANOSECOND + now.tv_nsec);
    EXPECT_EQ(result, nowNs);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetAtimeNs_001";
}

/**
 * @tc.name: FsStatTest_GetMtimeNs_001
 * @tc.desc: Test function of FsStat::GetMtimeNs interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetMtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetMtimeNs_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_mtim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_mtim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetMtimeNs();

    auto nowNs = static_cast<uint64_t>(now.tv_sec * SECOND_TO_NANOSECOND + now.tv_nsec);
    EXPECT_EQ(result, nowNs);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetMtimeNs_001";
}

/**
 * @tc.name: FsStatTest_GetCtimeNs_001
 * @tc.desc: Test function of FsStat::GetCtimeNs interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStatTest, FsStatTest_GetCtimeNs_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStatTest-begin FsStatTest_GetCtimeNs_001";

    uv_timespec64_t now;
    int ret = uv_clock_gettime(UV_CLOCK_REALTIME, &now);
    ASSERT_EQ(ret, 0);

    auto entity = std::make_unique<StatEntity>();
    FsStat fsStat(std::move(entity));
    fsStat.entity->stat_.st_ctim.tv_sec = now.tv_sec;
    fsStat.entity->stat_.st_ctim.tv_nsec = now.tv_nsec;

    auto result = fsStat.GetCtimeNs();

    auto nowNs = static_cast<uint64_t>(now.tv_sec * SECOND_TO_NANOSECOND + now.tv_nsec);
    EXPECT_EQ(result, nowNs);

    GTEST_LOG_(INFO) << "FsStatTest-end FsStatTest_GetCtimeNs_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test