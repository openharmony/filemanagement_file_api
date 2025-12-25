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
#include <filesystem>
#include <fstream>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "securec.h"

#define MY_ERROR_VALUE 128

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

namespace fs = std::filesystem;

class MovedirMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

    static constexpr std::string_view MOVE_DIR_TEST_PATH = "/data/test/MovedirMockTest";
};

void MovedirMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "MovedirMockTest");
    LibnMock::EnableMock();
}

void MovedirMockTest::TearDownTestSuite()
{
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void MovedirMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    std::error_code err;
    fs::path path = MOVE_DIR_TEST_PATH;
    fs::create_directories(path, err);
    ASSERT_FALSE(err) << "Failed to create test directory!";
    GTEST_LOG_(INFO) << "Test directory created.";
}

void MovedirMockTest::TearDown()
{
    std::error_code err;
    fs::path path = MOVE_DIR_TEST_PATH;
    fs::remove_all(path, err);
    EXPECT_FALSE(err) << "Failed to remove test directory!";
    GTEST_LOG_(INFO) << "TearDown";
}

static tuple<bool, unique_ptr<char[]>, size_t> JudgePath(const fs::path &path)
{
    auto pathSrc = path.c_str();
    auto srcLen = std::strlen(pathSrc);
    size_t destLen = srcLen + 1;
    unique_ptr<char[]> pathDest = make_unique<char[]>(destLen);
    auto ret = strncpy_s(pathDest.get(), destLen, pathSrc, srcLen);
    if (ret != EOK) {
        GTEST_LOG_(INFO) << "strncpy_s failed";
    }
    pathDest[destLen - 1] = '\0';
    return { true, move(pathDest), destLen };
}

static bool PrepareFile(const fs::path &filepath, const string_view &content)
{
    std::error_code err;
    if (fs::exists(filepath, err)) {
        if (err) {
            return false;
        }
        if (!fs::remove(filepath, err) || err) {
            return false;
        }
    }

    if (!fs::create_directories(filepath.parent_path(), err) && err) {
        return false;
    }

    std::ofstream file(filepath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.write(content.data(), content.size());
    file.close();
    if (file.fail()) {
        fs::remove(filepath, err);
        return false;
    }
    return true;
}

/**
 * @tc.name: MovedirMockTest_Sync_001
 * @tc.desc: Test function of MoveDir::Sync interface for SUCCESS when RmDirectory completes successfully.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MovedirMockTest, MovedirMockTest_Sync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MovedirMockTest-begin MovedirMockTest_Sync_001";

    fs::path parent = MOVE_DIR_TEST_PATH;
    fs::path srcDir = parent / "src";
    fs::path srcFile = srcDir / "test.txt";
    bool srcCreated = PrepareFile(srcFile, "MovedirMockTest_Sync_001");
    ASSERT_TRUE(srcCreated) << "Failed to create src file!";

    fs::path destDir = parent / "dest";
    fs::path destFile = destDir / "src/test.txt";
    bool destCreated = PrepareFile(destFile, "MovedirMockTest_Sync_001");
    ASSERT_TRUE(destCreated) << "Failed to create dest file!";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    tuple<bool, int32_t> myMode(true, DIRMODE_DIRECTORY_REPLACE);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::_, testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath())
        .WillOnce(testing::Return(JudgePath(srcDir)))
        .WillOnce(testing::Return(JudgePath(destDir)));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*libnMock, ToInt32(testing::_)).WillOnce(testing::Return(myMode));
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = MoveDir::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nv);

    GTEST_LOG_(INFO) << "MovedirMockTest-end MovedirMockTest_Sync_001";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS