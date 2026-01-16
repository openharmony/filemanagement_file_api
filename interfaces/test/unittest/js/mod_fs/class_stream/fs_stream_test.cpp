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

#include "fs_stream.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "create_stream_core.h"
#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;

class FsStreamTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsStreamTest";
};

void FsStreamTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsStreamTest");
}

void FsStreamTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsStreamTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsStreamTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsStreamTest_Close_001
 * @tc.desc: Test function of FsStream::Close interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Close_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Close_001";

    auto path = testDir + "/FsStreamTest_Close_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release

    auto ret = stream->Close();

    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Close_001";
}

/**
 * @tc.name: FsStreamTest_Close_002
 * @tc.desc: Test function of FsStream::Close interface for FAILURE When repeated close.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Close_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Close_002";

    auto path = testDir + "/FsStreamTest_Close_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    auto ret = stream->Close();
    ASSERT_TRUE(ret.IsSuccess());

    ret = stream->Close(); // Repeated close

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Close_002";
}

/**
 * @tc.name: FsStreamTest_Flush_001
 * @tc.desc: Test function of FsStream::Flush interface for FAILURE When fp is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Flush_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Flush_001";

    FsStream stream(nullptr);

    auto ret = stream.Flush();

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Flush_001";
}

/**
 * @tc.name: FsStreamTest_Flush_002
 * @tc.desc: Test function of FsStream::Flush interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Flush_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Flush_002";

    auto path = testDir + "/FsStreamTest_Flush_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release

    auto ret = stream->Flush();

    EXPECT_TRUE(ret.IsSuccess());
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Flush_002";
}

/**
 * @tc.name: FsStreamTest_Seek_001
 * @tc.desc: Test function of FsStream::Seek interface for FAILURE When fp is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Seek_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Seek_001";

    FsStream stream(nullptr);

    auto ret = stream.Seek(0);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);
    EXPECT_EQ(err.GetErrMsg(), "No such file or directory");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Seek_001";
}

/**
 * @tc.name: FsStreamTest_Seek_002
 * @tc.desc: Test function of FsStream::Seek interface for FAILURE when whence < SEEK_SET.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Seek_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Seek_002";

    auto path = testDir + "/FsStreamTest_Seek_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    int32_t whence = SEEK_SET - 1;

    auto ret = stream->Seek(0, whence);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Seek_002";
}

/**
 * @tc.name: FsStreamTest_Seek_003
 * @tc.desc: Test function of FsStream::Seek interface for FAILURE when whence > SEEK_END.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Seek_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Seek_003";

    auto path = testDir + "/FsStreamTest_Seek_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    auto whence = SEEK_END + 1;

    auto ret = stream->Seek(0, whence);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Seek_003";
}

/**
 * @tc.name: FsStreamTest_Seek_004
 * @tc.desc: Test function of FsStream::Seek interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Seek_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Seek_004";

    auto path = testDir + "/FsStreamTest_Seek_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release

    auto ret = stream->Seek(1);

    EXPECT_TRUE(ret.IsSuccess());
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Seek_004";
}

/**
 * @tc.name: FsStreamTest_Write_001
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When fp is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_001";

    FsStream stream(nullptr);

    auto ret = stream.Write("content");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_001";
}

/**
 * @tc.name: FsStreamTest_Write_002
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When fp is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_002";

    ArrayBuffer data(nullptr, 10);
    FsStream stream(nullptr);

    auto ret = stream.Write(data);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_002";
}

/**
 * @tc.name: FsStreamTest_Write_003
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_003";

    auto path = testDir + "/FsStreamTest_Write_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    WriteOptions opt;
    opt.offset = -1;

    auto ret = stream->Write("content", opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_003";
}

/**
 * @tc.name: FsStreamTest_Write_004
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_004";

    auto path = testDir + "/FsStreamTest_Write_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    ArrayBuffer data(nullptr, 10);
    WriteOptions opt;
    opt.offset = -1;

    auto ret = stream->Write(data, opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_004";
}

/**
 * @tc.name: FsStreamTest_Write_005
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When encoding is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_005";

    auto path = testDir + "/FsStreamTest_Write_005.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    WriteOptions opt;
    opt.encoding = "utf-16";

    auto ret = stream->Write("content", opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_005";
}

/**
 * @tc.name: FsStreamTest_Write_006
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When encoding is unsupported.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_006";

    auto path = testDir + "/FsStreamTest_Write_006.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    ArrayBuffer data(nullptr, 10);
    WriteOptions opt;
    opt.encoding = "utf-16";

    auto ret = stream->Write(data, opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_006";
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__LP64__)
/**
 * @tc.name: FsStreamTest_Write_007
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When bufLen > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_007";

    auto path = testDir + "/FsStreamTest_Write_007.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    size_t oversizedLen = static_cast<size_t>(UINT_MAX) + 1;
    std::allocator<char> alloc;
    constexpr std::__uninitialized_size_tag uninitTag {};
    string data(uninitTag, oversizedLen, std::move(alloc)); // Will only set string size, no actual memory allocated.

    auto ret = stream->Write(data);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_007";
}

/**
 * @tc.name: FsStreamTest_Write_008
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When bufLen > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_008";

    auto path = testDir + "/FsStreamTest_Write_008.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    size_t oversizedLen = static_cast<size_t>(UINT_MAX) + 1;
    ArrayBuffer data(nullptr, oversizedLen); // Will only set buffer size, no actual memory allocated.

    auto ret = stream->Write(data);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_008";
}
#endif

/**
 * @tc.name: FsStreamTest_Write_009
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_009";

    auto path = testDir + "/FsStreamTest_Write_009.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    WriteOptions opt;
    opt.length = -1;

    auto ret = stream->Write("content", opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_009";
}

/**
 * @tc.name: FsStreamTest_Write_010
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_010";

    auto path = testDir + "/FsStreamTest_Write_010.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    ArrayBuffer data(nullptr, 10);
    WriteOptions opt;
    opt.length = -1;

    auto ret = stream->Write(data, opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_010";
}

/**
 * @tc.name: FsStreamTest_Write_013
 * @tc.desc: Test function of FsStream::Write(string) interface for FAILURE When no write permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_013";

    auto path = testDir + "/FsStreamTest_Write_013.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release

    auto ret = stream->Write("content");

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_013";
}

/**
 * @tc.name: FsStreamTest_Write_014
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE When no write permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_014";

    auto path = testDir + "/FsStreamTest_Write_014.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer data(buf, len);

    auto ret = stream->Write(data);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_014";
}

/**
 * @tc.name: FsStreamTest_Write_015
 * @tc.desc: Test function of FsStream::Write(string) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_015";

    auto path = testDir + "/FsStreamTest_Write_015.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    string data = "FsStreamTest_Write_015";
    auto expectedLen = 5;
    WriteOptions opt;
    opt.offset = 0;
    opt.encoding = "utf-8";
    opt.length = expectedLen;

    auto ret = stream->Write(data, opt);

    EXPECT_TRUE(ret.IsSuccess());
    if (ret.IsSuccess()) {
        auto writeLen = ret.GetData().value();
        EXPECT_EQ(writeLen, expectedLen);
    }
    EXPECT_EQ(FileUtils::ReadTextFileContent(path), std::make_tuple(true, ""));
    auto flushRet = stream->Flush();
    EXPECT_TRUE(flushRet.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(path), std::make_tuple(true, data.substr(0, expectedLen)));
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_015";
}

/**
 * @tc.name: FsStreamTest_Write_016
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Write_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Write_016";

    auto path = testDir + "/FsStreamTest_Write_016.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    string content = "FsStreamTest_Write_016";
    ArrayBuffer data(static_cast<void *>(content.data()), content.length() + 1);
    auto expectedLen = 5;
    WriteOptions opt;
    opt.offset = 0;
    opt.encoding = "utf-8";
    opt.length = expectedLen;

    auto ret = stream->Write(data, opt);

    EXPECT_TRUE(ret.IsSuccess());
    if (ret.IsSuccess()) {
        auto writeLen = ret.GetData().value();
        EXPECT_EQ(writeLen, expectedLen);
    }
    EXPECT_EQ(FileUtils::ReadTextFileContent(path), std::make_tuple(true, ""));
    auto flushRet = stream->Flush();
    EXPECT_TRUE(flushRet.IsSuccess());
    EXPECT_EQ(FileUtils::ReadTextFileContent(path), std::make_tuple(true, content.substr(0, expectedLen)));
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Write_016";
}

/**
 * @tc.name: FsStreamTest_Read_001
 * @tc.desc: Test function of FsStream::Read interface for FAILURE when fp is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_001";

    ArrayBuffer buffer(nullptr, 10);
    FsStream stream(nullptr);

    auto ret = stream.Read(buffer);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_001";
}

/**
 * @tc.name: FsStreamTest_Read_002
 * @tc.desc: Test function of FsStream::Read interface for FAILURE When offset < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_002";

    auto path = testDir + "/FsStreamTest_Read_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    ArrayBuffer buffer(nullptr, 10);
    ReadOptions opt;
    opt.offset = -1;

    auto ret = stream->Read(buffer, opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_002";
}

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__LP64__)
/**
 * @tc.name: FsStreamTest_Read_003
 * @tc.desc: Test function of FsStream::Read interface for FAILURE When bufLen > UINT_MAX.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_003";

    auto path = testDir + "/FsStreamTest_Read_003.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    size_t oversizedLen = static_cast<size_t>(UINT_MAX) + 1;
    ArrayBuffer buffer(nullptr, oversizedLen); // Modify size only, no actual memory allocation

    auto ret = stream->Read(buffer);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_003";
}
#endif

/**
 * @tc.name: FsStreamTest_Read_004
 * @tc.desc: Test function of FsStream::Read interface for FAILURE When length < 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_004";

    auto path = testDir + "/FsStreamTest_Read_004.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    ArrayBuffer buffer(nullptr, 10);
    ReadOptions opt;
    opt.length = -1;

    auto ret = stream->Read(buffer, opt);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);
    EXPECT_EQ(err.GetErrMsg(), "Invalid argument");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_004";
}

/**
 * @tc.name: FsStreamTest_Read_005
 * @tc.desc: Test function of FsStream::Read interface for FAILURE When no read permission.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_005";

    auto path = testDir + "/FsStreamTest_Read_005.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "w");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);

    auto ret = stream->Read(buffer);

    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_005";
}

/**
 * @tc.name: FsStreamTest_Read_006
 * @tc.desc: Test function of FsStream::Read interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamTest, FsStreamTest_Read_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamTest-begin FsStreamTest_Read_006";

    auto path = testDir + "/FsStreamTest_Read_006.txt";
    string content = "FsStreamTest_Read_006";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));

    auto streamRet = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(streamRet.IsSuccess());
    std::unique_ptr<FsStream> stream(streamRet.GetData().value()); // To smart ptr for auto memory release
    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);
    auto expectedLen = 5;
    ReadOptions opt;
    opt.offset = 0;
    opt.length = expectedLen;

    auto ret = stream->Read(buffer, opt);

    EXPECT_TRUE(ret.IsSuccess());
    if (ret.IsSuccess()) {
        auto readLen = ret.GetData().value();
        EXPECT_EQ(readLen, expectedLen);
        string readContent(buf, expectedLen);
        EXPECT_EQ(readContent, content.substr(0, expectedLen));
    }
    auto closeRet = stream->Close();
    EXPECT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamTest-end FsStreamTest_Read_006";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS