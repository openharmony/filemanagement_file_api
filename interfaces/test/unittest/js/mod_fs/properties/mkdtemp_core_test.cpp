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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mkdtemp_core.h"
#include "mock/uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MkdtempCoreTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path MkdtempCoreTest::tempFilePath;

void MkdtempCoreTest::SetUpTestCase(void)
{
    tempFilePath = filesystem::temp_directory_path() / "test";
    std::filesystem::create_directory(tempFilePath);
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void MkdtempCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove_all(tempFilePath);
}

void MkdtempCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void MkdtempCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MkdtempCoreTest_DoMkdtemp_0001
 * @tc.desc: Test function of DoMkdtemp() interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(MkdtempCoreTest, MkdtempCoreTest_DoMkdtemp_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreTest-begin MkdtempCoreTest_DoMkdtemp_0001";

    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;
    EXPECT_CALL(*uv, uv_fs_mkdtemp(_, _, _, _)).WillOnce(Return(0));

    string path = tempFilePath.string() + "/XXXXXX";
    auto ret = MkdtempCore::DoMkdtemp(path);
    EXPECT_EQ(ret.IsSuccess(), true);

    GTEST_LOG_(INFO) << "MkdtempCoreTest-end MkdtempCoreTest_DoMkdtemp_0001";
}

/**
 * @tc.name: MkdtempCoreTest_DoMkdtemp_0002
 * @tc.desc: Test function of DoMkdtemp() interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000IGDNF
 */
HWTEST_F(MkdtempCoreTest, MkdtempCoreTest_DoMkdtemp_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkdtempCoreTest-begin MkdtempCoreTest_DoMkdtemp_0002";

    string path = tempFilePath.string() + "/XXXXXX";

    std::shared_ptr<UvfsMock> uv = std::make_shared<UvfsMock>();
    Uvfs::ins = uv;
    EXPECT_CALL(*uv, uv_fs_mkdtemp(_, _, _, _)).WillOnce(Return(-1));

    auto ret = MkdtempCore::DoMkdtemp(path);
    EXPECT_EQ(ret.IsSuccess(), false);

    GTEST_LOG_(INFO) << "MkdtempCoreTest-end MkdtempCoreTest_DoMkdtemp_0002";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test