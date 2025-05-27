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
#include "close_core.h"
#include "open_core.h"

#define FILE_PATH "/data/test/CloseCoreTest.txt"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class CloseCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void) {
        int32_t fd = open(FILE_PATH, CREATE | O_RDWR, 0644);
        close(fd);
    };
    static void TearDownTestCase() {
        rmdir(FILE_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: DoCloseTestFd_0001
* @tc.desc: Test function of DoClose() interface for invalid arguments.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(CloseCoreTest, DoCloseTestFd_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin DoCloseTestFd_0001";
    auto ret = CloseCore::DoClose(-1);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);


    GTEST_LOG_(INFO) << "CloseCoreTest-end DoCloseTestFd_0001";
}

/**
* @tc.name: DoCloseTestFd_0002
* @tc.desc: Test function of DoClose() interface for sucess.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(CloseCoreTest, DoCloseTestFd_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin DoCloseTestFd_0002";
    int32_t fd = open(FILE_PATH, O_RDWR);
    if (fd <= 0) {
        close(fd);
        ASSERT_TRUE(false);
    }

    auto ret = CloseCore::DoClose(fd);
    EXPECT_TRUE(ret.IsSuccess());

    int32_t fdEnd = open(FILE_PATH, O_RDWR);
    if (fdEnd <= 0) {
        close(fdEnd);
        ASSERT_TRUE(false);
    }
    EXPECT_EQ(fdEnd, fd);

    ret = CloseCore::DoClose(fd);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "CloseCoreTest-end DoCloseTestFd_0002";
}

/**
* @tc.name: DoCloseTestFd_0003
* @tc.desc: Test function of DoClose() interface for failed.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(CloseCoreTest, DoCloseTestFd_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin DoCloseTestFd_0003";
    int32_t fd = open(FILE_PATH, O_RDWR);
    if (fd <= 0) {
        close(fd);
        ASSERT_TRUE(false);
    }
    auto ret = CloseCore::DoClose(fd);
    EXPECT_TRUE(ret.IsSuccess());

    ret = CloseCore::DoClose(fd);
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900008);

    GTEST_LOG_(INFO) << "CloseCoreTest-end DoCloseTestFd_0003";
}

/**
* @tc.name: DoCloseTestFile_0001
* @tc.desc: Test function of DoClose() interface for success.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(CloseCoreTest, DoCloseTestFile_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin DoCloseTestFile_0001";
    auto fileRes = OpenCore::DoOpen(FILE_PATH);
    if (!fileRes.IsSuccess()) {
        ASSERT_TRUE(false);
    }
    FsFile *file = fileRes.GetData().value();
    auto ret = CloseCore::DoClose(file);
    EXPECT_TRUE(ret.IsSuccess());

    GTEST_LOG_(INFO) << "CloseCoreTest-end DoCloseTestFile_0001";
}

/**
* @tc.name: DoCloseTestFile_0002
* @tc.desc: Test function of DoClose() interface for failed get fd.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(CloseCoreTest, DoCloseTestFile_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseCoreTest-begin DoCloseTestFile_0002";
    auto fileRes = OpenCore::DoOpen(FILE_PATH);
    if (!fileRes.IsSuccess()) {
        ASSERT_TRUE(false);
    }
    FsFile *file = fileRes.GetData().value();
    auto ret = CloseCore::DoClose(file);
    EXPECT_TRUE(ret.IsSuccess());

    ret = CloseCore::DoClose(file);
    EXPECT_FALSE(ret.IsSuccess());
    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "CloseCoreTest-end DoCloseTestFile_0002";
}

}
}
}