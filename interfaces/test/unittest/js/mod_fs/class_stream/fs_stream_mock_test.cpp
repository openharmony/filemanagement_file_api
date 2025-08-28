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
#include "c_mock.h"
#include "create_stream_core.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;

static const string g_streamFilePath = "/data/test/FsStreamCoreTest.txt";

class FsStreamMockTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        CMock::EnableMock();
        int32_t fd = open(g_streamFilePath.c_str(), CREATE | O_RDWR, 0644);
        if (fd < 0) {
            GTEST_LOG_(ERROR) << "Open test file failed! ret: " << fd << ", errno: " << errno;
            ASSERT_TRUE(false);
        }
        close(fd);
    };
    static void TearDownTestCase()
    {
        CMock::DisableMock();
        rmdir(g_streamFilePath.c_str());
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: FsStreamSeekTest_0001
 * @tc.desc: Test function of Seek() interface for fail fseek.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamSeekTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamSeekTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(g_streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto mock_ = CMock::GetMock();
    EXPECT_CALL(*mock_, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    auto retSk = result->Seek(1);
    EXPECT_FALSE(retSk.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamSeekTest_0001";
}

/**
 * @tc.name: FsStreamSeekTest_0002
 * @tc.desc: Test function of Seek() interface for fail ftell.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamSeekTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamSeekTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(g_streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto mock_ = CMock::GetMock();
    EXPECT_CALL(*mock_, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mock_, ftell(testing::_)).WillOnce(testing::Return(-1));

    auto retSk = result->Seek(1);
    EXPECT_FALSE(retSk.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamSeekTest_0002";
}

/**
 * @tc.name: FsStreamWriteTest_0001
 * @tc.desc: Test function of Write() interface for string fail fseek.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamWriteTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamWriteTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(g_streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto mock_ = CMock::GetMock();
    EXPECT_CALL(*mock_, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    WriteOptions opt;
    opt.offset = 5;
    auto retWr = result->Write("FsStreamWriteTest_0001", opt);
    EXPECT_FALSE(retWr.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamWriteTest_0001";
}

/**
 * @tc.name: FsStreamWriteTest_0002
 * @tc.desc: Test function of Write() interface for ArrayBuffer fail fseek.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamWriteTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamWriteTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(g_streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto mock_ = CMock::GetMock();
    EXPECT_CALL(*mock_, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    WriteOptions opt;
    opt.offset = 5;
    string buf = "FsStreamWriteTest_0002";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), buf.length()), opt);
    EXPECT_FALSE(retWr.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamWriteTest_0002";
}

/**
 * @tc.name: FsStreamReadTest_0001
 * @tc.desc: Test function of Read() interface for fail fseek.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamReadTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamReadTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(g_streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);

    auto mock_ = CMock::GetMock();
    EXPECT_CALL(*mock_, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    ReadOptions opt;
    opt.offset = 5;
    auto retRd = result->Read(arrayBuffer, opt);
    EXPECT_FALSE(retRd.IsSuccess());

    free(buffer);
    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamReadTest_0001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS