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
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

static const string streamFilePath = "/data/test/FsStreamCoreTest.txt";

class FsStreamTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(streamFilePath.c_str(), CREATE | O_RDWR, 0644);
        if (fd <= 0) {
            ASSERT_TRUE(false);
        }
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(streamFilePath.c_str());
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: FsStreamCloseTest_0001
 * @tc.desc: Test function of Close() interface for close success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamCloseTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamCloseTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamCloseTest_0001";
}

/**
 * @tc.name: FsStreamCloseTest_0002
 * @tc.desc: Test function of Close() interface for close fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamCloseTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamCloseTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    retCs = result->Close();
    EXPECT_FALSE(retCs.IsSuccess());
    auto err = retCs.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamCloseTest_0002";
}

/**
 * @tc.name: FsStreamFlushTest_0001
 * @tc.desc: Test function of Flush() interface for null fp.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamFlushTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamFlushTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    auto retFl = result->Flush();
    EXPECT_FALSE(retFl.IsSuccess());
    auto err = retFl.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamFlushTest_0001";
}

/**
 * @tc.name: FsStreamFlushTest_0002
 * @tc.desc: Test function of Flush() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamFlushTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamFlushTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retFl = result->Flush();
    ASSERT_TRUE(retFl.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamFlushTest_0002";
}

/**
 * @tc.name: FsStreamSeekTest_0001
 * @tc.desc: Test function of Seek() interface for null fp.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamSeekTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamSeekTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    auto retSk = result->Seek(0);
    EXPECT_FALSE(retSk.IsSuccess());
    auto err = retSk.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamSeekTest_0001";
}

/**
 * @tc.name: FsStreamSeekTest_0002
 * @tc.desc: Test function of Seek() interface for error whence < SEEK_SET (0).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamSeekTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamSeekTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    optional<int32_t> opt = -1;
    auto retSk = result->Seek(0, opt);
    EXPECT_FALSE(retSk.IsSuccess());
    auto err = retSk.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamSeekTest_0002";
}

/**
 * @tc.name: FsStreamSeekTest_0003
 * @tc.desc: Test function of Seek() interface for error whence > SEEK_END (2).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamSeekTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamSeekTest_0003";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    optional<int32_t> opt = 3;
    auto retSk = result->Seek(0, opt);
    EXPECT_FALSE(retSk.IsSuccess());
    auto err = retSk.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamSeekTest_0003";
}

/**
 * @tc.name: FsStreamSeekTest_0004
 * @tc.desc: Test function of Seek() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamSeekTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamSeekTest_0004";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retSk = result->Seek(1);
    ASSERT_TRUE(retSk.IsSuccess());

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamSeekTest_0004";
}

/**
 * @tc.name: FsStreamWriteTest_0001
 * @tc.desc: Test function of Write() interface for string single argument fail null fp.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    auto retWr = result->Write("FsStreamWriteTest_0001");
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900005);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0001";
}

/**
 * @tc.name: FsStreamWriteTest_0002
 * @tc.desc: Test function of Write() interface for ArrayBuffer single argument fail null fp.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    string buf = "FsStreamWriteTest_0001";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22));
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900005);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0002";
}

/**
 * @tc.name: FsStreamWriteTest_0003
 * @tc.desc: Test function of Write() interface for string error offset.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0003";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.offset = -1;

    auto retWr = result->Write("FsStreamWriteTest_0003", opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0003";
}

/**
 * @tc.name: FsStreamWriteTest_0004
 * @tc.desc: Test function of Write() interface for ArrayBuffer error offset.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0004";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.offset = -1;

    string buf = "FsStreamWriteTest_0004";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22), opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0004";
}

/**
 * @tc.name: FsStreamWriteTest_0005
 * @tc.desc: Test function of Write() interface for string error encoding.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0005";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.encoding = "utf-16";

    auto retWr = result->Write("FsStreamWriteTest_0005", opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0005";
}

/**
 * @tc.name: FsStreamWriteTest_0006
 * @tc.desc: Test function of Write() interface for ArrayBuffer error encoding.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0006";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.encoding = "utf-16";

    string buf = "FsStreamWriteTest_0006";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22), opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0004";
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__LP64__)
/**
 * @tc.name: FsStreamWriteTest_0007
 * @tc.desc: Test function of Write() interface for string > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0007";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    size_t largeLength = static_cast<size_t>(UINT_MAX) + 1;
    string largeString(largeLength, 'a');

    auto retWr = result->Write(largeString);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0005";
}

/**
 * @tc.name: FsStreamWriteTest_0008
 * @tc.desc: Test function of Write() interface for ArrayBuffer > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0008";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    size_t largeLength = static_cast<size_t>(UINT_MAX) + 1;
    string largeString(largeLength, 'a');

    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(largeString.data()), largeLength));
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0008";
}
#endif

/**
 * @tc.name: FsStreamWriteTest_0009
 * @tc.desc: Test function of Write() interface for string length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0009";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.length = -1;

    auto retWr = result->Write("FsStreamWriteTest_0009", opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0009";
}

/**
 * @tc.name: FsStreamWriteTest_0010
 * @tc.desc: Test function of Write() interface for ArrayBuffer length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0010";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.length = -1;

    string buf = "FsStreamWriteTest_0010";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22), opt);
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0010";
}

/**
 * @tc.name: FsStreamWriteTest_0013
 * @tc.desc: Test function of Write() interface for string no permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0013";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retWr = result->Write("FsStreamWriteTest_0013");
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900005);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0013";
}

/**
 * @tc.name: FsStreamWriteTest_0014
 * @tc.desc: Test function of Write() interface for ArrayBuffer no permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0014";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    string buf = "FsStreamWriteTest_0014";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22));
    EXPECT_FALSE(retWr.IsSuccess());
    EXPECT_EQ(retWr.GetError().GetErrNo(), 13900005);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0014";
}

/**
 * @tc.name: FsStreamWriteTest_0015
 * @tc.desc: Test function of Write() interface for string success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0015";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.offset = 0;
    opt.encoding = "utf-8";
    opt.length = 5;

    auto retWr = result->Write("FsStreamWriteTest_0015", opt);
    ASSERT_TRUE(retWr.IsSuccess());

    size_t retLen = retWr.GetData().value();
    EXPECT_EQ(retLen, 5);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0015";
}

/**
 * @tc.name: FsStreamWriteTest_0016
 * @tc.desc: Test function of Write() interface for ArrayBuffer success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamWriteTest_0016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamWriteTest_0016";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    WriteOptions opt;
    opt.offset = 0;
    opt.encoding = "utf-8";
    opt.length = 5;

    string buf = "FsStreamWriteTest_0016";
    auto retWr = result->Write(ArrayBuffer(static_cast<void *>(buf.data()), 22), opt);
    ASSERT_TRUE(retWr.IsSuccess());

    size_t retLen = retWr.GetData().value();
    EXPECT_EQ(retLen, 5);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamWriteTest_0016";
}

/**
 * @tc.name: FsStreamReadTest_0001
 * @tc.desc: Test function of Read() interface for single argument null fp.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0001";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);
    auto retRd = result->Read(arrayBuffer);
    EXPECT_FALSE(retRd.IsSuccess());
    EXPECT_EQ(retRd.GetError().GetErrNo(), 13900005);

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0001";
}

/**
 * @tc.name: FsStreamReadTest_0002
 * @tc.desc: Test function of Read() interface for error offset.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0002";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    ReadOptions opt;
    opt.offset = -1;

    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);
    auto retRd = result->Read(arrayBuffer, opt);
    EXPECT_FALSE(retRd.IsSuccess());
    EXPECT_EQ(retRd.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0002";
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__LP64__)
/**
 * @tc.name: FsStreamReadTest_0003
 * @tc.desc: Test function of Read() interface for bufLen > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0003";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    size_t largeLength = static_cast<size_t>(UINT_MAX) + 1;

    void *buffer = std::malloc(largeLength);
    ArrayBuffer arrayBuffer(buffer, largeLength);
    auto retRd = result->Read(arrayBuffer);
    EXPECT_FALSE(retRd.IsSuccess());
    EXPECT_EQ(retRd.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0003";
}
#endif

/**
 * @tc.name: FsStreamReadTest_0004
 * @tc.desc: Test function of Read() interface for error length.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0004";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    ReadOptions opt;
    opt.length = -1;

    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);
    auto retRd = result->Read(arrayBuffer, opt);
    EXPECT_FALSE(retRd.IsSuccess());
    EXPECT_EQ(retRd.GetError().GetErrNo(), 13900020);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0004";
}

/**
 * @tc.name: FsStreamReadTest_0005
 * @tc.desc: Test function of Read() interface for no permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0005";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "w");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);
    auto retRd = result->Read(arrayBuffer);
    EXPECT_FALSE(retRd.IsSuccess());
    EXPECT_EQ(retRd.GetError().GetErrNo(), 13900005);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0005";
}

/**
 * @tc.name: FsStreamReadTest_0006
 * @tc.desc: Test function of Read() interface for success.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamReadTest_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamReadTest_0006";
    auto ret = CreateStreamCore::DoCreateStream(streamFilePath, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    auto result = ret.GetData().value();

    ReadOptions opt;
    opt.offset = 0;
    opt.length = 5;
    void *buffer = std::malloc(4096);
    ArrayBuffer arrayBuffer(buffer, 4096);

    auto retRd = result->Read(arrayBuffer, opt);
    ASSERT_TRUE(retRd.IsSuccess());

    size_t retLen = retRd.GetData().value();
    EXPECT_EQ(retLen, 0);

    auto retCs = result->Close();
    ASSERT_TRUE(retCs.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamReadTest_0006";
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS