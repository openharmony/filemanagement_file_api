/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "listfile.h"

#include <fcntl.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <securec.h>
#include <sys/prctl.h>

#include "file_filter_mock.h"
#include "libn_mock.h"
#include "ut_file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace std;
using namespace std::filesystem;
using namespace OHOS::FileManagement::ModuleFileIO;

class ListFileMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();

private:
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    const string testDir = FileUtils::testRootDir + "/ListFileMockTest";
    const string dataDir = testDir + "/data";
    const string emptyDir = testDir + "/emptyDir";
    const size_t totalRecursiveFiles = 12;
};

void ListFileMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "ListFileMockTest");
}

void ListFileMockTest::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void ListFileMockTest::SetUp()
{
    LibnMock::EnableMock();
    GTEST_LOG_(INFO) << "SetUp";

    constexpr int32_t dataFileSize = 512;
    constexpr int32_t imageFileSize = 128;
    constexpr int32_t photoFileSize = 1024;

    string dirLevel1 = dataDir + "/level1";
    string dirLevel2 = dirLevel1 + "/level2";
    ASSERT_TRUE(FileUtils::CreateDirectories(testDir, true));
    ASSERT_TRUE(FileUtils::CreateDirectories(dirLevel1));
    ASSERT_TRUE(FileUtils::CreateDirectories(dirLevel2));
    ASSERT_TRUE(FileUtils::CreateDirectories(emptyDir));
    const std::vector<fs::path> directories = { dataDir, dirLevel1, dirLevel2 };
    for (auto i = 0; i < directories.size(); i++) {
        auto txtFile = directories[i] / ("text_" + std::to_string(i + 1) + ".txt");
        auto imageFile = directories[i] / ("image_" + std::to_string(i + 1) + ".png");
        auto photoFile = directories[i] / ("photo_" + std::to_string(i + 1) + ".jpg");
        auto dataFile = directories[i] / ("data_" + std::to_string(i + 1) + ".data");
        ASSERT_TRUE(FileUtils::CreateFile(txtFile, "content"));
        ASSERT_TRUE(FileUtils::CreateFile(imageFile, imageFileSize));
        ASSERT_TRUE(FileUtils::CreateFile(photoFile, photoFileSize));
        ASSERT_TRUE(FileUtils::CreateFile(dataFile, dataFileSize));
    }
}

void ListFileMockTest::TearDown()
{
    ASSERT_TRUE(FileUtils::RemoveAll(testDir));
    LibnMock::DisableMock();
    FileFilterMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDown";
}

static tuple<bool, unique_ptr<char[]>, size_t> BuildTuplePath(const string &path)
{
    auto len = path.length();
    auto ptr = make_unique<char[]>(len + 1);
    auto ret = strncpy_s(ptr.get(), len + 1, path.c_str(), len);
    if (ret != EOK) {
        GTEST_LOG_(ERROR) << "strncpy_s failed with error code: " << ret;
        return { false, nullptr, 0 };
    }
    return { true, move(ptr), 1 };
}

