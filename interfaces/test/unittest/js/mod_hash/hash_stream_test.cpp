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

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS