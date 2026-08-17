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

#include "readeriterator_n_exporter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libn_mock.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
class ReaderIteratorNExporterMockTest : public testing::Test {
public:
    void SetUp() override
    {
        LibnMock::EnableMock();
    }

    void TearDown() override
    {
        LibnMock::GetMock()->ResetErrState();
        LibnMock::DisableMock();
    }
};

/**
 * @tc.name: ReaderIteratorNExporterMockTest_Constructor_001
 * @tc.desc: Verify Constructor rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ReaderIteratorNExporterMockTest, ReaderIteratorNExporterMockTest_Constructor_001,
    testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(ReaderIteratorNExporter::Constructor(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ReaderIteratorNExporterMockTest_Next_001
 * @tc.desc: Verify Next rejects an invalid argument count.
 * @tc.type: FUNC
 */
HWTEST_F(ReaderIteratorNExporterMockTest, ReaderIteratorNExporterMockTest_Next_001,
    testing::ext::TestSize.Level1)
{
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(ReaderIteratorNExporter::Next(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900020, "Invalid argument");
}

/**
 * @tc.name: ReaderIteratorNExporterMockTest_Next_002
 * @tc.desc: Verify Next reports an error when the receiver has no entity.
 * @tc.type: FUNC
 */
HWTEST_F(ReaderIteratorNExporterMockTest, ReaderIteratorNExporterMockTest_Next_002,
    testing::ext::TestSize.Level1)
{
    napi_value thisVar = reinterpret_cast<napi_value>(0x1200);
    auto mock = LibnMock::GetMock();
    EXPECT_CALL(*mock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*mock, GetThisVar()).WillOnce(testing::Return(thisVar));
    EXPECT_CALL(*mock, napi_unwrap(testing::_, thisVar, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(nullptr), testing::Return(napi_ok)));
    EXPECT_CALL(*mock, ThrowErr(testing::_)).Times(1);

    EXPECT_EQ(ReaderIteratorNExporter::Next(reinterpret_cast<napi_env>(0x1000),
        reinterpret_cast<napi_callback_info>(0x1000)), nullptr);
    mock->VerifyAndClearErr(13900001, "Operation not permitted");
}

/**
 * @tc.name: ReaderIteratorNExporterMockTest_GetClassName_001
 * @tc.desc: Verify GetClassName returns the registered class name.
 * @tc.type: FUNC
 */
HWTEST_F(ReaderIteratorNExporterMockTest, ReaderIteratorNExporterMockTest_GetClassName_001,
    testing::ext::TestSize.Level1)
{
    ReaderIteratorNExporter exporter(nullptr, nullptr);
    EXPECT_EQ(exporter.GetClassName(), "ReaderIterator");
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test
