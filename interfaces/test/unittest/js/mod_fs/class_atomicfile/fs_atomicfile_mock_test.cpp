/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "fs_atomicfile.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "stdio_mock.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsAtomicFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsAtomicFileMockTest";
};

void FsAtomicFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsAtomicFileMockTest");
    StdioMock::EnableMock();
}

void FsAtomicFileMockTest::TearDownTestSuite()
{
    StdioMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsAtomicFileMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsAtomicFileMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsAtomicFileMockTest_FailWrite_001
 * @tc.desc: Test function of FsAtomicFile::FailWrite interface for FAILURE when remove fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */

HWTEST_F(FsAtomicFileMockTest, FsAtomicFileMockTest_FailWrite_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-begin FsAtomicFileMockTest_FailWrite_001";

    auto entity = std::make_unique<FsAtomicFileEntity>();
    entity->newFileName = testDir + "/FsAtomicFileMockTest_FailWrite_001.txt";
    FsAtomicFile fsAtomicFile(std::move(entity));

    auto stdioMock = StdioMock::GetMock();
    EXPECT_CALL(*stdioMock, remove(testing::_)).WillOnce(testing::SetErrnoAndReturn(EIO, -1));

    auto ret = fsAtomicFile.FailWrite();

    testing::Mock::VerifyAndClearExpectations(stdioMock.get());
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900005);
    EXPECT_EQ(err.GetErrMsg(), "I/O error");

    GTEST_LOG_(INFO) << "FsAtomicFileMockTest-end FsAtomicFileMockTest_FailWrite_001";
}
}