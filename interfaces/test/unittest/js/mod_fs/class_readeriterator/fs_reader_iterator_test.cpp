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

#include "fs_reader_iterator.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsReaderIteratorTest : public testing::Test {
public:
    static std::filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

filesystem::path FsReaderIteratorTest::tempFilePath;

void FsReaderIteratorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = std::filesystem::temp_directory_path() / "test_file.txt";
    ofstream tempfile(tempFilePath);
    tempfile << "Test content\n123\n456";
    tempfile.close();
}

void FsReaderIteratorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove(tempFilePath);
}

void FsReaderIteratorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsReaderIteratorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsReaderIteratorTest_Constructor_001
 * @tc.desc: Test FsReaderIterator::Constructor for success case
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsReaderIteratorTest, FsReaderIteratorTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsReaderIteratorTest-begin FsReaderIteratorTest_Constructor_001";
    
    auto result = FsReaderIterator::Constructor();
    EXPECT_EQ(result.IsSuccess(), true);
    
    GTEST_LOG_(INFO) << "FsReaderIteratorTest-end FsReaderIteratorTest_Constructor_001";
}

}