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

#include "trans_listener_core.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "copy_core.h"
#include "dfs_mock.h"
#include "unistd_mock.h"

using namespace OHOS;
using namespace OHOS::Storage::DistributedFile;

class MockTaskSignalListener : public OHOS::DistributedFS::ModuleTaskSignal::TaskSignalListener {
public:
    MOCK_METHOD(void, OnCancel, (), (override));
};

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static const string FILE_MANAGER_AUTHORITY = "docs";
static const string MEDIA_AUTHORITY = "media";

class IProgressListenerTest : public IProgressListener {
public:
    void InvokeListener(uint64_t progressSize, uint64_t totalSize) const override {}
};

class TransListenerCoreMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void TransListenerCoreMockTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "TransListenerCoreMockTest");
    UnistdMock::EnableMock();
    DfsMock::EnableMock();
}

void TransListenerCoreMockTest::TearDownTestCase()
{
    DfsMock::DisableMock();
    UnistdMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void TransListenerCoreMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void TransListenerCoreMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: TransListenerCoreMockTest_PrepareCopySession_001
 * @tc.desc: Test function of TransListenerCore::PrepareCopySession interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreMockTest, TransListenerCoreMockTest_PrepareCopySession_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-begin TransListenerCoreMockTest_PrepareCopySession_001";

    Storage::DistributedFile::HmdfsInfo info;
    info.authority = FILE_MANAGER_AUTHORITY;
    string srcUri = "TransListenerCoreMockTest_PrepareCopySession_001_src";
    string destUri = "TransListenerCoreMockTest_PrepareCopySession_001_dest";

    string disSandboxPath = "disSandboxPath";
    auto dfsMock = DfsMock::GetMock();
    EXPECT_CALL(*dfsMock, PrepareSession(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(ERRNO_NOERR));

    auto result = TransListenerCore::PrepareCopySession(srcUri, destUri, nullptr, info, disSandboxPath);

    testing::Mock::VerifyAndClearExpectations(dfsMock.get());
    EXPECT_EQ(result, ERRNO_NOERR);

    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-end TransListenerCoreMockTest_PrepareCopySession_001";
}

/**
 * @tc.name: TransListenerCoreMockTest_CopyFileFromSoftBus_001
 * @tc.desc: Test function of TransListenerCore::CopyFileFromSoftBus interface for FAILURE when PrepareSession fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(TransListenerCoreMockTest, TransListenerCoreMockTest_CopyFileFromSoftBus_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-begin TransListenerCoreMockTest_CopyFileFromSoftBus_001";

    string srcUri = "TransListenerCoreMockTest_CopyFileFromSoftBus_001";
    string destUri = "http://media?networkid=AD125AD1CF";
    std::shared_ptr<TransListenerCore> transListener = std::make_shared<TransListenerCore>();
    std::shared_ptr<FsFileInfos> infos = std::make_shared<FsFileInfos>();

    auto unistdMock = UnistdMock::GetMock();
    auto dfsMock = DfsMock::GetMock();
    EXPECT_CALL(*unistdMock, read(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*dfsMock, PrepareSession(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(EIO));

    auto res = transListener->CopyFileFromSoftBus(srcUri, destUri, infos, nullptr);

    testing::Mock::VerifyAndClearExpectations(unistdMock.get());
    testing::Mock::VerifyAndClearExpectations(dfsMock.get());
    EXPECT_EQ(res, EIO);

    GTEST_LOG_(INFO) << "TransListenerCoreMockTest-end TransListenerCoreMockTest_CopyFileFromSoftBus_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
