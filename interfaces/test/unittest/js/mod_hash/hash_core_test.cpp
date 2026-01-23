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

#include "hash_core.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
namespace {
const int MD5_HASH_LENGTH = 32;
const int SHA1_HASH_LENGTH = 40;
const int SHA256_HASH_LENGTH = 64;
} // namespace

class HashCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/HashCoreTest";
};

void HashCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "HashCoreTest");
}

void HashCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void HashCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void HashCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HashCoreTest_DoHash_001
 * @tc.desc: Test function of HashCore::DoHash interface for FAILURE when algorithm is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, HashCoreTest_DoHash_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin HashCoreTest_DoHash_001";

    auto path = testDir + "/HashCoreTest_DoHash_001.txt";
    string alg = "sha128";

    auto ret = HashCore::DoHash(path, alg);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");

    GTEST_LOG_(INFO) << "HashCoreTest-end HashCoreTest_DoHash_001";
}

/**
 * @tc.name: HashCoreTest_DoHash_002
 * @tc.desc: Test function of HashCore::DoHash interface for SUCCESS with MD5 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, HashCoreTest_DoHash_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin HashCoreTest_DoHash_002";

    auto path = testDir + "/HashCoreTest_DoHash_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "HashCoreTest_DoHash_002"));

    auto ret = HashCore::DoHash(path, "md5");

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest-end HashCoreTest_DoHash_002";
}

/**
 * @tc.name: HashCoreTest_DoHash_003
 * @tc.desc: Test function of HashCore::DoHash interface for SUCCESS with SHA1 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, HashCoreTest_DoHash_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin HashCoreTest_DoHash_003";

    auto path = testDir + "/HashCoreTest_DoHash_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "HashCoreTest_DoHash_003"));

    auto ret = HashCore::DoHash(path, "sha1");

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest-end HashCoreTest_DoHash_003";
}

/**
 * @tc.name: HashCoreTest_DoHash_004
 * @tc.desc: Test function of HashCore::DoHash interface for SUCCESS with SHA256 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, HashCoreTest_DoHash_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin HashCoreTest_DoHash_004";

    auto path = testDir + "/HashCoreTest_DoHash_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "HashCoreTest_DoHash_004"));

    auto ret = HashCore::DoHash(path, "sha256");

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest-end HashCoreTest_DoHash_004";
}

/**
 * @tc.name: HashCoreTest_DoHash_005
 * @tc.desc: Test function of HashCore::DoHash interface for FAILURE when file not exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, HashCoreTest_DoHash_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin HashCoreTest_DoHash_005";

    auto path = testDir + "/HashCoreTest_DoHash_005_non_existent.txt";

    auto ret = HashCore::DoHash(path, "sha256");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "HashCoreTest-end HashCoreTest_DoHash_005";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS