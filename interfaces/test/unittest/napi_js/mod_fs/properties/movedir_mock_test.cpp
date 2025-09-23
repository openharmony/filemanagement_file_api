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

#include "movedir.h"

#include <cstring>
#include <fcntl.h>
#include <gtest/gtest.h>

#include "securec.h"

#define MY_ERROR_VALUE 128

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

static const string g_movedirSrc = "/data/test/MovedirMockTestSrc1";
static const string g_movedirDest = "/data/test/MovedirMockTestDest1";

class MovedirMockTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        MovedirMock::EnableMock();
    };
    static void TearDownTestCase()
    {
        MovedirMock::DisableMock();
    };
    void SetUp() {};
    void TearDown() {};
};

static tuple<bool, unique_ptr<char[]>, size_t> JudgePath(const string &path)
{
    size_t srcLen = strlen(path.c_str()) + 1;
    unique_ptr<char[]> srcPtr = make_unique<char[]>(srcLen);
    auto ret = strncpy_s(srcPtr.get(), srcLen, path.c_str(), srcLen - 1);
    if (ret != EOK) {
        GTEST_LOG_(INFO) << "strncpy_s failed";
    }
    srcPtr[srcLen - 1] = '\0';
    return { true, move(srcPtr), srcLen };
}

/**
 * @tc.name: RmDirectory_0001
 * @tc.desc: Test function of RmDirectory interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MovedirMockTest, RmDirectory_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MovedirMockTest-begin RmDirectory_0001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    error_code myEcRemove(MY_ERROR_VALUE, generic_category());
    filesystem::path myPath(g_movedirSrc);
    tuple<bool, int32_t> myMode(true, DIRMODE_DIRECTORY_REPLACE);
    auto mock_ = MovedirMock::GetMock();

    auto statMock = [](const char *path, struct stat *buf) {
        buf->st_mode = S_IFDIR | 0755;
        return 0;
    };

    testing::Sequence s;
    EXPECT_CALL(*mock_, InitArgs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_, ToUTF8StringPath())
        .WillOnce(testing::Return(JudgePath(g_movedirSrc)))
        .WillOnce(testing::Return(JudgePath(g_movedirDest)));
    EXPECT_CALL(*mock_, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*mock_, ToInt32(testing::_)).WillOnce(testing::Return(myMode));
    EXPECT_CALL(*mock_, stat(testing::_, testing::_)).Times(4).InSequence(s).WillRepeatedly(testing::Invoke(statMock));
    EXPECT_CALL(*mock_, stat(testing::_, testing::_))
        .InSequence(s)
        .WillOnce(testing::Return(-1))
        .WillOnce(testing::Invoke(statMock))
        .WillOnce(testing::Return(-1));
    EXPECT_CALL(*mock_, napi_create_array(testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*mock_, ThrowErrAddData(testing::_, testing::_, testing::_));

    auto res = MoveDir::Sync(env, info);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "MovedirMockTest-end RmDirectory_0001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS