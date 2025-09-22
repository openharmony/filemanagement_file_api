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

#include <gtest/gtest.h>
#include "create_stream_core.h"

#define CREATE_STREAM_FILE_PATH "/data/test/CreateStreamCoreTest.txt"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class CreateStreamCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(CREATE_STREAM_FILE_PATH, CREATE | O_RDWR, 0644);
        if (fd < 0) {
            GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
            ASSERT_TRUE(false);
        }
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(CREATE_STREAM_FILE_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};
/**
 * @tc.name: DoCreateStreamTest_0001
 * @tc.desc: Test function of DoCreateStream() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateStreamCoreTest, DoCreateStreamTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateStreamCoreTest-begin DoCreateStreamTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(CREATE_STREAM_FILE_PATH, "r");
    ASSERT_TRUE(ret.IsSuccess());

    auto stream = ret.GetData().value();
    ASSERT_NE(stream, nullptr);
    auto retClose = stream->Close();
    EXPECT_TRUE(retClose.IsSuccess());

    GTEST_LOG_(INFO) << "CreateStreamCoreTest-end DoCreateStreamTest_0001";
}

/**
 * @tc.name: DoCreateStreamTest_0002
 * @tc.desc: Test function of DoCreateStream() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CreateStreamCoreTest, DoCreateStreamTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateStreamCoreTest-begin DoCreateStreamTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(CREATE_STREAM_FILE_PATH, "ssss");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "CreateStreamCoreTest-end DoCreateStreamTest_0002";
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
