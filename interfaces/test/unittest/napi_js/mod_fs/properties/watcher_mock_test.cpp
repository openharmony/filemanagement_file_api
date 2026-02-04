/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "watcher.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "eventfd_mock.h"
#include "inotify_mock.h"
#include "libn_mock.h"
#include "securec.h"
#include "watcher_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class WatcherMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void WatcherMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "WatcherMockTest");
    LibnMock::EnableMock();
    EventfdMock::EnableMock();
    InotifyMock::EnableMock();
}

void WatcherMockTest::TearDownTestSuite()
{
    LibnMock::DisableMock();
    EventfdMock::DisableMock();
    InotifyMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void WatcherMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
    // Reset FileWatcher state
    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.reading_ = false;
    watcher.closed_ = false;
    watcher.notifyFd_ = -1;
    watcher.eventFd_ = -1;
    watcher.watcherInfoSet_.clear();
    watcher.wdFileNameMap_.clear();
}

void WatcherMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
    // Reset FileWatcher state
    auto &watcher = FileWatcher::GetInstance();
    watcher.run_ = false;
    watcher.reading_ = false;
    watcher.closed_ = false;
    watcher.notifyFd_ = -1;
    watcher.eventFd_ = -1;
    watcher.watcherInfoSet_.clear();
    watcher.wdFileNameMap_.clear();
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_001
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_001";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_002
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when ToUTF8StringPath fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_002";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { false, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_002";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_003
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when ToInt32 fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_003";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_003";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(false, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_003";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_004
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when event is <= 0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_004";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_004";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, 0)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_004";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_005
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when event is negative.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_005";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_005";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, -1)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_005";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_006
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when TypeIs fails (not a function).
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_006, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_006";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_006";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CREATE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_006";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_007
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when InstantiateClass fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_007, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_007";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_007";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CREATE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nullptr));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_007";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_008
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when napi_unwrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_008, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_008";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_008";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CREATE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nVal));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_)).WillOnce(testing::Return(napi_invalid_arg));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_008";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_009
 * @tc.desc: Test function of Watcher::CreateWatcher interface for SUCCESS with valid parameters.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_009, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_009";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_009";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    WatcherEntity entity;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CREATE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nVal));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void*>(&entity)), testing::Return(napi_ok)));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nVal);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_009";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_010
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when GetNotifyId < 0 and InitNotify fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_010, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_010";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_010";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CREATE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*inotifyMock, inotify_init()).WillOnce(testing::Return(-1));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_010";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_011
 * @tc.desc: Test function of Watcher::CreateWatcher interface for FAILURE when event is invalid.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_011, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_011";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_011";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    // Use invalid event that will fail CheckEventValid (event not in IN_ALL_EVENTS)
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, ~IN_ALL_EVENTS)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_011";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_012
 * @tc.desc: Test function of Watcher::CreateWatcher interface for SUCCESS with IN_ALL_EVENTS event.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_012, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_012";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_012";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    WatcherEntity entity;

    auto libnMock = LibnMock::GetMock();
    auto inotifyMock = InotifyMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_ALL_EVENTS)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nVal));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void*>(&entity)), testing::Return(napi_ok)));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    testing::Mock::VerifyAndClearExpectations(inotifyMock.get());
    EXPECT_EQ(res, nVal);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_012";
}

/**
 * @tc.name: WatcherMockTest_CreateWatcher_013
 * @tc.desc: Test function of Watcher::CreateWatcher interface for SUCCESS with IN_CLOSE event.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(WatcherMockTest, WatcherMockTest_CreateWatcher_013, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WatcherMockTest-begin WatcherMockTest_CreateWatcher_013";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value nVal = reinterpret_cast<napi_value>(0x1300);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);

    const char *initSrc = "WatcherMockTest_CreateWatcher_013";
    size_t strLen = strlen(initSrc) + 1;
    auto srcPtr = std::make_unique<char[]>(strLen);
    ASSERT_NE(srcPtr, nullptr);
    auto ret = strncpy_s(srcPtr.get(), strLen, initSrc, strLen - 1);
    ASSERT_EQ(ret, EOK);

    std::tuple<bool, std::unique_ptr<char[]>, size_t> srcRes = { true, move(srcPtr), 1 };

    WatcherEntity entity;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, ToUTF8StringPath()).WillOnce(testing::Return(move(srcRes)));
    EXPECT_CALL(*libnMock, ToInt32()).WillOnce(testing::Return(std::make_tuple(true, IN_CLOSE)));
    EXPECT_CALL(*libnMock, TypeIs(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, InstantiateClass(testing::_, testing::_, testing::_)).WillOnce(testing::Return(nVal));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void*>(&entity)), testing::Return(napi_ok)));

    auto res = Watcher::CreateWatcher(env, info);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nVal);

    GTEST_LOG_(INFO) << "WatcherMockTest-end WatcherMockTest_CreateWatcher_013";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
