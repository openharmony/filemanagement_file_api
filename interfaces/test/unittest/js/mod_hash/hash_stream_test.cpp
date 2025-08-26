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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include "hs_hashstream.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static const string g_filePath = "/data/test/HashStreamTest.txt";
class HashStreamTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(g_filePath.c_str(), O_CREAT | O_RDWR, 0644);
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(g_filePath.c_str());
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: GetHsEntityTest_0001
 * @tc.desc: Test function of GetHsEntity() interface for null entity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, GetHsEntityTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin GetHsEntityTest_0001";
    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    HsHashStream stream(move(streamEntity));
    stream.entity = nullptr;
    auto [succ, entity] = stream.GetHsEntity();
    EXPECT_FALSE(succ);

    GTEST_LOG_(INFO) << "HashStreamTest-end GetHsEntityTest_0001";
}

/**
 * @tc.name: GetHsEntityTest_0002
 * @tc.desc: Test function of GetHsEntity() interface for has entity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, GetHsEntityTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin GetHsEntityTest_0002";
    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    HsHashStream stream(move(streamEntity));
    auto [succ, entity] = stream.GetHsEntity();
    EXPECT_TRUE(succ);
    entity = nullptr;

    GTEST_LOG_(INFO) << "HashStreamTest-end GetHsEntityTest_0002";
}

/**
 * @tc.name: UpdateTest_0001
 * @tc.desc: Test function of Update() interface for null entity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, UpdateTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin UpdateTest_0001";
    unique_ptr<HsHashStreamEntity> streamEntity = make_unique<HsHashStreamEntity>();
    HsHashStream stream(move(streamEntity));
    stream.entity = nullptr;
    ArrayBuffer buffer(nullptr, 0);

    auto ret = stream.Update(buffer);
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end UpdateTest_0001";
}

/**
 * @tc.name: UpdateTest_0002
 * @tc.desc: Test function of Update() interface for md5.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, UpdateTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin UpdateTest_0002";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_MD5;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    streamEntity->md5Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    int buffer[4096];
    ArrayBuffer arrayBuffer{ buffer, 4096 };

    auto ret = stream.Update(arrayBuffer);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end UpdateTest_0002";
}

/**
 * @tc.name: UpdateTest_0003
 * @tc.desc: Test function of Update() interface for sha1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, UpdateTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin UpdateTest_0003";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA1;
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    streamEntity->shaCtx = ctx;
    HsHashStream stream(move(streamEntity));

    int buffer[4096];
    ArrayBuffer arrayBuffer{ buffer, 4096 };

    auto ret = stream.Update(arrayBuffer);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end UpdateTest_0003";
}

/**
 * @tc.name: UpdateTest_0004
 * @tc.desc: Test function of Update() interface for sha256.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, UpdateTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin UpdateTest_0004";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA1;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    streamEntity->sha256Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    int buffer[4096];
    ArrayBuffer arrayBuffer{ buffer, 4096 };

    auto ret = stream.Update(arrayBuffer);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end UpdateTest_0004";
}

/**
 * @tc.name: DigestTest_0001
 * @tc.desc: Test function of Digest() interface for null entity.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, DigestTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin DigestTest_0001";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    HsHashStream stream(move(streamEntity));
    stream.entity = nullptr;

    auto ret = stream.Digest();
    EXPECT_FALSE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end DigestTest_0001";
}

/**
 * @tc.name: DigestTest_0002
 * @tc.desc: Test function of Digest() interface for md5.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, DigestTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin DigestTest_0002";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_MD5;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    streamEntity->md5Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end DigestTest_0002";
}

/**
 * @tc.name: DigestTest_0003
 * @tc.desc: Test function of Digest() interface for sha1.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, DigestTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin DigestTest_0003";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA1;
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    streamEntity->shaCtx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end DigestTest_0003";
}

/**
 * @tc.name: DigestTest_0004
 * @tc.desc: Test function of Digest() interface for sha256.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashStreamTest, DigestTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashStreamTest-begin DigestTest_0004";
    HsHashStreamEntity *rawPtr = new (std::nothrow) HsHashStreamEntity();
    unique_ptr<HsHashStreamEntity> streamEntity(rawPtr);
    streamEntity->algType = HASH_ALGORITHM_TYPE_SHA256;
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    streamEntity->sha256Ctx = ctx;
    HsHashStream stream(move(streamEntity));

    auto ret = stream.Digest();
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashStreamTest-end DigestTest_0004";
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS