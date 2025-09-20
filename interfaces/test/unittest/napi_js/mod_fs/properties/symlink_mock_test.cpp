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

#include "symlink.h"

#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <memory>

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

class SymlinkMockTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        LibnMock::EnableMock();
        UvfsMock::EnableMock();
    };
    static void TearDownTestCase()
    {
        LibnMock::DisableMock();
        UvfsMock::DisableMock();
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SymlinkSync_0001
 * @tc.desc: Test function of SymlinkSync() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(SymlinkMockTest, SymlinkSync_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SymlinkMockTest-begin SymlinkSync_0001";
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

    auto libnMock_ = LibnMock::GetMock();
    auto uvMock_ = UvfsMock::GetMock();
    EXPECT_CALL(*libnMock_, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock_, ToUTF8StringPath())
        .WillOnce(testing::Return(move(srcRes)))
        .WillOnce(testing::Return(move(destRes)));
    EXPECT_CALL(*uvMock_, uv_fs_req_cleanup(testing::_));
    EXPECT_CALL(*uvMock_, uv_fs_symlink(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock_, ThrowErr(testing::_));

    auto res = Symlink::Sync(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "SymlinkMockTest-end SymlinkSync_0001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS