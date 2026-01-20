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

#include "create_stream_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "fs_utils.h"
#include "stdio_mock.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsStreamMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsStreamMockTest";
};

void FsStreamMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsStreamMockTest");
    StdioMock::EnableMock();
}

void FsStreamMockTest::TearDownTestSuite()
{
    StdioMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsStreamMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsStreamMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsStreamMockTest_Seek_001
 * @tc.desc: Test function of FsStream::Seek interface for FAILURE when fseek fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamMockTest_Seek_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamMockTest_Seek_001";

    string path = testDir + "/FsStreamMockTest_Seek_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    auto ret = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto seekRet = stream->Seek(1);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(seekRet.IsSuccess());
    auto err = seekRet.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    ASSERT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamMockTest_Seek_001";
}

/**
 * @tc.name: FsStreamMockTest_Seek_002
 * @tc.desc: Test function of FsStream::Seek interface for FAILURE when ftell fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamMockTest_Seek_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamMockTest_Seek_002";

    string path = testDir + "/FsStreamMockTest_Seek_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    auto ret = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*stdioMock, ftell(testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto seekRet = stream->Seek(1);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(seekRet.IsSuccess());
    auto err = seekRet.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    ASSERT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamMockTest_Seek_002";
}

/**
 * @tc.name: FsStreamMockTest_Write_001
 * @tc.desc: Test function of FsStream::Write(String) interface for FAILURE when fseek fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamMockTest_Write_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamMockTest_Write_001";

    string path = testDir + "/FsStreamMockTest_Write_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    WriteOptions opt;
    opt.offset = 1;
    auto ret = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto writeRet = stream->Write("FsStreamMockTest_Write_001", opt);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(writeRet.IsSuccess());
    auto err = writeRet.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    ASSERT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamMockTest_Write_001";
}

/**
 * @tc.name: FsStreamMockTest_Write_002
 * @tc.desc: Test function of FsStream::Write(ArrayBuffer) interface for FAILURE when fseek fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamMockTest_Write_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamMockTest_Write_002";

    string path = testDir + "/FsStreamMockTest_Write_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    WriteOptions opt;
    opt.offset = 1;
    auto ret = CreateStreamCore::DoCreateStream(path, "w+");
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);
    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer data(buf, len);

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto writeRet = stream->Write(data, opt);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(writeRet.IsSuccess());
    auto err = writeRet.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    ASSERT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamMockTest_Write_002";
}

/**
 * @tc.name: FsStreamMockTest_Read_001
 * @tc.desc: TTest function of FsStream::Read interface for FAILURE when fseek fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsStreamMockTest, FsStreamMockTest_Read_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsStreamMockTest-begin FsStreamMockTest_Read_001";

    string path = testDir + "/FsStreamMockTest_Read_001.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));
    ReadOptions opt;
    opt.offset = 1;
    auto ret = CreateStreamCore::DoCreateStream(path, "r+");
    ASSERT_TRUE(ret.IsSuccess());
    std::unique_ptr<FsStream> stream(ret.GetData().value()); // To smart ptr for auto memory release
    ASSERT_NE(stream, nullptr);
    const size_t len = 10;
    char buf[len] = { 0 };
    ArrayBuffer buffer(buf, len);

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, fseek(testing::_, testing::_, testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto readRet = stream->Read(buffer, opt);

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(readRet.IsSuccess());
    auto err = readRet.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");
    auto closeRet = stream->Close();
    ASSERT_TRUE(closeRet.IsSuccess());

    GTEST_LOG_(INFO) << "FsStreamMockTest-end FsStreamMockTest_Read_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test