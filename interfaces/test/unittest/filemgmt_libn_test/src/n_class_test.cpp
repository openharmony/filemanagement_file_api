/*
 * Copyright (C) 2023-2025 Huawei Device Co., Ltd.
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

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "n_class.h"
#include "napi_mock.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
namespace Test {
using namespace std;

class NClassTest : public testing::Test {
public:
    static void SetUpTestSuite(void) {}
    static void TearDownTestSuite(void) {}

    void SetUp() override
    {
        mock_ = std::make_unique<NapiMock>();
        scopedMock_ = std::make_unique<ScopedNapiMock>(mock_.get());
    }

    void TearDown() override
    {
        scopedMock_.reset();
        mock_.reset();
    }

    NapiMock &GetMock()
    {
        return *mock_;
    }

private:
    std::unique_ptr<NapiMock> mock_;
    std::unique_ptr<ScopedNapiMock> scopedMock_;
};

/**
 * @tc.name: NClassTest_DefineClass_001
 * @tc.desc: Test function of NClass::DefineClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_DefineClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_DefineClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_DefineClass_001";
    napi_callback callback = reinterpret_cast<napi_callback>(0x1122);
    vector<napi_property_descriptor> props = {};

    // Set mock behaviors
    napi_value expectedClass = reinterpret_cast<napi_value>(0x1234);
    EXPECT_CALL(GetMock(), napi_define_class(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
                               testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<7>(expectedClass), testing::Return(napi_ok)));

    // Do testing
    auto [succ, classValue] = NClass::DefineClass(env, className, callback, std::move(props));

    // Verify results
    EXPECT_TRUE(succ);
    EXPECT_EQ(classValue, expectedClass);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_DefineClass_001";
}

/**
 * @tc.name: NClassTest_DefineClass_002
 * @tc.desc: Test function of NClass::DefineClass interface for FAILURE when napi_define_class fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_DefineClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_DefineClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_DefineClass_002";
    napi_callback callback = reinterpret_cast<napi_callback>(0x1122);
    vector<napi_property_descriptor> props = {};

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_define_class(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_,
                               testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto [succ, classValue] = NClass::DefineClass(env, className, callback, std::move(props));

    // Verify results
    EXPECT_FALSE(succ);
    EXPECT_EQ(classValue, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_DefineClass_002";
}

/**
 * @tc.name: NClassTest_SaveClass_001
 * @tc.desc: Test function of NClass::SaveClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_SaveClass_001";

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    EXPECT_CALL(GetMock(), napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(GetMock(), napi_add_env_cleanup_hook(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);

    // Verify results
    EXPECT_TRUE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_001";
}

/**
 * @tc.name: NClassTest_SaveClass_002
 * @tc.desc: Test function of NClass::SaveClass interface for FAILURE when napi_create_reference fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SaveClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SaveClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_SaveClass_002";
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    bool succ = NClass::SaveClass(env, className, classValue);

    // Verify results
    EXPECT_FALSE(succ);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SaveClass_002";
}

/**
 * @tc.name: NClassTest_CleanClass_001
 * @tc.desc: Test function of NClass::CleanClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_CleanClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_CleanClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_delete_reference(testing::_, testing::_)).WillOnce(testing::Return(napi_ok));

    // Do testing
    NClass::CleanClass(env);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_CleanClass_001";
}

/**
 * @tc.name: NClassTest_InstantiateClass_001
 * @tc.desc: Test function of NClass::InstantiateClass interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_001";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    EXPECT_CALL(GetMock(), napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(GetMock(), napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    napi_value expectedInstance = reinterpret_cast<napi_value>(0x3456);
    EXPECT_CALL(GetMock(), napi_new_instance(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<4>(expectedInstance), testing::Return(napi_ok)));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    EXPECT_EQ(instance, expectedInstance);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_001";
}

/**
 * @tc.name: NClassTest_InstantiateClass_002
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when class hasn't been saved yet.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_002";
    vector<napi_value> emptyArgs = {};

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_002";
}

/**
 * @tc.name: NClassTest_InstantiateClass_003
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when napi_get_reference_value fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_003";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    EXPECT_CALL(GetMock(), napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(GetMock(), napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_invalid_arg));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_003";
}

/**
 * @tc.name: NClassTest_InstantiateClass_004
 * @tc.desc: Test function of NClass::InstantiateClass interface for FAILURE when napi_new_instance fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_InstantiateClass_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_InstantiateClass_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    string className = "NClassTest_InstantiateClass_004";
    vector<napi_value> emptyArgs = {};

    // Set mock behaviors
    napi_value classValue = reinterpret_cast<napi_value>(0x1234);
    napi_ref mockReference = reinterpret_cast<napi_ref>(0x2345);
    EXPECT_CALL(GetMock(), napi_create_reference(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<3>(mockReference), testing::Return(napi_ok)));

    EXPECT_CALL(GetMock(), napi_get_reference_value(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    EXPECT_CALL(GetMock(), napi_new_instance(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Prepare instance
    bool succ = NClass::SaveClass(env, className, classValue);
    EXPECT_TRUE(succ);

    // Do testing
    auto instance = NClass::InstantiateClass(env, className, emptyArgs);

    // Verify results
    EXPECT_EQ(instance, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_InstantiateClass_004";
}

/**
 * @tc.name: NClassTest_GetEntityOf_001
 * @tc.desc: Test function of NClass::GetEntityOf interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    void *expectedEntity = reinterpret_cast<void *>(0x23456);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(expectedEntity), testing::Return(napi_ok)));

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, object);

    // Verify results
    EXPECT_EQ(entity, expectedEntity);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_001";
}

/**
 * @tc.name: NClassTest_GetEntityOf_002
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when arg env nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_002";
    // Prepare test parameters
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Do testing
    auto entity = NClass::GetEntityOf<string>(nullptr, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_002";
}

/**
 * @tc.name: NClassTest_GetEntityOf_003
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when arg objStat nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, nullptr);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_003";
}

/**
 * @tc.name: NClassTest_GetEntityOf_004
 * @tc.desc: Test function of NClass::GetEntityOf interface for FAILURE when napi_unwrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_GetEntityOf_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_GetEntityOf_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_unwrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto entity = NClass::GetEntityOf<string>(env, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_GetEntityOf_004";
}

/**
 * @tc.name: NClassTest_SetEntityFor_001
 * @tc.desc: Test function of NClass::SetEntityFor interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SetEntityFor_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SetEntityFor_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    string entity = "NClassTest_SetEntityFor_001";

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_wrap(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_ok));

    // Do testing
    bool result = NClass::SetEntityFor<string>(env, object, make_unique<string>(entity));

    // Verify results
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SetEntityFor_001";
}

/**
 * @tc.name: NClassTest_SetEntityFor_002
 * @tc.desc: Test function of NClass::SetEntityFor interface for FAILURE when napi_wrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_SetEntityFor_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_SetEntityFor_002";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    string entity = "NClassTest_SetEntityFor_001";

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_wrap(testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    bool result = NClass::SetEntityFor<string>(env, object, make_unique<string>(entity));

    // Verify results
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_SetEntityFor_002";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_001
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_001";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);
    void *expectedEntity = reinterpret_cast<void *>(0x23456);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<2>(expectedEntity), testing::Return(napi_ok)));

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, object);

    // Verify results
    EXPECT_EQ(entity, expectedEntity);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_001";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_002
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when arg env nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_002";
    // Prepare test parameters
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(nullptr, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_002";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_003
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when arg objStat nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_003";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, nullptr);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_003";
}

/**
 * @tc.name: NClassTest_RemoveEntityOfFinal_004
 * @tc.desc: Test function of NClass::RemoveEntityOfFinal interface for FAILURE when napi_remove_wrap fails.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 */
HWTEST_F(NClassTest, NClassTest_RemoveEntityOfFinal_004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NClassTest-begin NClassTest_RemoveEntityOfFinal_004";
    // Prepare test parameters
    napi_env env = reinterpret_cast<napi_env>(0x1000);
    napi_value object = reinterpret_cast<napi_value>(0x1234);

    // Set mock behaviors
    EXPECT_CALL(GetMock(), napi_remove_wrap(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(napi_generic_failure));

    // Do testing
    auto entity = NClass::RemoveEntityOfFinal<string>(env, object);

    // Verify results
    EXPECT_EQ(entity, nullptr);
    GTEST_LOG_(INFO) << "NClassTest-end NClassTest_RemoveEntityOfFinal_004";
}

} // namespace Test
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS