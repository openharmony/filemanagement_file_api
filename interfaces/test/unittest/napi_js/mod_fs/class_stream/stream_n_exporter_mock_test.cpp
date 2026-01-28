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

#include "stream_n_exporter.h"

#include <cstdio>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "common_func.h"
#include "libn_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace testing;
using namespace std;

class StreamNExporterMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void StreamNExporterMockTest::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
    prctl(PR_SET_NAME, "StreamNExporterMockTest");
    LibnMock::EnableMock();
}

void StreamNExporterMockTest::TearDownTestSuite()
{
    LibnMock::DisableMock();
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void StreamNExporterMockTest::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StreamNExporterMockTest::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StreamNExporterMockTest_Constructor_001
 * @tc.desc: Test function of StreamNExporter::Constructor interface for FAILURE
 * when InitArgs fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_Constructor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_Constructor_001";
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(false));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = StreamNExporter::Constructor(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_Constructor_001";
}

/**
 * @tc.name: StreamNExporterMockTest_CloseSync_001
 * @tc.desc: Test function of StreamNExporter::CloseSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_CloseSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_CloseSync_001";
    
    auto streamEntity = make_unique<StreamEntity>();
    auto mockFile = std::shared_ptr<FILE>(
        reinterpret_cast<FILE *>(0x2000),
        [](FILE *) {}
    );
    streamEntity->fp = mockFile;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).Times(2).WillRepeatedly(testing::Return(nv));
    
    // Mock GetEntityOf to return stream entity
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_ok)));
    
    // Mock RemoveEntityOfFinal to return non-null
    EXPECT_CALL(*libnMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_ok)));
    
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = StreamNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_CloseSync_001";
}

/**
 * @tc.name: StreamNExporterMockTest_CloseSync_002
 * @tc.desc: Test function of StreamNExporter::CloseSync interface for FAILURE
 * when GetEntityOf returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_CloseSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_CloseSync_002";
    
    auto streamEntity = make_unique<StreamEntity>();
    auto mockFile = std::shared_ptr<FILE>(
        reinterpret_cast<FILE *>(0x2000),
        [](FILE *) {}
    );
    streamEntity->fp = mockFile;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    
    // Mock GetEntityOf to return nullptr (stream already closed)
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), testing::Return(napi_ok)));
    
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = StreamNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_CloseSync_002";
}

/**
 * @tc.name: StreamNExporterMockTest_CloseSync_003
 * @tc.desc: Test function of StreamNExporter::CloseSync interface for FAILURE
 * when RemoveEntityOfFinal returns nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_CloseSync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_CloseSync_003";
    
    auto streamEntity = make_unique<StreamEntity>();
    auto mockFile = std::shared_ptr<FILE>(
        reinterpret_cast<FILE *>(0x2000),
        [](FILE *) {}
    );
    streamEntity->fp = mockFile;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar())
        .Times(2).WillRepeatedly(Return(nv));
    
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_ok)));
    
    // Mock RemoveEntityOfFinal to return nullptr
    EXPECT_CALL(*libnMock, napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), testing::Return(napi_ok)));

    auto res = StreamNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_CloseSync_003";
}

/**
 * @tc.name: StreamNExporterMockTest_CloseSync_004
 * @tc.desc: Test function of StreamNExporter::CloseSync interface for FAILURE
 * when argument count is wrong.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_CloseSync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_CloseSync_004";
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>()))
        .WillOnce(testing::Return(false));

    EXPECT_CALL(*libnMock, ThrowErr(testing::_))
        .Times(1);

    auto res = StreamNExporter::CloseSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_CloseSync_004";
}

/**
 * @tc.name: StreamNExporterMockTest_FlushSync_001
 * @tc.desc: Test function of StreamNExporter::FlushSync interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_FlushSync_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_FlushSync_001";

    auto streamEntity = make_unique<StreamEntity>();
    FILE *filePtr = tmpfile();
    ASSERT_NE(filePtr, nullptr);
    auto mockFile = std::shared_ptr<FILE>(
        filePtr,
        [](FILE *fp) {
            if (fp) {
                fclose(fp);
            }
        }
    );
    streamEntity->fp = mockFile;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);
    NVal mockNval = { env, nv };

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, CreateUndefined(testing::_)).WillOnce(testing::Return(mockNval));

    auto res = StreamNExporter::FlushSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_NE(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_FlushSync_001";
}

/**
 * @tc.name: StreamNExporterMockTest_FlushSync_002
 * @tc.desc: Test function of StreamNExporter::FlushSync interface for FAILURE
 * when GetEntityOf fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_FlushSync_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_FlushSync_002";
    
    auto streamEntity = make_unique<StreamEntity>();
    FILE *filePtr = tmpfile();
    ASSERT_NE(filePtr, nullptr);
    auto mockFile = std::shared_ptr<FILE>(
        filePtr,
        [](FILE *fp) {
            if (fp) {
                fclose(fp);
            }
        }
    );
    streamEntity->fp = mockFile;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_invalid_arg)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_));

    auto res = StreamNExporter::FlushSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_FlushSync_002";
}

/**
 * @tc.name: StreamNExporterMockTest_FlushSync_003
 * @tc.desc: Test function of StreamNExporter::FlushSync interface for FAILURE
 * when file pointer is null.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_FlushSync_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_FlushSync_003";
    
    auto streamEntity = make_unique<StreamEntity>();
    streamEntity->fp = nullptr;
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar()).WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(static_cast<void *>(streamEntity.get())), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_)).Times(1);

    auto res = StreamNExporter::FlushSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_FlushSync_003";
}

/**
 * @tc.name: StreamNExporterMockTest_FlushSync_004
 * @tc.desc: Test function of StreamNExporter::FlushSync interface for FAILURE
 * when argument count is wrong.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_FlushSync_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_FlushSync_004";
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>()))
        .WillOnce(testing::Return(false));

    EXPECT_CALL(*libnMock, ThrowErr(testing::_))
        .Times(1);

    auto res = StreamNExporter::FlushSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_FlushSync_004";
}

/**
 * @tc.name: StreamNExporterMockTest_FlushSync_005
 * @tc.desc: Test function of StreamNExporter::FlushSync interface for FAILURE
 * when stream entity is null.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StreamNExporterMockTest, StreamNExporterMockTest_FlushSync_005, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StreamNExporterMockTest-begin StreamNExporterMockTest_FlushSync_005";
    
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info mInfo = reinterpret_cast<napi_callback_info>(0x1122);
    napi_value nv = reinterpret_cast<napi_value>(0x1200);

    auto libnMock = LibnMock::GetMock();
    
    EXPECT_CALL(*libnMock, InitArgs(testing::A<size_t>())).WillOnce(testing::Return(true));
    EXPECT_CALL(*libnMock, GetThisVar())
        .WillOnce(testing::Return(nv));
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(nullptr), testing::Return(napi_ok)));
    EXPECT_CALL(*libnMock, ThrowErr(testing::_))
        .Times(1);

    auto res = StreamNExporter::FlushSync(env, mInfo);

    testing::Mock::VerifyAndClearExpectations(libnMock.get());
    EXPECT_EQ(res, nullptr);

    GTEST_LOG_(INFO) << "StreamNExporterMockTest-end StreamNExporterMockTest_FlushSync_005";
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS