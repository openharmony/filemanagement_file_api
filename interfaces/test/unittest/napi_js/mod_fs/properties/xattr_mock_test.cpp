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

#include "xattr.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "sys_xattr_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class XattrMockTest : public testing::Test {
public:
    static void SetUpTestSuite(void);
    static void TearDownTestSuite(void);
    void SetUp();
    void TearDown();
};

void XattrMockTest::SetUpTestSuite(void)
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "XattrMockTest");
    SysXattrMock::EnableMock();
    LibnMock::EnableMock();
}

void XattrMockTest::TearDownTestSuite(void)
{
    SysXattrMock::DisableMock();
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void XattrMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void XattrMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_001
 * @tc.desc: Test function of Xattr::SetSync interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_001";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_002
 * @tc.desc: Test function of Xattr::SetSync interface for FAILURE when ToUTF8StringPath is false.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    const char *initStr = "hello world";
    size_t strLen = strlen(initStr) + 1;
    auto strPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPtr, nullptr);
    auto ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfPath = { false, move(strPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(toUtfPath)));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_002";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_003
 * @tc.desc: Test function of Fdatasync::Sync interface for FAILURE when ToUTF8String fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    const char *initStr = "hello world";
    size_t strLen = strlen(initStr) + 1;
    auto strPathPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPathPtr, nullptr);
    auto ret = strncpy_s(strPathPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto strPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPtr, nullptr);
    ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfPath = { true, move(strPathPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfRes = { false, move(strPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArg(_)).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(toUtfPath)));
    EXPECT_CALL(*libnMock, ToUTF8String()).WillOnce(Return(move(toUtfRes)));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_003";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_004
 * @tc.desc: Test function of Fdatasync::Sync interface for FAILURE when ToUTF8StringPath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    const char *initStr = "hello world";
    size_t strLen = strlen(initStr) + 1;

    auto strPathPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPathPtr, nullptr);
    auto ret = strncpy_s(strPathPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto strPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPtr, nullptr);
    ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto valPtr = make_unique<char[]>(strLen);
    ASSERT_NE(valPtr, nullptr);
    ret = strncpy_s(valPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfPath = { true, move(strPathPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfRes = { true, move(strPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toValUtfRes = { false, move(valPtr), 1 };


    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArg(_)).WillOnce(Return(nv)).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(toUtfPath)));
    EXPECT_CALL(*libnMock, ToUTF8String()).WillOnce(Return(move(toUtfRes))).WillOnce(Return(move(toValUtfRes)));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_004";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_005
 * @tc.desc: Test function of Fdatasync::Sync interface for FAILURE when setxattr fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    const char *initStr = "hello world";
    size_t strLen = strlen(initStr) + 1;

    auto strPathPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPathPtr, nullptr);
    auto ret = strncpy_s(strPathPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto strPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPtr, nullptr);
    ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto valPtr = make_unique<char[]>(strLen);
    ASSERT_NE(valPtr, nullptr);
    ret = strncpy_s(valPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfPath = { true, move(strPathPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfRes = { true, move(strPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toValUtfRes = { true, move(valPtr), 1 };


    auto libnMock = LibnMock::GetMock();
    auto xattrMock = SysXattrMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArg(_)).WillOnce(Return(nv)).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(toUtfPath)));
    EXPECT_CALL(*libnMock, ToUTF8String()).WillOnce(Return(move(toUtfRes))).WillOnce(Return(move(toValUtfRes)));
    EXPECT_CALL(*xattrMock, setxattr(_, _, _, _, _)).WillOnce(SetErrnoAndReturn(EIO, -1));
    EXPECT_CALL(*libnMock, ThrowErr(_));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_005";
}

/**
 * @tc.name: XattrMockTest_SetXattr_Sync_006
 * @tc.desc: Test function of Fdatasync::Sync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(XattrMockTest, XattrMockTest_SetXattr_Sync_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrMockTest-begin XattrMockTest_SetXattr_Sync_006";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    NVal mockNval = { env, nVal };

    const char *initStr = "hello world";
    size_t strLen = strlen(initStr) + 1;

    auto strPathPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPathPtr, nullptr);
    auto ret = strncpy_s(strPathPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto strPtr = make_unique<char[]>(strLen);
    ASSERT_NE(strPtr, nullptr);
    ret = strncpy_s(strPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    auto valPtr = make_unique<char[]>(strLen);
    ASSERT_NE(valPtr, nullptr);
    ret = strncpy_s(valPtr.get(), strLen, initStr, strLen - 1);
    ASSERT_EQ(ret, EOK);

    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfPath = { true, move(strPathPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toUtfRes = { true, move(strPtr), 1 };
    tuple<bool, std::unique_ptr<char[]>, size_t> toValUtfRes = { true, move(valPtr), 1 };


    auto libnMock = LibnMock::GetMock();
    auto xattrMock = SysXattrMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetArg(_)).WillOnce(Return(nv)).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(Return(move(toUtfPath)));
    EXPECT_CALL(*libnMock, ToUTF8String()).WillOnce(Return(move(toUtfRes))).WillOnce(Return(move(toValUtfRes)));
    EXPECT_CALL(*xattrMock, setxattr(_, _, _, _, _)).WillOnce(Return(0));
   EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = Xattr::SetSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(xattrMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "XattrMockTest-end XattrMockTest_SetXattr_Sync_006";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test