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
#include "libn_mock.h"
#include "prop_n_exporter_mock.h"
#include "prop_n_exporter.h"
#include "uv_fs_mock.h"
#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class PropNExporterMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<UvfsMock> uvMock = nullptr;
protected:
};

void PropNExporterMockTest::SetUpTestCase(void)
{
    uvMock = make_shared<UvfsMock>();
    PropNExporterMock::EnableMock();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void PropNExporterMockTest::TearDownTestCase(void)
{
    uvMock = nullptr;
    PropNExporterMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void PropNExporterMockTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void PropNExporterMockTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: PropNExporterMockTest_UnlinkSync_001
 * @tc.desc: Test function of UnlinkSync() interface for failed.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 
*/
HWTEST_F(PropNExporterMockTest, PropNExporterMockTest_UnlinkSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PropNExporterMockTest-begin PropNExporterMockTest_UnlinkSync_001";
    int envAddr = 0x1000;
    int valueAddr = 0x1200;
    int callbackInfoAddr = 0x1122;
    napi_env env = reinterpret_cast<napi_env>(envAddr);
    napi_value nv = reinterpret_cast<napi_value>(valueAddr);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(callbackInfoAddr);
    
    size_t strLen = 10;
    auto strPtr = make_unique<char []>(strLen);
    tuple<bool, std::unique_ptr<char[]>, size_t> tp = { true, move(strPtr), strLen };

    auto pMock = LibnMock::GetMock();
    EXPECT_CALL(*pMock, InitArgs(A<size_t>())).WillOnce(Return(true));
    EXPECT_CALL(*pMock, GetArg(_)).WillOnce(Return(nv));
    EXPECT_CALL(*pMock, ToUTF8StringPath()).WillOnce(Return(move(tp)));
    EXPECT_CALL(*uvMock, uv_fs_unlink(_, _, _, _)).WillOnce(Return(-1));
    EXPECT_CALL(*pMock, ThrowErr(_));

    auto res = PropNExporter::UnlinkSync(env, mInfo);
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "PropNExporterMockTest-end PropNExporterMockTest_UnlinkSync_001";
}

}