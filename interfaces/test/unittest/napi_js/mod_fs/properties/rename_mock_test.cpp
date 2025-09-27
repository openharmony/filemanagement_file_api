/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rename.h"

#include <cstring>
#include <fcntl.h>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "securec.h"
#include "uv_fs_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class RenameMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RenameMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "RenameMockTest");
    LibnMock::EnableMock();
    UvFsMock::EnableMock();
}

void RenameMockTest::TearDownTestCase(void)
{
    LibnMock::DisableMock();
    UvFsMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RenameMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void RenameMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RenameMockTest_Sync_001
 * @tc.desc: Test function of Rename::Sync interface for FAILURE when uv_fs_rename fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(RenameMockTest, RenameMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameMockTest-begin RenameMockTest_Sync_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "hello world";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    const char *initDest = "hello world";
    strLen = strlen(initDest) + 1;
    auto destPtr = make_unique<char[]>(strLen);
    ASSERT_NE(destPtr, nullptr);
    ret = strncpy_s(destPtr.get(), strLen, initDest, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> destRes = { true, move(destPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(move(srcRes)))
        .WillOnce(testing::Return(move(destRes)));
    EXPECT_CALL(*uvMock, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock, uv_fs_rename(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Rename::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "RenameMockTest-end RenameMockTest_Sync_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS