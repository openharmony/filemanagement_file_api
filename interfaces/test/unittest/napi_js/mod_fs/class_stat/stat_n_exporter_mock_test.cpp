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

#include "stat_n_exporter.h"

#include <gtest/gtest.h>
#include <sys/prctl.h>

#include "libn_mock.h"
#include "securec.h"
#include "stat_entity.h"
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class StatNExporter : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp();
    void TearDown();
};

void StatNExporter::SetUpTestSuite()
{
    GTEST_LOG_(INFO) << "SetUpTestSuite";
}

void StatNExporter::TearDownTestSuite()
{
    GTEST_LOG_(INFO) << "TearDownTestSuite";
}

void StatNExporter::SetUp()
{
    GTEST_LOG_(INFO) << "SetUp";
}

void StatNExporter::TearDown()
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StatNExporter_IsBlockDevice_001
 * @tc.desc: Test function of StatNExporter::IsBlockDevice interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsBlockDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsBlockDevice_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFBLK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsBlockDevice(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsBlockDevice_001";
}

/**
 * @tc.name: StatNExporter_IsBlockDevice_002
 * @tc.desc: Test function of StatNExporter::IsBlockDevice interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsBlockDevice_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsBlockDevice_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFCHR;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsBlockDevice(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsBlockDevice_002";
}

/**
 * @tc.name: StatNExporter_IsCharacterDevice_001
 * @tc.desc: Test function of StatNExporter::IsCharacterDevice interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsCharacterDevice_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsCharacterDevice_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFCHR;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsCharacterDevice(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsCharacterDevice_001";
}

/**
 * @tc.name: StatNExporter_IsCharacterDevice_002
 * @tc.desc: Test function of StatNExporter::IsCharacterDevice interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsCharacterDevice_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsCharacterDevice_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFDIR;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsCharacterDevice(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsCharacterDevice_002";
}

/**
 * @tc.name: StatNExporter_IsDirectory_001
 * @tc.desc: Test function of StatNExporter::IsDirectory interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsDirectory_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsDirectory_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFDIR;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsDirectory(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsDirectory_001";
}

/**
 * @tc.name: StatNExporter_IsDirectory_002
 * @tc.desc: Test function of StatNExporter::IsDirectory interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsDirectory_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsDirectory_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFIFO;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsDirectory(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsDirectory_002";
}

/**
 * @tc.name: StatNExporter_IsFIFO_001
 * @tc.desc: Test function of StatNExporter::IsFIFO interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsFIFO_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsFIFO_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFIFO;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsFIFO(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsFIFO_001";
}

/**
 * @tc.name: StatNExporter_IsFIFO_002
 * @tc.desc: Test function of StatNExporter::IsFIFO interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsFIFO_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsFIFO_002";
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFREG;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsFIFO(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsFIFO_002";
}

/**
 * @tc.name: StatNExporter_IsFile_001
 * @tc.desc: Test function of StatNExporter::IsFile interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsFile_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsFile_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFREG;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsFile(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsFile_001";
}

/**
 * @tc.name: StatNExporter_IsFile_002
 * @tc.desc: Test function of StatNExporter::IsFile interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsFile_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsFile_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFSOCK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsFile(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsFile_002";
}

/**
 * @tc.name: StatNExporter_IsSocket_001
 * @tc.desc: Test function of StatNExporter::IsSocket interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsSocket_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsSocket_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFSOCK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsSocket(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsSocket_001";
}

/**
 * @tc.name: StatNExporter_IsSocket_002
 * @tc.desc: Test function of StatNExporter::IsSocket interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsSocket_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsSocket_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFLNK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsSocket(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsSocket_002";
}

/**
 * @tc.name: StatNExporter_IsSymbolicLink_001
 * @tc.desc: Test function of StatNExporter::IsSymbolicLink interface for SUCCESS.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsSymbolicLink_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsSymbolicLink_001";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFLNK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsSymbolicLink(env, info);

    ASSERT_TRUE(result);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsSymbolicLink_001";
}

/**
 * @tc.name: StatNExporter_IsSymbolicLink_002
 * @tc.desc: Test function of StatNExporter::IsSymbolicLink interface for FAILURE.
 * @tc.size: SMALL
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(StatNExporter, StatNExporter_IsSymbolicLink_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StatNExporter-begin StatNExporter_IsSymbolicLink_002";

    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_callback_info info = reinterpret_cast<napi_callback_info>(0x1000);
    StatEntity entity;
    entity.stat_.st_mode = S_IFSOCK;

    auto libnMock = LibnMock::GetMock();
    EXPECT_CALL(*libnMock, napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(static_cast<void *>(&entity)), testing::Return(napi_ok)));

    bool result = ModuleFileIO::StatNExporter::IsSymbolicLink(env, info);

    ASSERT_NE(result, false);

    GTEST_LOG_(INFO) << "StatNExporter-end StatNExporter_IsSymbolicLink_002";
}
} // namespace OHOS::FileManagement::ModuleFileIO::Test