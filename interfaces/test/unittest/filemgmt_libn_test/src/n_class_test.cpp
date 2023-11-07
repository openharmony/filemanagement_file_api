/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "n_class_test.h"
#include "n_class.h"

using namespace std;
using namespace OHOS;
using namespace testing::ext;

namespace OHOS {
namespace Media {
void FilemangementLibnTest::SetUpTestCase(void) {}

void FilemangementLibnTest::TearDownTestCase(void) {}

void FilemangementLibnTest::SetUp() {}

void FilemangementLibnTest::TearDown() {}
    
HWTEST_F(FilemangementLibnTest, NClassTest_DefineClass_001, TestSize.Level1)
{
    string className = "TestClassAB";
    napi_callback vlaue = nullptr;
    vector<napi_property_descriptor> props = {};
    bool succ = false;
    napi_value classValue = nullptr;
    napi_env envTest = nullptr;
    tie(succ, classValue) = FileManagement::LibN::NClass::DefineClass(envTest, className, vlaue, std::move(props));
    EXPECT_EQ(succ, false);
    EXPECT_EQ((classValue == nullptr), true);
}

HWTEST_F(FilemangementLibnTest, NClassTest_SaveClass_001, TestSize.Level1)
{
    string className = "TestClassAB";
    bool succ = false;
    napi_value classValue = nullptr;
    napi_env envTest = nullptr;
    succ = FileManagement::LibN::NClass::SaveClass(envTest, className, classValue);
    EXPECT_EQ(succ, false);
}
} // namespace Media
} // namespace OHOS