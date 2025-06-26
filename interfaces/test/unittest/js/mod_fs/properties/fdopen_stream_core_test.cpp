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
#include "fdopen_stream_core.h"

#define FDOPEN_STREAM_FILE_PATH "/data/test/FdopenStreamCoreTest.txt"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class FdopenStreamCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(FDOPEN_STREAM_FILE_PATH, CREATE | O_RDWR, 0644);
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(FDOPEN_STREAM_FILE_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: DoFdopenStreamTest_0001
* @tc.desc: Test function of DoFdopenStream() interface for success.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
*/
HWTEST_F(FdopenStreamCoreTest, DoFdopenStreamTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin DoFdopenStreamTest_0001";
    int32_t fd = open(FDOPEN_STREAM_FILE_PATH, O_RDWR);
    if (fd <= 0) {
        close(fd);
        ASSERT_TRUE(false);
    }

    auto ret = FdopenStreamCore::DoFdopenStream(fd, "r");
    ASSERT_TRUE(ret.IsSuccess());

    auto stream = ret.GetData().value();
    ASSERT_NE(stream, nullptr);
    auto retClose = stream->Close();
    EXPECT_TRUE(retClose.IsSuccess());

    close(fd);

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end DoFdopenStreamTest_0001";
}

/**
* @tc.name: DoFdopenStreamTest_0002
* @tc.desc: Test function of DoFdopenStream() interface for fd < 0.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
*/
HWTEST_F(FdopenStreamCoreTest, DoFdopenStreamTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin DoFdopenStreamTest_0002";
    auto ret = FdopenStreamCore::DoFdopenStream(-1, "r");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end DoFdopenStreamTest_0002";
}

/**
* @tc.name: DoFdopenStreamTest_0003
* @tc.desc: Test function of DoFdopenStream() interface for fail.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
*/
HWTEST_F(FdopenStreamCoreTest, DoFdopenStreamTest_0003, testing::ext::TestSize.Level1)
{
    int32_t fd = open(FDOPEN_STREAM_FILE_PATH, O_RDWR);
    if (fd <= 0) {
        close(fd);
        ASSERT_TRUE(false);
    }

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-begin DoFdopenStreamTest_0003";
    auto ret = FdopenStreamCore::DoFdopenStream(fd, "sssssss");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    close(fd);

    GTEST_LOG_(INFO) << "FdopenStreamCoreTest-end DoFdopenStreamTest_0003";
}

}
}
}