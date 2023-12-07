/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <string>
#include <unistd.h>

#include "rust_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class RustTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};
/**
* @tc.name: RustTest_ReaderIterator_0001
* @tc.desc: Test function of ReaderIterator() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_ReaderIterator_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_ReaderIterator_0001";

    char filePath[] = "/data/test/rust_test.txt";
    void *iter = ReaderIterator(filePath);
    EXPECT_TRUE(iter != nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_ReaderIterator_0001";
}

/**
* @tc.name: RustTest_ReaderIterator_0002
* @tc.desc: Test function of ReaderIterator() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_ReaderIterator_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_ReaderIterator_0002";
    
    char filePath[] = "/data/test/rust_false.txt";
    void *iter = ReaderIterator(filePath);
    ASSERT_TRUE(errno == 2);
    EXPECT_TRUE(iter == nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_ReaderIterator_0002";
}

/**
* @tc.name: RustTest_ReaderIterator_0003
* @tc.desc: Test function of ReaderIterator() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_ReaderIterator_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_ReaderIterator_0003";
    
    void *iter = ReaderIterator(nullptr);
    ASSERT_TRUE(errno = 22);
    EXPECT_TRUE(iter == nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_ReaderIterator_0003";
}

/**
* @tc.name: RustTest_NextLine_0001
* @tc.desc: Test function of NextLine() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_NextLine_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_NextLine_0001";

    char filePath[] = "/data/test/rust_test.txt";
    void *iter = ReaderIterator(filePath);
    Str *ret = NextLine(iter);
    ASSERT_TRUE(ret != nullptr);
    int result = strcmp(ret->str, "abc\n");
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(ret->len == 4);
    StrFree(ret);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_NextLine_0001";
}

/**
* @tc.name: RustTest_NextLine_0002
* @tc.desc: Test function of NextLine() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_NextLine_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_NextLine_0002";

    char filePath[] = "/data/test/false.txt";
    void *iter = ReaderIterator(filePath);
    Str *ret = NextLine(iter);
    EXPECT_TRUE(ret == nullptr);
    StrFree(ret);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_NextLine_0002";
}

/**
* @tc.name: RustTest_NextLine_0003
* @tc.desc: Test function of NextLine() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_NextLine_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_NextLine_0003";

    Str *ret = NextLine(nullptr);
    EXPECT_TRUE(ret == nullptr);
    StrFree(ret);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_NextLine_0003";
}

/**
* @tc.name: RustTest_Lseek_0001
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0001";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = -15;
    long long ret = Lseek(fd, offset, END);
    EXPECT_TRUE(ret == 105);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0001";
}

/**
* @tc.name: RustTest_Lseek_0002
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0002";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = -15;
    long long ret = Lseek(fd, offset, END);
    EXPECT_TRUE(ret == 105);
    offset = -10;
    ret = Lseek(fd, offset, CURRENT);
    EXPECT_TRUE(ret == 95);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0002";
}

/**
* @tc.name: RustTest_Lseek_0003
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0003";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = 20;
    long long ret = Lseek(fd, offset, START);
    EXPECT_TRUE(ret == 20);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0003";
}

/**
* @tc.name: RustTest_Lseek_0004
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0004";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = -10;
    long long ret = Lseek(fd, offset, START);
    EXPECT_TRUE(ret == -1);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0004";
}

/**
* @tc.name: RustTest_Lseek_0005
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0005";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = 20;
    long long ret = Lseek(fd, offset, START);
    EXPECT_TRUE(ret == 20);
    offset = 18;
    ret = Lseek(fd, offset, CURRENT);
    EXPECT_TRUE(ret == 38);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0005";
}

/**
* @tc.name: RustTest_Lseek_0006
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0006";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    ASSERT_TRUE(fd >= 0);
    long long offset = 30;
    long long ret = Lseek(fd, offset, END);
    EXPECT_TRUE(ret == 150);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0006";
}

/**
* @tc.name: RustTest_Lseek_0007
* @tc.desc: Test function of Lseek() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGCS3
*/
HWTEST_F(RustTest, RustTest_Lseek_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Lseek_0007";

    long long offset = 30;
    long long ret = Lseek(34, offset, END);
    EXPECT_TRUE(ret == -1);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Lseek_0007";
}

