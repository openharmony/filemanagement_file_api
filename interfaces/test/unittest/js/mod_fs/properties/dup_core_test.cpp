/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "dup_core.h"

#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DupCoreTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/DupCoreTest";
};

void DupCoreTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "DupCoreTest");
}

void DupCoreTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void DupCoreTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void DupCoreTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DupCoreTest_DoDup_001
 * @tc.desc: Test function of DupCore::DoDup interface for FAILURE when fd is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupCoreTest, DupCoreTest_DoDup_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin DupCoreTest_DoDup_001";
    int32_t fd = -1;
    auto res = DupCore::DoDup(fd);

    EXPECT_EQ(res.IsSuccess(), false);

    GTEST_LOG_(INFO) << "NClassTest-end DupCoreTest_DoDup_001";
}

/**
 * @tc.name: DupCoreTest_DoDup_002
 * @tc.desc: Test function of DupCore::DoDup interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(DupCoreTest, DupCoreTest_DoDup_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin DupCoreTest_DoDup_002";

    string path = testDir + "/DupCoreTest_DoDup_002.txt";
    ASSERT_TRUE(FileUtils::CreateFile(path, "content"));

    int fd = open(path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ASSERT_GT(fd, -1);

    auto res = DupCore::DoDup(fd);

    EXPECT_TRUE(res.IsSuccess());
    close(fd);
    if (res.IsSuccess()) {
        auto *file = res.GetData().value();
        ASSERT_NE(file, nullptr);
        auto fdRes = file->GetFD();
        EXPECT_TRUE(fdRes.IsSuccess());
        if (fdRes.IsSuccess()) {
            auto dupFd = fdRes.GetData().value();
            close(dupFd);
            EXPECT_NE(fd, dupFd);
        }
        delete file;
        file = nullptr;
    }

    GTEST_LOG_(INFO) << "NClassTest-end DupCoreTest_DoDup_002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test