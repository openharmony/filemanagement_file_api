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

#include "rename_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RenameCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RenameCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void RenameCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RenameCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void RenameCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RenameCoreTest_DoRename_001
 * @tc.desc: Test function of RenameCore::DoRename interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_001";
    string src = "/data/storage/el2/RenameCoreTestFileSrc.txt";
    string dest = "/data/storage/el2/RenameCoreTestFileDest.txt";
    auto file = open(src.c_str(), O_CREAT | O_RDWR);
    if (file < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_001 create failed! ret: " << file << ", errno: " << errno;
        EXPECT_GT(file, 0);
    }
    close(file);

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_EQ(res.IsSuccess(), true);

    auto result = remove(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_001 remove file failed! ret: " << result << ", errno: " << errno;
    }

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_001";
}

/**
 * @tc.name: RenameCoreTest_DoRename_002
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_002";
    string src = "/data/storage/el2/RenameCoreTestFileSrc.txt";
    string dest = "/data/storage/el2/RenameCoreTestFileDest.txt";
    auto file = open(src.c_str(), O_CREAT | O_RDWR);
    if (file < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_002 create failed! ret: " << file << ", errno: " << errno;
        EXPECT_GT(file, 0);
    }
    close(file);

    auto fileDest = open(src.c_str(), O_CREAT | O_RDWR);
    if (fileDest < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_002 create failed! ret: " << fileDest << ", errno: " << errno;
        EXPECT_GT(fileDest, 0);
    }
    close(fileDest);

    auto res = RenameCore::DoRename(src, dest);

    auto result = remove(src.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_002 remove file failed! ret: " << result << ", errno: " << errno;
    }
    result = remove(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_002 remove file failed! ret: " << result << ", errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_002";
}

/**
 * @tc.name: RenameCoreTest_DoRename_003
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_003";
    string src = "/data/storage/el2/RenameCoreTestFileSrc.txt";
    string dest = "/data/storage/el2/RenameCoreTestFileDest";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    auto file = open(src.c_str(), O_CREAT | O_RDWR);
    if (file < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_003 create failed! ret: " << file << ", errno: " << errno;
        EXPECT_GT(file, 0);
    }
    close(file);

    auto result = mkdir(dest.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_003 mkdir failed! ret: " << result << ", errno: " << errno;
    }

    auto res = RenameCore::DoRename(src, dest);

    result = remove(src.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_003 remove file failed! ret: " << result << ", errno: " << errno;
    }
    result = rmdir(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_003 rmdir failed! ret: " << result << ", errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_003";
}

/**
 * @tc.name: RenameCoreTest_DoRename_004
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_004";
    string src = "/data/storage/el2/RenameCoreTestFileSrc";
    string dest = "/data/storage/el2/RenameCoreTestFileDest";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    auto result = mkdir(src.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_004 mkdir failed! ret: " << result << ", errno: " << errno;
    }

    auto res = RenameCore::DoRename(src, dest);

    result = rmdir(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_004 rmdir failed! ret: " << result << ", errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_004";
}

/**
 * @tc.name: RenameCoreTest_DoRename_005
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_005";
    string src = "/data/storage/el2/RenameCoreTestFileSrc";
    string dest = "/data/storage/el2/RenameCoreTestFileDest";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    auto result = mkdir(src.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_005 mkdir failed! ret: " << result << ", errno: " << errno;
    }
    result = mkdir(dest.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_005 mkdir failed! ret: " << result << ", errno: " << errno;
    }

    auto res = RenameCore::DoRename(src, dest);

    result = rmdir(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_004 rmdir failed! ret: " << result << ", errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), true);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_004";
}

/**
 * @tc.name: RenameCoreTest_DoRename_006
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_006";
    string src = "/data/storage/el2/RenameCoreTestFileSrc";
    string dest = "/data/storage/el2/RenameCoreTestFileDest.txt";
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    auto result = mkdir(src.c_str(), mode);
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_006 mkdir failed! ret: " << result << ", errno: " << errno;
    }
    auto file = open(dest.c_str(), O_CREAT | O_RDWR);
    if (file < 0) {
        GTEST_LOG_(ERROR) << "UnlinkCoreTest_DoUnlink_006 create failed! ret: " << file << ", errno: " << errno;
        EXPECT_GT(file, 0);
    }
    close(file);

    auto res = RenameCore::DoRename(src, dest);

    result = rmdir(src.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_006 rmdir failed! ret: " << result << ", errno: " << errno;
    }
    result = remove(dest.c_str());
    if (result < 0) {
        GTEST_LOG_(ERROR) << "RenameCoreTest_DoRename_006 remove file failed! ret: " << result << ", errno: " << errno;
    }

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_006";
}

/**
 * @tc.name: RenameCoreTest_DoRename_007
 * @tc.desc: Test function of RenameCore::DoRename interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameCoreTest, RenameCoreTest_DoRename_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameCoreTest-begin RenameCoreTest_DoRename_007";
    string src = "/data/storage/el2/RenameCoreTestFileSrc.txt";
    string dest = "/data/storage/el2/RenameCoreTestFileDest.txt";

    auto res = RenameCore::DoRename(src, dest);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "RenameCoreTest-end RenameCoreTest_DoRename_007";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test