/**
* @tc.name: RustTest_Mkdirs_0001
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0001";

    char filePathSingle[] = "/data/test/test1";
    int ret1 = access(filePathSingle, F_OK);
    ASSERT_TRUE(ret1 == -1);
    int ret = Mkdirs(filePathSingle, SINGLE);
    EXPECT_TRUE(ret == 0);
    ret1 = access(filePathSingle, F_OK);
    ASSERT_TRUE(ret1 == 0);
    int ret2 = remove(filePathSingle);
    EXPECT_TRUE(ret2 == 0);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0001";
}

/**
* @tc.name: RustTest_Mkdirs_0002
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0002";

    char filePathMultiple[] = "/data/test/test2/test3";
    int ret1 = access(filePathMultiple, F_OK);
    ASSERT_TRUE(ret1 == -1);
    int ret = Mkdirs(filePathMultiple, MULTIPLE);
    EXPECT_TRUE(ret == 0);
    ret1 = access(filePathMultiple, F_OK);
    ASSERT_TRUE(ret1 == 0);
    int ret2 = remove(filePathMultiple);
    EXPECT_TRUE(ret2 == 0);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0002";
}

/**
* @tc.name: RustTest_Mkdirs_0003
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0003";

    char filePathSingle[] = "/data/test/testfalse/test";
    int ret1 = access(filePathSingle, F_OK);
    ASSERT_TRUE(ret1 == -1);
    int ret = Mkdirs(filePathSingle, SINGLE);
    EXPECT_TRUE(ret == -1);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0003";
}

/**
* @tc.name: RustTest_Mkdirs_0004
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0004";

    char filePathMultiplr[] = "/data/test";
    int ret = Mkdirs(filePathMultiplr, MULTIPLE);
    EXPECT_TRUE(ret == 0);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0004";
}

/**
* @tc.name: RustTest_Mkdirs_0005
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0005";

    char filePath[] = "";
    int ret = Mkdirs(filePath, SINGLE);
    ASSERT_TRUE(errno == 2);
    EXPECT_TRUE(ret == -1);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0005";
}

/**
* @tc.name: RustTest_Mkdirs_0006
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0006";

    char filePath[] = "";
    int ret = Mkdirs(filePath, MULTIPLE);
    ASSERT_TRUE(errno == 2);
    EXPECT_TRUE(ret == 0);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0006";
}

/**
* @tc.name: RustTest_Mkdirs_0007
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0007";

    char filePath[] = "/data/test/rust_test.txt";
    int ret = Mkdirs(filePath, SINGLE);
    ASSERT_TRUE(errno == 17);
    EXPECT_TRUE(ret == -1);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0007";
}

/**
* @tc.name: RustTest_Mkdirs_0008
* @tc.desc: Test function of Mkdirs() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNJ
*/
HWTEST_F(RustTest, RustTest_Mkdirs_0008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_Mkdirs_0008";

    char filePath[] = "/data/test/rust_test.txt";
    int ret = Mkdirs(filePath, MULTIPLE);
    ASSERT_TRUE(errno == 17);
    EXPECT_TRUE(ret == -1);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_Mkdirs_0008";
}

/**
* @tc.name: RustTest_GetParent_0001
* @tc.desc: Test function of GetParent() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNL
*/
HWTEST_F(RustTest, RustTest_GetParent_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_GetParent_0001";

    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    Str *str = GetParent(fd);
    ASSERT_TRUE(str != nullptr);
    int result = strcmp(str->str, "/data/test");
    EXPECT_TRUE(result == 0);
    StrFree(str);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_GetParent_0001";
}

/**
* @tc.name: RustTest_GetParent_0002
* @tc.desc: Test function of GetParent() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNL
*/
HWTEST_F(RustTest, RustTest_GetParent_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_GetParent_0002";

    Str *str = GetParent(-1);
    ASSERT_TRUE(errno != 0);
    EXPECT_TRUE(str == nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_GetParent_0002";
}

/**
* @tc.name: RustTest_GetParent_0003
* @tc.desc: Test function of GetParent() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNL
*/
HWTEST_F(RustTest, RustTest_GetParent_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_GetParent_0003";

    Str *str = GetParent(34);
    ASSERT_TRUE(errno == 2);
    EXPECT_TRUE(str == nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_GetParent_0003";
}

/**
* @tc.name: RustTest_StrFree_0001
* @tc.desc: Test function of StrFree() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_StrFree_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_StrFree_0001";
    
    const char fileStr[] = "/data/test/rust_test.txt";
    int fd = open(fileStr, O_RDWR);
    Str *str1 = GetParent(fd);
    int result = strcmp(str1->str, "/data/test");
    EXPECT_TRUE(result == 0);
    StrFree(str1);
    close(fd);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_StrFree_0001";
}

/**
* @tc.name: RustTest_StrFree_0002
* @tc.desc: Test function of StrFree() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_StrFree_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_StrFree_0002";

    Str *str = nullptr;
    StrFree(str);
    EXPECT_TRUE(str == nullptr);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_StrFree_0002";
}

/**
* @tc.name: RustTest_CutFileName_0000
* @tc.desc: Test function of CutFileName() interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000IGDNF
*/
HWTEST_F(RustTest, RustTest_CutFileName_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RustTest-begin RustTest_CutFileName_0000";
    string myStr = "<?,你好，世界！hello, world! ?XML, 你好，世界！";

    Str *str1 = CutFileName(myStr.c_str(), 2);
    GTEST_LOG_(INFO) << "RustTest_CutFileName_0000 str1" << str1->str;
    GTEST_LOG_(INFO) << "RustTest_CutFileName_0000 str1" << str1->len;
    EXPECT_TRUE(string(str1->str).compare("<?,你好，世界！hello, world! ?XML, 你好，世") == 0);
    StrFree(str1);
    Str *str2 = CutFileName(myStr.c_str(), 10);
    GTEST_LOG_(INFO) << "RustTest_CutFileName_0000 str2" << str2->str;
    GTEST_LOG_(INFO) << "RustTest_CutFileName_0000 str2" << str2->len;
    EXPECT_TRUE(string(str2->str).compare("<?,你好，世界！hello, world! ?X") == 0);
    StrFree(str2);

    GTEST_LOG_(INFO) << "RustTest-end RustTest_CutFileName_0000";
}

}
}
}