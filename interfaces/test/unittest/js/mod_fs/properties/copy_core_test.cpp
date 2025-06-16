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

#include "copy_core.h"

#include <fcntl.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CopyCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CopyCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    int32_t fd = open("/data/test/src.txt", O_CREAT | O_RDWR, 0644);
    close(fd);
}

void CopyCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    rmdir("/data/test/CopyCoreTest.txt");
}

void CopyCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CopyCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_001
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_001";

    string src = "invalid://data/test/src.txt";
    string dest = "file://data/test/dest.txt";

    auto res = CopyCore::ValidParams(src, dest);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_001";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_002
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_002";

    string src = "file://data/test/src.txt";
    string dest = "invalid://data/test/dest.txt";

    auto res = CopyCore::ValidParams(src, dest);
    EXPECT_EQ(res, false);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_002";
}

/**
 * @tc.name: CopyCoreTest_ValidParams_003
 * @tc.desc: Test function of CopyCore::ValidParams interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_ValidParams_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_ValidParams_003";

    string src = "file://data/test/src.txt";
    string dest = "file://data/test/dest.txt";

    auto res = CopyCore::ValidParams(src, dest);

    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_ValidParams_003";
}

/**
 * @tc.name: CopyCoreTest_CreateFileInfos_001
 * @tc.desc: Test function of CopyCore::CreateFileInfos interface for FALSE.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(CopyCoreTest, CopyCoreTest_CreateFileInfos_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyCoreTest-begin CopyCoreTest_CreateFileInfos_001";

    string src = "file://data/test/src.txt";
    string dest = "file://data/test/dest.txt";
    optional<CopyOptions> options = std::make_optional<CopyOptions>();
    options->listener =  std::make_shared<IProgressListener>();
    options->


    auto res = CopyCore::CreateFileInfos(src, dest, options);

    EXPECT_EQ(res, true);

    GTEST_LOG_(INFO) << "CopyCoreTest-end CopyCoreTest_CreateFileInfos_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test