/**
 * @tc.name: ListFileMockTest_001
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching all files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_001";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);

    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_001";
}

/**
 * @tc.name: ListFileMockTest_002
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching all files with listNum limit
 * in non-recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_002";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    constexpr int64_t listNum = 2;

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "listNum" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToInt64(testing::_)).WillOnce(testing::Return(std::make_tuple(true, listNum)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), static_cast<size_t>(listNum));

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_002";
}

/**
 * @tc.name: ListFileMockTest_003
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching all files in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_003";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_003";
}

/**
 * @tc.name: ListFileMockTest_004
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching all files with listNum
 * limit in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_004";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    constexpr int64_t listNum = 5;

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "listNum" || prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToInt64(testing::_)).WillOnce(testing::Return(std::make_tuple(true, listNum)));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), static_cast<size_t>(listNum));

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_004";
}

/**
 * @tc.name: ListFileMockTest_005
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching specific file in
 * non-recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_005";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Invoke([](const std::string &name) {
        return name == "text_1.txt";
    }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    ASSERT_EQ(actualFiles.size(), 1);
    EXPECT_EQ(actualFiles[0], "text_1.txt");

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_005";
}

/**
 * @tc.name: ListFileMockTest_006
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching files starting with / in
 * recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_006";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Invoke([](const std::string &name) {
        return name.find("/") == 0;
    }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_006";
}

/**
 * @tc.name: ListFileMockTest_007
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching files starting with
 * /level1 in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_007";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Invoke([](const std::string &name) {
        return name.find("/level1") == 0;
    }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    std::vector<std::string> expected = { "/level1/data_2.data", "/level1/image_2.png", "/level1/level2/data_3.data",
        "/level1/level2/image_3.png", "/level1/level2/photo_3.jpg", "/level1/level2/text_3.txt", "/level1/photo_2.jpg",
        "/level1/text_2.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_007";
}

/**
 * @tc.name: ListFileMockTest_008
 * @tc.desc: Test function of ListFile::Sync interface for FAILURE when listNum is negative with fileFilter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_008";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    constexpr int64_t listNum = -1;

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "listNum" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToInt64(testing::_)).WillOnce(testing::Return(std::make_tuple(true, listNum)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_008";
}

/**
 * @tc.name: ListFileMockTest_009
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS when fileFilter matches no files.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_009";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(false));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), 0);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_009";
}

/**
 * @tc.name: ListFileMockTest_010
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS when listNum exceeds actual file count in
 * recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_010";

    constexpr int64_t listNum = 20;

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "listNum" || prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToInt64(testing::_)).WillOnce(testing::Return(std::make_tuple(true, listNum)));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_010";
}

/**
 * @tc.name: ListFileMockTest_011
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with fileFilter matching specific file in
 * subdirectory in recursive mode.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_011";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Invoke([](const std::string &name) {
        return name == "/level1/text_2.txt";
    }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);
    ASSERT_EQ(actualFiles.size(), 1);
    EXPECT_EQ(actualFiles[0], "/level1/text_2.txt");

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_011";
}

/**
 * @tc.name: ListFileMockTest_012
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS when no filter and no fileFilter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_012";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillOnce(testing::Return(NARG_CNT::ONE));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, expectedRes);

    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_012";
}

/**
 * @tc.name: ListFileMockTest_013
 * @tc.desc: Test function of ListFile::Sync interface for SUCCESS with suffix filter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_013";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "filter" || prop == "suffix";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToStringArray())
        .WillOnce(testing::Return(std::make_tuple(true, std::vector<std::string> { ".txt" }, 1)));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, expectedRes);

    std::vector<std::string> expected = { "level1", "text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_013";
}

/**
 * @tc.name: ListFileMockTest_014
 * @tc.desc: Test function of GetFileFilterFunction when filter is not a function.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_014, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_014";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_014";
}

/**
 * @tc.name: ListFileMockTest_015
 * @tc.desc: Test function of GetOptionParam when neither filter nor fileFilter exists.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_015, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_015";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(NVal(env, nullptr)));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, expectedRes);
    EXPECT_EQ(actualFiles.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_015";
}

/**
 * @tc.name: ListFileMockTest_016
 * @tc.desc: Test function of GetOptionParam when fileFilter property is undefined.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_016, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_016";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, expectedRes);

    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_016";
}

/**
 * @tc.name: ListFileMockTest_017
 * @tc.desc: Test function of ListFile::Sync when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_017, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_017";

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_017";
}

/**
 * @tc.name: ListFileMockTest_018
 * @tc.desc: Test function of ListFile::Sync with fileFilter and recursion, filtering .txt files only.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_018, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_018";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Invoke([](const std::string &name) {
        return name.find(".txt") != std::string::npos;
    }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Sync(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_EQ(result, expectedRes);

    std::vector<std::string> expected = { "/level1/level2/text_3.txt", "/level1/text_2.txt", "/text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_018";
}

/**
 * @tc.name: ListFileMockTest_Async_001
 * @tc.desc: Test function of ListFile::Async interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_001";

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_001";
}

/**
 * @tc.name: ListFileMockTest_Async_002
 * @tc.desc: Test function of ListFile::Async interface for FAILURE when path parsing fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_002";

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(std::make_tuple(false, nullptr, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_002";
}

/**
 * @tc.name: ListFileMockTest_Async_003
 * @tc.desc: Test function of ListFile::Async interface for FAILURE when GetOptionArg fails with negative listNum.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_003";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "listNum";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(NVal(env, nullptr)));
    EXPECT_CALL(*libnMock, ToInt64(testing::_)).WillOnce(testing::Return(std::make_tuple(true, int64_t(-1))));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).WillOnce(testing::Return());

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_003";
}

/**
 * @tc.name: ListFileMockTest_Async_004
 * @tc.desc: Test function of ListFile::Async interface for SUCCESS with Promise path (argc=1).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_004";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::ONE));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, GetThisVar()).WillRepeatedly(testing::Return(napiValue));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(result, nullptr);

    std::vector<std::string> expected = { "data_1.data", "image_1.png", "level1", "photo_1.jpg", "text_1.txt" };
    std::sort(actualFiles.begin(), actualFiles.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(actualFiles, expected);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_004";
}

/**
 * @tc.name: ListFileMockTest_Async_005
 * @tc.desc: Test function of ListFile::Async interface for SUCCESS with Promise path (argc=2, second arg not function).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_005";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, GetThisVar()).WillRepeatedly(testing::Return(napiValue));
    {
        testing::InSequence seq;
        EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillOnce(testing::Return(false));
        EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillOnce(testing::Return(true));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    }

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_005";
}

/**
 * @tc.name: ListFileMockTest_Async_006
 * @tc.desc: Test function of ListFile::Async interface for SUCCESS with Callback path (argc=2, second arg is function).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_006";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, GetThisVar()).WillRepeatedly(testing::Return(napiValue));
    {
        testing::InSequence seq;
        EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillOnce(testing::Return(false));
        EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    }

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_006";
}

/**
 * @tc.name: ListFileMockTest_Async_007
 * @tc.desc: Test function of ListFile::Async interface for SUCCESS with Callback path (argc=3) in recursive mode
 * with fileFilter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_007";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::THREE));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, GetThisVar()).WillRepeatedly(testing::Return(napiValue));
    {
        testing::InSequence seq;
        EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillOnce(testing::Return(true));
        EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
    }

    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "recursion" || prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));
    EXPECT_CALL(*libnMock, ToBool(testing::_)).WillOnce(testing::Return(std::make_tuple(true, true)));

    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));

    EXPECT_CALL(*libnMock, napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](napi_env, napi_value, uint32_t, napi_ref *result) {
            *result = reinterpret_cast<napi_ref>(0x2000);
            return napi_ok;
        }));

    std::vector<std::string> actualFiles;
    napi_value expectedRes = reinterpret_cast<napi_value>(0x2000);
    EXPECT_CALL(*libnMock, CreateArrayString(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&actualFiles), testing::Return(NVal(env, expectedRes))));

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(actualFiles.size(), totalRecursiveFiles);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_007";
}

/**
 * @tc.name: ListFileMockTest_Async_008
 * @tc.desc: Test function of ListFile::Async cbCompl for HasException path with fileFilter.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(ListFileMockTest, ListFileMockTest_Async_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ListFileMockTest-begin ListFileMockTest_Async_008";

    auto tuplePath = BuildTuplePath(dataDir);
    ASSERT_TRUE(std::get<0>(tuplePath));

    napi_value napiValue = reinterpret_cast<napi_value>(0x1000);
    NVal nval(env, nullptr);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>(), testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(testing::ByMove(std::move(tuplePath))));
    EXPECT_CALL(*libnMock, GetArgc()).WillRepeatedly(testing::Return(NARG_CNT::TWO));
    EXPECT_CALL(*libnMock, GetArg(testing::_)).WillRepeatedly(testing::Return(napiValue));
    EXPECT_CALL(*libnMock, GetThisVar()).WillRepeatedly(testing::Return(napiValue));
    {
        testing::InSequence seq;
        EXPECT_CALL(*libnMock, TypeIs(napi_object)).WillOnce(testing::Return(true));
        EXPECT_CALL(*libnMock, TypeIs(napi_undefined)).WillOnce(testing::Return(false));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(true));
        EXPECT_CALL(*libnMock, TypeIs(napi_function)).WillOnce(testing::Return(false));
    }
    EXPECT_CALL(*libnMock, HasProp(testing::_)).WillRepeatedly(testing::Invoke([](std::string prop) -> bool {
        return prop == "fileFilter";
    }));
    EXPECT_CALL(*libnMock, GetProp(testing::_)).WillRepeatedly(testing::Return(nval));

    auto filterMock = FileFilterMock::GetMock();
    EXPECT_CALL(*filterMock, Filter(testing::_)).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*filterMock, HasException()).WillRepeatedly(testing::Return(true));
    napi_value exceptionResult = reinterpret_cast<napi_value>(0x3000);
    EXPECT_CALL(*filterMock, HandleException(testing::_)).WillRepeatedly(testing::Return(exceptionResult));

    auto result = ListFile::Async(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(filterMock.get());
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "ListFileMockTest-end ListFileMockTest_Async_008";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
