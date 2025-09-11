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

#include "read_text.h"

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

class ReadTextMockTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        LibnMock::EnableMock();
        UvfsMock::EnableMock();
    };
    static void TearDownTestCase()
    {
        LibnMock::DisableMock();
        UvfsMock::EnableMock();
    };
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: ReadTextSync_0001
 * @tc.desc: Test function of ReadTextSync() interface for fail.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ReadTextMockTest, ReadTextSync_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextMockTest-begin ReadTextSync_0001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    napi_value val = reinterpret_cast<napi_value>(0x1000);
    NVal myOp(env, val);

    const char* initStr = "hello world";
    size_t strLen = strlen (initStr) + 1;
    auto strPtr = make_unique<char []>(strLen);
    ASSERT_NE(strPtr, nullptr);
    auto ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfRes = { true, move(strPtr), 1 };
    tuple<bool, int64_t> toIntRes = { false, -1 };

    auto libnMock_ = LibnMock::GetMock();
    EXPECT_CALL(*libnMock_, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock_, ToUTF8StringPath()).WillOnce(testing::Return(move(toUtfRes)));
    EXPECT_CALL(*libnMock_, HasProp(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock_, GetProp(testing::_)).Times(2).WillRepeatedly(testing::Return(myOp));
    EXPECT_CALL(*libnMock_, TypeIs(testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock_, ToInt64()).WillOnce(testing::Return(toIntRes));
    EXPECT_CALL(*libnMock_, ThrowErr(testing::_));

    auto res = ReadText::Sync(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "ReadTextMockTest-end ReadTextSync_0001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS