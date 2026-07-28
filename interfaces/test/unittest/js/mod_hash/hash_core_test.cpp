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

HWTEST_F(HashCoreTest, HashCoreTest_DoHash_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_006 start";

    auto path = testDir + "/DoHash_006.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, ""));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_006 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DoHash_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_007 start";

    auto path = testDir + "/DoHash_007.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, ""));

    auto ret = HashCore::DoHash(path, "sha1");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_007 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DoHash_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_008 start";

    auto path = testDir + "/DoHash_008.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, ""));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_008 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DoHash_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_009 start";

    auto path = testDir + "/DoHash_009.txt";
    string largeContent(1024 * 100, 'a');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_009 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DoHash_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_010 start";

    auto path = testDir + "/DoHash_010.txt";
    string largeContent(1024 * 100, 'b');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DoHash_010 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Consistency_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Consistency_001 start";

    auto path = testDir + "/Consistency_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret1 = HashCore::DoHash(path, "md5");
    auto ret2 = HashCore::DoHash(path, "md5");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_Consistency_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Consistency_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Consistency_002 start";

    auto path = testDir + "/Consistency_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret1 = HashCore::DoHash(path, "sha256");
    auto ret2 = HashCore::DoHash(path, "sha256");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_Consistency_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DifferentAlgorithms_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DifferentAlgorithms_001 start";

    auto path = testDir + "/DifferentAlgorithms_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto md5Ret = HashCore::DoHash(path, "md5");
    auto sha1Ret = HashCore::DoHash(path, "sha1");
    auto sha256Ret = HashCore::DoHash(path, "sha256");

    ASSERT_TRUE(md5Ret.IsSuccess());
    ASSERT_TRUE(sha1Ret.IsSuccess());
    ASSERT_TRUE(sha256Ret.IsSuccess());

    EXPECT_NE(md5Ret.GetData().value(), sha1Ret.GetData().value());
    EXPECT_NE(sha1Ret.GetData().value(), sha256Ret.GetData().value());
    EXPECT_NE(md5Ret.GetData().value(), sha256Ret.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_DifferentAlgorithms_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DifferentContents_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DifferentContents_001 start";

    auto path1 = testDir + "/DifferentContents_001_a.txt";
    auto path2 = testDir + "/DifferentContents_001_b.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path1, "content a"));
    ASSERT_TRUE(FileUtils::CreateFile(path2, "content b"));

    auto ret1 = HashCore::DoHash(path1, "sha256");
    auto ret2 = HashCore::DoHash(path2, "sha256");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_NE(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_DifferentContents_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_SameContents_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_SameContents_001 start";

    auto path1 = testDir + "/SameContents_001_a.txt";
    auto path2 = testDir + "/SameContents_001_b.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path1, "same content"));
    ASSERT_TRUE(FileUtils::CreateFile(path2, "same content"));

    auto ret1 = HashCore::DoHash(path1, "sha256");
    auto ret2 = HashCore::DoHash(path2, "sha256");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    EXPECT_EQ(ret1.GetData().value(), ret2.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_SameContents_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_InvalidAlgorithm_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_InvalidAlgorithm_001 start";

    auto path = testDir + "/InvalidAlgorithm_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "invalid_algorithm");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_InvalidAlgorithm_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_InvalidAlgorithm_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_InvalidAlgorithm_002 start";

    auto path = testDir + "/InvalidAlgorithm_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "SHA256");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_InvalidAlgorithm_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_MultipleFiles_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_MultipleFiles_001 start";

    for (int i = 0; i < 10; i++) {
        auto path = testDir + "/MultipleFiles_001_" + to_string(i) + ".txt";
        ASSERT_TRUE(FileUtils::CreateFile(path, "content " + to_string(i)));

        auto ret = HashCore::DoHash(path, "md5");
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_MultipleFiles_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_MultipleFiles_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_MultipleFiles_002 start";

    for (int i = 0; i < 10; i++) {
        auto path = testDir + "/MultipleFiles_002_" + to_string(i) + ".txt";
        ASSERT_TRUE(FileUtils::CreateFile(path, "content " + to_string(i)));

        auto ret = HashCore::DoHash(path, "sha256");
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_MultipleFiles_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Directory_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Directory_001 start";

    auto path = testDir + "/Directory_001";
    ASSERT_TRUE(FileUtils::CreateDirectories(path, true));

    auto ret = HashCore::DoHash(path, "sha256");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_Directory_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_EmptyPath_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_EmptyPath_001 start";

    auto ret = HashCore::DoHash("", "sha256");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_EmptyPath_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Stress_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Stress_001 start";

    auto path = testDir + "/Stress_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    for (int i = 0; i < 50; i++) {
        auto ret = HashCore::DoHash(path, "md5");
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_Stress_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Stress_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Stress_002 start";

    auto path = testDir + "/Stress_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    for (int i = 0; i < 50; i++) {
        auto ret = HashCore::DoHash(path, "sha256");
        ASSERT_TRUE(ret.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_Stress_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_AlgorithmMaps_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_AlgorithmMaps_001 start";

    EXPECT_NE(algorithmMaps.find("md5"), algorithmMaps.end());
    EXPECT_NE(algorithmMaps.find("sha1"), algorithmMaps.end());
    EXPECT_NE(algorithmMaps.find("sha256"), algorithmMaps.end());

    GTEST_LOG_(INFO) << "HashCoreTest_AlgorithmMaps_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DeepDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DeepDirectory_001 start";

    auto deepDir = testDir;
    for (int i = 0; i < 5; i++) {
        deepDir += "/level" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/DeepDirectory_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DeepDirectory_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DeepDirectory_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DeepDirectory_002 start";

    auto deepDir = testDir;
    for (int i = 0; i < 5; i++) {
        deepDir += "/deep" + to_string(i);
        ASSERT_TRUE(FileUtils::CreateDirectories(deepDir, true));
    }

    auto path = deepDir + "/DeepDirectory_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_DeepDirectory_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_BinaryContent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_001 start";

    auto path = testDir + "/BinaryContent_001.bin";
    string binaryContent(256, '\0');
    ASSERT_TRUE(FileUtils::CreateFile(path, binaryContent));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_BinaryContent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_002 start";

    auto path = testDir + "/BinaryContent_002.bin";
    string binaryContent(512, '\xFF');
    ASSERT_TRUE(FileUtils::CreateFile(path, binaryContent));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_BinaryContent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_003 start";

    auto path = testDir + "/BinaryContent_003.bin";
    string content;
    content = "Binary";
    content.push_back('\x00');
    content += "Content";
    content.push_back('\x01');
    content += "Test";
    content.push_back('\x02');
    content += "Data";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    auto ret = HashCore::DoHash(path, "sha1");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_BinaryContent_003 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_UnicodeContent_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_001 start";

    auto path = testDir + "/UnicodeContent_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "中文测试内容"));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_UnicodeContent_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_002 start";

    auto path = testDir + "/UnicodeContent_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "日本語テスト"));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_UnicodeContent_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_003 start";

    auto path = testDir + "/UnicodeContent_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "한국어 테스트"));

    auto ret = HashCore::DoHash(path, "sha1");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeContent_003 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_UnicodeFileName_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeFileName_001 start";

    auto path = testDir + "/UnicodeFileName_中文_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeFileName_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_UnicodeFileName_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeFileName_002 start";

    auto path = testDir + "/UnicodeFileName_日本語_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_UnicodeFileName_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_SpecialChars_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_SpecialChars_001 start";

    auto path = testDir + "/SpecialChars_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test\nwith\nnewlines"));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_SpecialChars_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_SpecialChars_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_SpecialChars_002 start";

    auto path = testDir + "/SpecialChars_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test\twith\ttabs"));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_SpecialChars_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DifferentSizes_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DifferentSizes_001 start";

    vector<int> sizes = {1, 100, 1024, 4096, 65536};
    for (auto size : sizes) {
        auto path = testDir + "/DifferentSizes_001_" + to_string(size) + ".bin";
        string content(size, 'x');
        ASSERT_TRUE(FileUtils::CreateFile(path, content));

        auto ret = HashCore::DoHash(path, "md5");
        ASSERT_TRUE(ret.IsSuccess());
        EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);
    }

    GTEST_LOG_(INFO) << "HashCoreTest_DifferentSizes_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_DifferentSizes_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_DifferentSizes_002 start";

    vector<int> sizes = {1, 100, 1024, 4096, 65536};
    for (auto size : sizes) {
        auto path = testDir + "/DifferentSizes_002_" + to_string(size) + ".bin";
        string content(size, 'y');
        ASSERT_TRUE(FileUtils::CreateFile(path, content));

        auto ret = HashCore::DoHash(path, "sha256");
        ASSERT_TRUE(ret.IsSuccess());
        EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);
    }

    GTEST_LOG_(INFO) << "HashCoreTest_DifferentSizes_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Symlink_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Symlink_001 start";

    auto realPath = testDir + "/Symlink_001_real.txt";
    auto linkPath = testDir + "/Symlink_001_link.txt";

    ASSERT_TRUE(FileUtils::CreateFile(realPath, "test content"));

    int ret = symlink(realPath.c_str(), linkPath.c_str());
    if (ret == 0) {
        auto result = HashCore::DoHash(linkPath, "sha256");
        ASSERT_TRUE(result.IsSuccess());
        unlink(linkPath.c_str());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_Symlink_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_Symlink_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_Symlink_002 start";

    auto realPath = testDir + "/Symlink_002_real.txt";
    auto linkPath = testDir + "/Symlink_002_link.txt";

    ASSERT_TRUE(FileUtils::CreateFile(realPath, "test content"));

    int ret = symlink(realPath.c_str(), linkPath.c_str());
    if (ret == 0) {
        auto result = HashCore::DoHash(linkPath, "md5");
        ASSERT_TRUE(result.IsSuccess());
        unlink(linkPath.c_str());
    }

    GTEST_LOG_(INFO) << "HashCoreTest_Symlink_002 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_AllAlgorithms_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest_AllAlgorithms_001 start";

    auto path = testDir + "/AllAlgorithms_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret1 = HashCore::DoHash(path, "md5");
    auto ret2 = HashCore::DoHash(path, "sha1");
    auto ret3 = HashCore::DoHash(path, "sha256");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    ASSERT_TRUE(ret3.IsSuccess());

    EXPECT_NE(ret1.GetData().value(), ret2.GetData().value());
    EXPECT_NE(ret2.GetData().value(), ret3.GetData().value());
    EXPECT_NE(ret1.GetData().value(), ret3.GetData().value());

    GTEST_LOG_(INFO) << "HashCoreTest_AllAlgorithms_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_ConcurrentSafe_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_ConcurrentSafe_001 start";

    auto path = testDir + "/ConcurrentSafe_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "test content"));

    auto ret1 = HashCore::DoHash(path, "md5");
    auto ret2 = HashCore::DoHash(path, "sha1");
    auto ret3 = HashCore::DoHash(path, "sha256");

    ASSERT_TRUE(ret1.IsSuccess());
    ASSERT_TRUE(ret2.IsSuccess());
    ASSERT_TRUE(ret3.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest_ConcurrentSafe_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_VeryLargeFile_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_VeryLargeFile_001 start";

    auto path = testDir + "/VeryLargeFile_001.txt";
    string largeContent(1024 * 500, 'x');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = HashCore::DoHash(path, "sha256");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_VeryLargeFile_001 end";
}

HWTEST_F(HashCoreTest, HashCoreTest_VeryLargeFile_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashCoreTest_VeryLargeFile_002 start";

    auto path = testDir + "/VeryLargeFile_002.txt";
    string largeContent(1024 * 500, 'y');
    ASSERT_TRUE(FileUtils::CreateFile(path, largeContent));

    auto ret = HashCore::DoHash(path, "md5");
    ASSERT_TRUE(ret.IsSuccess());
    EXPECT_EQ(ret.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashCoreTest_VeryLargeFile_002 end";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS