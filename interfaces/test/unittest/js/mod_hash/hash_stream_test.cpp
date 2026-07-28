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

#include "hs_hashstream.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

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

class HashStreamTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void HashStreamTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "HashStreamTest");
}

void HashStreamTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void HashStreamTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void HashStreamTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HashStreamTest_GetHsEntity_001
 * @tc.desc: Test function of HsHashStream::GetHsEntity interface for FAILURE when entity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_GetHsEntity_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_GetHsEntity_001";

    HsHashStream stream(nullptr);

    auto [succ, entity] = stream.GetHsEntity();

    EXPECT_FALSE(succ);
    EXPECT_EQ(entity, nullptr);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_GetHsEntity_001";
}

/**
 * @tc.name: HashStreamTest_GetHsEntity_002
 * @tc.desc: Test function of HsHashStream::GetHsEntity interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_GetHsEntity_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin GetHsEntityTest_0002";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();

    HsHashStream stream(move(streamEntity));

    auto [succ, entity] = stream.GetHsEntity();

    EXPECT_TRUE(succ);
    EXPECT_NE(entity, nullptr);
    entity = nullptr;

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_GetHsEntity_002";
}

/**
 * @tc.name: HashStreamTest_Update_001
 * @tc.desc: Test function of HsHashStream::Update interface for FAILURE when entity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Update_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Update_001";

    ArrayBuffer buffer(nullptr, 1);
    HsHashStream stream(nullptr);

    auto ret = stream.Update(buffer);
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Update_001";
}

/**
 * @tc.name: HashStreamTest_Update_002
 * @tc.desc: Test function of HsHashStream::Update interface for SUCCESS with MD5 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Update_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Update_002";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_MD5;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    streamEntity->md5Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    string content = "HashStreamTest_Update_002";
    ArrayBuffer data(static_cast<void *>(content.data()), content.length() + 1);

    auto ret = stream.Update(data);

    ASSERT_TRUE(ret.IsSuccess());
    auto digestRet = stream.Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    auto value = digestRet.GetData().value();
    EXPECT_EQ(value.length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Update_002";
}

/**
 * @tc.name: HashStreamTest_Update_003
 * @tc.desc: Test function of HsHashStream::Update interface for SUCCESS with SHA1 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Update_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Update_003";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA1;
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    streamEntity->shaCtx = ctx;
    HsHashStream stream(move(streamEntity));

    string content = "HashStreamTest_Update_003";
    ArrayBuffer data(static_cast<void *>(content.data()), content.length() + 1);

    auto ret = stream.Update(data);

    ASSERT_TRUE(ret.IsSuccess());
    auto digestRet = stream.Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    auto value = digestRet.GetData().value();
    EXPECT_EQ(value.length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Update_003";
}

/**
 * @tc.name: HashStreamTest_Update_004
 * @tc.desc: Test function of HsHashStream::Update interface for SUCCESS with SHA256 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Update_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Update_004";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA256;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    streamEntity->sha256Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    string content = "HashStreamTest_Update_004";
    ArrayBuffer data(static_cast<void *>(content.data()), content.length() + 1);

    auto ret = stream.Update(data);

    ASSERT_TRUE(ret.IsSuccess());
    auto digestRet = stream.Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    auto value = digestRet.GetData().value();
    EXPECT_EQ(value.length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Update_004";
}

/**
 * @tc.name: HashStreamTest_Digest_001
 * @tc.desc: Test function of HsHashStream::Digest interface for FAILURE when entity is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Digest_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Digest_001";

    HsHashStream stream(nullptr);

    auto ret = stream.Digest();

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Digest_001";
}

/**
 * @tc.name: HashStreamTest_Digest_002
 * @tc.desc: Test function of HsHashStream::Digest interface SUCCESS with MD5 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Digest_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Digest_002";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_MD5;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    streamEntity->md5Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Digest_002";
}

/**
 * @tc.name: HashStreamTest_Digest_003
 * @tc.desc: Test function of HsHashStream::Digest interface for SUCCESS with SHA1 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Digest_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Digest_003";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA1;
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    streamEntity->shaCtx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Digest_003";
}

/**
 * @tc.name: HashStreamTest_Digest_004
 * @tc.desc: Test function of HsHashStream::Digest interface for SUCCESS with SHA256 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Digest_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Digest_004";

    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA256;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    streamEntity->sha256Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();

    ASSERT_TRUE(ret.IsSuccess());
    auto value = ret.GetData().value();
    EXPECT_EQ(value.length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Digest_004";
}

/**
 * @tc.name: HashStreamTest_Constructor_001
 * @tc.desc: Test function of HsHashStream::Constructor interface for SUCCESS with SHA256 algorithm.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, HashStreamTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin HashStreamTest_Constructor_001";

    auto ret = HsHashStream::Constructor("sha256");

    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<HsHashStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);
    ASSERT_NE(stream->entity, nullptr);
    EXPECT_EQ(stream->entity->algType, HASH_ALGORITHM_TYPE_SHA256);

    GTEST_LOG_(INFO) << "HashStreamTest-end HashStreamTest_Constructor_001";
}

HWTEST_F(HashStreamTest, HashStreamTest_Constructor_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_002 start";

    auto ret = HsHashStream::Constructor("md5");
    ASSERT_TRUE(ret.IsSuccess());
    unique_ptr<HsHashStream> stream(ret.GetData().value());
    ASSERT_NE(stream, nullptr);
    ASSERT_NE(stream->entity, nullptr);
    EXPECT_EQ(stream->entity->algType, HASH_ALGORITHM_TYPE_MD5);

    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Constructor_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_003 start";

    auto ret = HsHashStream::Constructor("sha1");
    ASSERT_TRUE(ret.IsSuccess());
    unique_ptr<HsHashStream> stream(ret.GetData().value());
    ASSERT_NE(stream, nullptr);
    ASSERT_NE(stream->entity, nullptr);
    EXPECT_EQ(stream->entity->algType, HASH_ALGORITHM_TYPE_SHA1);

    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_003 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Constructor_004, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_004 start";

    auto ret = HsHashStream::Constructor("invalid");
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest_Constructor_004 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_UpdateAndDigest_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_UpdateAndDigest_001 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "test data";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_UpdateAndDigest_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_UpdateAndDigest_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_UpdateAndDigest_002 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "test data";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_UpdateAndDigest_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_MultipleUpdates_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_MultipleUpdates_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    for (int i = 0; i < 10; i++) {
        string data = "chunk" + to_string(i);
        ArrayBuffer buf(static_cast<void *>(const_cast<char*>(data.data())), data.length());
        auto updateRet = stream->Update(buf);
        ASSERT_TRUE(updateRet.IsSuccess());
    }

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_MultipleUpdates_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_MultipleUpdates_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_MultipleUpdates_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    for (int i = 0; i < 10; i++) {
        string data = "data" + to_string(i);
        ArrayBuffer buf(static_cast<void *>(const_cast<char*>(data.data())), data.length());
        auto updateRet = stream->Update(buf);
        ASSERT_TRUE(updateRet.IsSuccess());
    }

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_MultipleUpdates_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_EmptyUpdate_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_EmptyUpdate_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string emptyData;
    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(emptyData.data())), 0);
    auto updateRet = stream->Update(buf);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest_EmptyUpdate_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_LargeData_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_LargeData_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string largeData(1024 * 100, 'a');
    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(largeData.data())), largeData.length());
    auto updateRet = stream->Update(buf);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_LargeData_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_LargeData_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_LargeData_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string largeData(1024 * 100, 'b');
    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(largeData.data())), largeData.length());
    auto updateRet = stream->Update(buf);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_LargeData_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Consistency_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Consistency_001 start";

    string testData = "consistency test data";

    auto result1 = HsHashStream::Constructor("sha256");
    auto result2 = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_TRUE(result2.IsSuccess());

    unique_ptr<HsHashStream> stream1(result1.GetData().value());
    unique_ptr<HsHashStream> stream2(result2.GetData().value());

    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    stream1->Update(buf);
    stream2->Update(buf);

    auto digest1 = stream1->Digest();
    auto digest2 = stream2->Digest();

    ASSERT_TRUE(digest1.IsSuccess());
    ASSERT_TRUE(digest2.IsSuccess());
    EXPECT_EQ(digest1.GetData().value(), digest2.GetData().value());

    GTEST_LOG_(INFO) << "HashStreamTest_Consistency_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Consistency_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Consistency_002 start";

    string testData = "another consistency test";

    auto result1 = HsHashStream::Constructor("md5");
    auto result2 = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_TRUE(result2.IsSuccess());

    unique_ptr<HsHashStream> stream1(result1.GetData().value());
    unique_ptr<HsHashStream> stream2(result2.GetData().value());

    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    stream1->Update(buf);
    stream2->Update(buf);

    auto digest1 = stream1->Digest();
    auto digest2 = stream2->Digest();

    ASSERT_TRUE(digest1.IsSuccess());
    ASSERT_TRUE(digest2.IsSuccess());
    EXPECT_EQ(digest1.GetData().value(), digest2.GetData().value());

    GTEST_LOG_(INFO) << "HashStreamTest_Consistency_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_DifferentAlgorithms_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_DifferentAlgorithms_001 start";

    string testData = "test data for all algorithms";

    auto md5Result = HsHashStream::Constructor("md5");
    auto sha1Result = HsHashStream::Constructor("sha1");
    auto sha256Result = HsHashStream::Constructor("sha256");

    ASSERT_TRUE(md5Result.IsSuccess());
    ASSERT_TRUE(sha1Result.IsSuccess());
    ASSERT_TRUE(sha256Result.IsSuccess());

    unique_ptr<HsHashStream> md5Stream(md5Result.GetData().value());
    unique_ptr<HsHashStream> sha1Stream(sha1Result.GetData().value());
    unique_ptr<HsHashStream> sha256Stream(sha256Result.GetData().value());

    ArrayBuffer buf(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    md5Stream->Update(buf);
    sha1Stream->Update(buf);
    sha256Stream->Update(buf);

    auto md5Digest = md5Stream->Digest();
    auto sha1Digest = sha1Stream->Digest();
    auto sha256Digest = sha256Stream->Digest();

    ASSERT_TRUE(md5Digest.IsSuccess());
    ASSERT_TRUE(sha1Digest.IsSuccess());
    ASSERT_TRUE(sha256Digest.IsSuccess());

    EXPECT_EQ(md5Digest.GetData().value().length(), MD5_HASH_LENGTH);
    EXPECT_EQ(sha1Digest.GetData().value().length(), SHA1_HASH_LENGTH);
    EXPECT_EQ(sha256Digest.GetData().value().length(), SHA256_HASH_LENGTH);

    EXPECT_NE(md5Digest.GetData().value(), sha1Digest.GetData().value());
    EXPECT_NE(sha1Digest.GetData().value(), sha256Digest.GetData().value());

    GTEST_LOG_(INFO) << "HashStreamTest_DifferentAlgorithms_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Stress_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Stress_001 start";

    for (int i = 0; i < 50; i++) {
        auto result = HsHashStream::Constructor("sha256");
        ASSERT_TRUE(result.IsSuccess());
        unique_ptr<HsHashStream> stream(result.GetData().value());

        string data = "stress test " + to_string(i);
        ArrayBuffer buf(static_cast<void *>(const_cast<char*>(data.data())), data.length());
        auto updateRet = stream->Update(buf);
        ASSERT_TRUE(updateRet.IsSuccess());

        auto digestRet = stream->Digest();
        ASSERT_TRUE(digestRet.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashStreamTest_Stress_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_Stress_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_Stress_002 start";

    for (int i = 0; i < 50; i++) {
        auto result = HsHashStream::Constructor("md5");
        ASSERT_TRUE(result.IsSuccess());
        unique_ptr<HsHashStream> stream(result.GetData().value());

        string data = "stress test md5 " + to_string(i);
        ArrayBuffer buf(static_cast<void *>(const_cast<char*>(data.data())), data.length());
        auto updateRet = stream->Update(buf);
        ASSERT_TRUE(updateRet.IsSuccess());

        auto digestRet = stream->Digest();
        ASSERT_TRUE(digestRet.IsSuccess());
    }

    GTEST_LOG_(INFO) << "HashStreamTest_Stress_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_MultipleStreams_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_MultipleStreams_001 start";

    vector<unique_ptr<HsHashStream>> streams;

    for (int i = 0; i < 5; i++) {
        auto result = HsHashStream::Constructor("sha256");
        ASSERT_TRUE(result.IsSuccess());
        streams.push_back(unique_ptr<HsHashStream>(result.GetData().value()));
    }

    for (auto& stream : streams) {
        ASSERT_NE(stream, nullptr);
        auto [succ, entity] = stream->GetHsEntity();
        EXPECT_TRUE(succ);
    }

    GTEST_LOG_(INFO) << "HashStreamTest_MultipleStreams_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_MultipleStreams_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_MultipleStreams_002 start";

    vector<unique_ptr<HsHashStream>> streams;

    for (int i = 0; i < 5; i++) {
        auto result = HsHashStream::Constructor("md5");
        ASSERT_TRUE(result.IsSuccess());
        streams.push_back(unique_ptr<HsHashStream>(result.GetData().value()));
    }

    string testData = "test data for multiple streams";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    for (auto& stream : streams) {
        auto updateRet = stream->Update(data);
        ASSERT_TRUE(updateRet.IsSuccess());
    }

    for (auto& stream : streams) {
        auto digestRet = stream->Digest();
        ASSERT_TRUE(digestRet.IsSuccess());
        EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);
    }

    GTEST_LOG_(INFO) << "HashStreamTest_MultipleStreams_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_BinaryData_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_BinaryData_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string binaryContent;
    for (int i = 0; i < 256; i++) {
        binaryContent += static_cast<char>(i);
    }
    ArrayBuffer data(static_cast<void *>(binaryContent.data()), binaryContent.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_BinaryData_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_BinaryData_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_BinaryData_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string binaryContent(512, '\xFF');
    ArrayBuffer data(static_cast<void *>(binaryContent.data()), binaryContent.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_BinaryData_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_UnicodeData_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "中文测试内容";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_UnicodeData_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "日本語テスト";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_UnicodeData_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_003 start";

    auto result = HsHashStream::Constructor("sha1");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "한국어 테스트";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_UnicodeData_003 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_SpecialChars_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_SpecialChars_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "test\nwith\nnewlines";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest_SpecialChars_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_SpecialChars_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_SpecialChars_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string testData = "test\twith\ttabs";
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(testData.data())), testData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest_SpecialChars_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_DifferentChunkSizes_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_DifferentChunkSizes_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string fullData(1000, 'a');

    for (int offset = 0; offset < fullData.length(); offset += 100) {
        string chunk = fullData.substr(offset, 100);
        ArrayBuffer data(static_cast<void *>(const_cast<char*>(chunk.data())), chunk.length());
        auto updateRet = stream->Update(data);
        ASSERT_TRUE(updateRet.IsSuccess());
    }

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_DifferentChunkSizes_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_DifferentChunkSizes_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_DifferentChunkSizes_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string fullData(1000, 'b');

    vector<int> chunkSizes = {1, 10, 50, 100, 200};
    int offset = 0;
    for (int size : chunkSizes) {
        string chunk = fullData.substr(offset, size);
        ArrayBuffer data(static_cast<void *>(const_cast<char*>(chunk.data())), chunk.length());
        auto updateRet = stream->Update(data);
        ASSERT_TRUE(updateRet.IsSuccess());
        offset += size;
    }

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_DifferentChunkSizes_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_VeryLargeData_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string largeData(1024 * 500, 'x');
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(largeData.data())), largeData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA256_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_VeryLargeData_002, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_002 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string largeData(1024 * 500, 'y');
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(largeData.data())), largeData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), MD5_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_IncrementalHash_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_IncrementalHash_001 start";

    auto result1 = HsHashStream::Constructor("sha256");
    auto result2 = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_TRUE(result2.IsSuccess());

    unique_ptr<HsHashStream> stream1(result1.GetData().value());
    unique_ptr<HsHashStream> stream2(result2.GetData().value());

    string fullData = "This is a test message for incremental hashing";

    for (size_t i = 0; i < fullData.length(); i += 10) {
        string chunk = fullData.substr(i, 10);
        ArrayBuffer data(static_cast<void *>(const_cast<char*>(chunk.data())), chunk.length());
        stream1->Update(data);
    }

    ArrayBuffer fullBuf(static_cast<void *>(const_cast<char*>(fullData.data())), fullData.length());
    stream2->Update(fullBuf);

    auto digest1 = stream1->Digest();
    auto digest2 = stream2->Digest();

    ASSERT_TRUE(digest1.IsSuccess());
    ASSERT_TRUE(digest2.IsSuccess());
    EXPECT_EQ(digest1.GetData().value(), digest2.GetData().value());

    GTEST_LOG_(INFO) << "HashStreamTest_IncrementalHash_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_IncrementalHash_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_IncrementalHash_002 start";

    auto result1 = HsHashStream::Constructor("md5");
    auto result2 = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_TRUE(result2.IsSuccess());

    unique_ptr<HsHashStream> stream1(result1.GetData().value());
    unique_ptr<HsHashStream> stream2(result2.GetData().value());

    string fullData = "Another test message for incremental hashing with MD5";

    for (char c : fullData) {
        string chunk(1, c);
        ArrayBuffer data(static_cast<void *>(const_cast<char*>(chunk.data())), 1);
        stream1->Update(data);
    }

    ArrayBuffer fullBuf(static_cast<void *>(const_cast<char*>(fullData.data())), fullData.length());
    stream2->Update(fullBuf);

    auto digest1 = stream1->Digest();
    auto digest2 = stream2->Digest();

    ASSERT_TRUE(digest1.IsSuccess());
    ASSERT_TRUE(digest2.IsSuccess());
    EXPECT_EQ(digest1.GetData().value(), digest2.GetData().value());

    GTEST_LOG_(INFO) << "HashStreamTest_IncrementalHash_002 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_GetHsEntity_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_003 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    auto [succ, entity] = stream->GetHsEntity();
    EXPECT_TRUE(succ);
    EXPECT_NE(entity, nullptr);

    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_003 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_GetHsEntity_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_004 start";

    auto result = HsHashStream::Constructor("md5");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    auto [succ, entity] = stream->GetHsEntity();
    EXPECT_TRUE(succ);
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->algType, HASH_ALGORITHM_TYPE_MD5);

    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_004 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_GetHsEntity_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_005 start";

    auto result = HsHashStream::Constructor("sha1");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    auto [succ, entity] = stream->GetHsEntity();
    EXPECT_TRUE(succ);
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->algType, HASH_ALGORITHM_TYPE_SHA1);

    GTEST_LOG_(INFO) << "HashStreamTest_GetHsEntity_005 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_ConcurrentSafe_001, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_ConcurrentSafe_001 start";

    auto result = HsHashStream::Constructor("sha256");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string data1 = "data1";
    string data2 = "data2";
    string data3 = "data3";

    ArrayBuffer buf1(static_cast<void *>(const_cast<char*>(data1.data())), data1.length());
    ArrayBuffer buf2(static_cast<void *>(const_cast<char*>(data2.data())), data2.length());
    ArrayBuffer buf3(static_cast<void *>(const_cast<char*>(data3.data())), data3.length());

    stream->Update(buf1);
    stream->Update(buf2);
    stream->Update(buf3);

    auto digest = stream->Digest();
    ASSERT_TRUE(digest.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest_ConcurrentSafe_001 end";
}

HWTEST_F(HashStreamTest, HashStreamTest_VeryLargeData_003, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_003 start";

    auto result = HsHashStream::Constructor("sha1");
    ASSERT_TRUE(result.IsSuccess());
    unique_ptr<HsHashStream> stream(result.GetData().value());

    string largeData(1024 * 500, 'z');
    ArrayBuffer data(static_cast<void *>(const_cast<char*>(largeData.data())), largeData.length());

    auto updateRet = stream->Update(data);
    ASSERT_TRUE(updateRet.IsSuccess());

    auto digestRet = stream->Digest();
    ASSERT_TRUE(digestRet.IsSuccess());
    EXPECT_EQ(digestRet.GetData().value().length(), SHA1_HASH_LENGTH);

    GTEST_LOG_(INFO) << "HashStreamTest_VeryLargeData_003 end";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS