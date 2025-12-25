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

#include <memory>

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "rust_file.h"
#include "ut_file_utils.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsReaderIteratorTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    const string testDir = FileUtils::testRootDir + "/FsReaderIteratorTest";
};

void FsReaderIteratorTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "FsReaderIteratorTest");
}

void FsReaderIteratorTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void FsReaderIteratorTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
}

void FsReaderIteratorTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsReaderIteratorTest_Constructor_001
 * @tc.desc: Test function of FsReaderIterator::Constructor interface for SUCCESS.
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

/**
 * @tc.name: FsReaderIteratorTest_Next_001
 * @tc.desc: Test function of FsReaderIterator::Next interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsReaderIteratorTest, FsReaderIteratorTest_Next_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsReaderIteratorTest-begin FsReaderIteratorTest_Next_001";

    // Prepare test parameters
    auto path = testDir + "/FsReaderIteratorTest_Next_001.txt";
    auto content = "FsReaderIteratorTest_Next_001 Content";
    ASSERT_TRUE(FileUtils::CreateFile(path, content));
    // Prepare test condition
    auto iterator = ::ReaderIterator(path.c_str());
    ASSERT_NE(iterator, nullptr);
    auto entity = std::make_unique<ReaderIteratorEntity>();
    entity->iterator = iterator;
    FsReaderIterator fsReaderIterator(std::move(entity));
    // Do testing
    auto nextResult = fsReaderIterator.Next();
    // Verify results
    ASSERT_TRUE(nextResult.IsSuccess());
    EXPECT_TRUE(nextResult.GetData().value().done);
    EXPECT_EQ(nextResult.GetData().value().value, content);

    GTEST_LOG_(INFO) << "FsReaderIteratorTest-end FsReaderIteratorTest_Next_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test