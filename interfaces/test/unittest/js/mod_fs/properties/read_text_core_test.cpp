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

#include <gtest/gtest.h>
#include <read_text_core.h>

#define FILE_INFO "hello world"
#define FILE_PATH "/data/test/ReadTextCoreTest.txt"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

const string content = "hello world";

class ReadTextCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        int32_t fd = open(FILE_PATH, CREATE | O_RDWR, 0644);
        write(fd, content.c_str(), content.length());
        close(fd);
    };
    static void TearDownTestCase()
    {
        rmdir(FILE_PATH);
    };
    void SetUp() {};
    void TearDown() {};
};
/**
* @tc.name: DoReadTextTest_0001
* @tc.desc: Test function of DoReadText() interface for single argument.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0001";
    auto ret = ReadTextCore::DoReadText(FILE_PATH);
    EXPECT_TRUE(ret.IsSuccess());

    const auto &resText = ret.GetData().value();
    string res = std::get<0>(resText);
    EXPECT_EQ(res, FILE_INFO);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0001";
}

/**
* @tc.name: DoReadTextTest_0002
* @tc.desc: Test function of DoReadText() interface for invalid offset.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0002";
    ReadTextOptions options;
    options.offset = -1;
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0002";
}

/**
* @tc.name: DoReadTextTest_0003
* @tc.desc: Test function of DoReadText() interface for invalid length < 0.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0003";
    ReadTextOptions options;
    options.length = -5;
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0003";
}

/**
* @tc.name: DoReadTextTest_0004
* @tc.desc: Test function of DoReadText() interface for invalid length > UINT_MAX.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0004";
    ReadTextOptions options;
    options.length = static_cast<int64_t>(UINT_MAX) + 1;
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0004";
}

/**
* @tc.name: DoReadTextTest_0005
* @tc.desc: Test function of DoReadText() interface for invalid encoding.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0005";
    ReadTextOptions options;
    options.encoding = "gbk";
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0005";
}

/**
* @tc.name: DoReadTextTest_0006
* @tc.desc: Test function of DoReadText() interface for no such file or directory.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0006";
    auto ret = ReadTextCore::DoReadText("ReadTextCoreTest-begin-DoReadTextTest_0006.txt");
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900002);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0006";
}

/**
* @tc.name: DoReadTextTest_0007
* @tc.desc: Test function of DoReadText() interface for huge offset.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0007";
    ReadTextOptions options;
    options.offset = 1000000; // 假设文件较小
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    EXPECT_FALSE(ret.IsSuccess());

    auto err = ret.GetError();
    EXPECT_EQ(err.GetErrNo(), 13900020);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0007";
}

/**
* @tc.name: DoReadTextTest_0008
* @tc.desc: Test function of DoReadText() interface for no such file or directory.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0008";
    ReadTextOptions options;
    options.length = 1000000; // 超过实际大小
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    ASSERT_TRUE(ret.IsSuccess());

    const auto &resText = ret.GetData().value();
    string res = std::get<0>(resText);
    EXPECT_EQ(res, FILE_INFO);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0008";
}

/**
* @tc.name: DoReadTextTest_0009
* @tc.desc: Test function of DoReadText() interface for success.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(ReadTextCoreTest, DoReadTextTest_0009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTextCoreTest-begin DoReadTextTest_0009";
    ReadTextOptions options;
    options.offset = 2;
    options.length = 5;
    auto ret = ReadTextCore::DoReadText(FILE_PATH, options);
    ASSERT_TRUE(ret.IsSuccess());

    const auto &resText = ret.GetData().value();
    string res = std::get<0>(resText);
    std::string extracted = std::string(FILE_INFO).substr(
        options.offset.value(), options.length.value());
    EXPECT_EQ(res, extracted);

    GTEST_LOG_(INFO) << "ReadTextCoreTest-end DoReadTextTest_0009";
}

}
}
}