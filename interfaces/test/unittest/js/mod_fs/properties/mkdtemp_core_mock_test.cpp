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

#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mkdtemp_core.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MkdtempCoreMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

filesystem::path MkdtempCoreMockTest::tempFilePath;

void MkdtempCoreMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = filesystem::temp_directory_path() / "test";
    std::filesystem::create_directory(tempFilePath);
    uvMock = std::make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void MkdtempCoreMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove_all(tempFilePath);
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void MkdtempCoreMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MkdtempCoreMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MkdtempCoreMockTest_DoMkdtemp_0001
 * @tc.desc: Test function of DoMkdtemp() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdtempCoreMockTest, MkdtempCoreMockTest_DoMkdtemp_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-begin MkdtempCoreMockTest_DoMkdtemp_0001";

    uv_fs_t mock_req;
    mock_req.path = const_cast<char *>("/data/local/tmp/test/XXXXXX");

    EXPECT_CALL(*uvMock, uv_fs_mkdtemp(_, _, _, _))
        .WillOnce(Invoke([&](uv_loop_t*, uv_fs_t* req, const char*, uv_fs_cb) {
            *req = mock_req;
            return 0;
        }));

    auto ret = MkdtempCore::DoMkdtemp("/data/local/tmp/test/XXXXXX");
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-end MkdtempCoreMockTest_DoMkdtemp_0001";
}

/**
 * @tc.name: MkdtempCoreMockTest_DoMkdtemp_0002
 * @tc.desc: Test function of DoMkdtemp() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(MkdtempCoreMockTest, MkdtempCoreMockTest_DoMkdtemp_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-begin MkdtempCoreMockTest_DoMkdtemp_0002";

    string path = tempFilePath.string() + "/XXXXXX";

    EXPECT_CALL(*uvMock, uv_fs_mkdtemp(_, _, _, _)).WillOnce(Return(-1));
    auto ret = MkdtempCore::DoMkdtemp(path);
    EXPECT_EQ(ret.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MkdtempCoreMockTest-end MkdtempCoreMockTest_DoMkdtemp_0002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test