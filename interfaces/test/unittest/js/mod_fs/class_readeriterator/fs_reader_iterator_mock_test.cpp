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

#include <filesystem>
#include <fstream>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fs_reader_iterator.h"
#include "read_lines_core.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FsReaderIteratorMockTest : public testing::Test {
public:
    static filesystem::path tempFilePath;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
};

filesystem::path FsReaderIteratorMockTest::tempFilePath;

void FsReaderIteratorMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    tempFilePath = filesystem::temp_directory_path() / "FsReaderIteratorMockTest_test_file.txt";
    ofstream tempfile(tempFilePath);
    tempfile << "";
    tempfile.close();
    uvMock = make_shared<UvfsMock>();
    Uvfs::ins = uvMock;
}

void FsReaderIteratorMockTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    filesystem::remove(tempFilePath);
    Uvfs::ins = nullptr;
    uvMock = nullptr;
}

void FsReaderIteratorMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FsReaderIteratorMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FsReaderIteratorMockTest_Next_001
 * @tc.desc: Test FsReaderIterator::Next for success case
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(FsReaderIteratorMockTest, FsReaderIteratorMockTest_Next_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FsReaderIteratorMockTest-begin FsReaderIteratorMockTest_Next_001";
    
    string path = tempFilePath.string();
    optional<Options> option = nullopt;

    EXPECT_CALL(*uvMock, uv_fs_stat(_, _, _, _)).WillOnce(Return(1));

    auto result = ReadLinesCore::DoReadLines(path, option);
    EXPECT_TRUE(result.IsSuccess());

    auto iterator = result.GetData().value();
    ASSERT_NE(iterator, nullptr);

    auto nextResult = iterator->Next();
    EXPECT_TRUE(nextResult.IsSuccess());
    EXPECT_TRUE(nextResult.GetData().value().done);
    EXPECT_EQ(nextResult.GetData().value().value, "");

    GTEST_LOG_(INFO) << "FsReaderIteratorMockTest-end FsReaderIteratorMockTest_Next_001";
}

}