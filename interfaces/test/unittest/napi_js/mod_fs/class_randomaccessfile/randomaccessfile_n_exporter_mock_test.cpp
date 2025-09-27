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

#include "randomaccessfile_n_exporter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "randomaccessfile_entity.h"
#include "uv_fs_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class RandomAccessFileNExporterMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    unique_ptr<RandomAccessFileEntity> rafEntity;
};

void RandomAccessFileNExporterMockTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    prctl(PR_SET_NAME, "RandomAccessFileNExporterMockTest");
    UvFsMock::EnableMock();
    LibnMock::EnableMock();
}

void RandomAccessFileNExporterMockTest::TearDownTestCase(void)
{
    UvFsMock::DisableMock();
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RandomAccessFileNExporterMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    rafEntity = make_unique<RandomAccessFileEntity>();
    const int fdValue = 3;
    const bool isClosed = false;
    rafEntity->fd = make_unique<DistributedFS::FDGuard>(fdValue, isClosed);
    rafEntity->filePointer = 0;
}

void RandomAccessFileNExporterMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: RandomAccessFileNExporterMockTest_CloseSync_001
 * @tc.desc: Test function of RandomAccessFileNExporter::CloseSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1

*/
HWTEST_F(RandomAccessFileNExporterMockTest, RandomAccessFileNExporterMockTest_CloseSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-begin RandomAccessFileNExporterMockTest_CloseSync_001";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    auto uvMock = UvFsMock::GetMock();
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv)).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*uvMock, uv_fs_close(_, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*libnMock, napi_remove_wrap(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(rafEntity.get())), Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(_)).WillOnce(Return(mockNval));

    auto res = RandomAccessFileNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(uvMock.get());
    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "RandomAccessFileNExporterMockTest-end RandomAccessFileNExporterMockTest_CloseSync_001";
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test