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
#include "hash_core.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static const string g_filePath = "/data/test/HashCoreTest.txt";
class HashCoreTest : public testing::Test {
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
 * @tc.name: DoHashTest_0001
 * @tc.desc: Test function of DoHash() interface for invalid alg.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, DoHashTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin DoHashTest_0001";
    string alg = "sha128";
    auto ret = HashCore::DoHash(g_filePath, alg);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "HashCoreTest-end DoHashTest_0001";
}

/**
 * @tc.name: DoHashTest_0002
 * @tc.desc: Test function of DoHash() interface for md5 success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, DoHashTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin DoHashTest_0002";
    auto ret = HashCore::DoHash(g_filePath, "md5");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest-end DoHashTest_0002";
}

/**
 * @tc.name: DoHashTest_0003
 * @tc.desc: Test function of DoHash() interface for sha1 success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, DoHashTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin DoHashTest_0003";
    auto ret = HashCore::DoHash(g_filePath, "sha1");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest-end DoHashTest_0003";
}

/**
 * @tc.name: DoHashTest_0004
 * @tc.desc: Test function of DoHash() interface for sha256 success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, DoHashTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin DoHashTest_0004";
    auto ret = HashCore::DoHash(g_filePath, "sha256");
    ASSERT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "HashCoreTest-end DoHashTest_0004";
}

/**
 * @tc.name: DoHashTest_0005
 * @tc.desc: Test function of DoHash() interface for no such file or directory.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(HashCoreTest, DoHashTest_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HashCoreTest-begin DoHashTest_0005";
    auto ret = HashCore::DoHash("/data/local/tmp/azuxyicayhyskjeh", "sha256");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "HashCoreTest-end DoHashTest_0005";
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS