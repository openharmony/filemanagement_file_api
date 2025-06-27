/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_MOCK_WATCHER_CALLBACK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_MOCK_WATCHER_CALLBACK_H

#include <string>
#include <gmock/gmock.h>
#include "i_watcher_callback.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {
using namespace OHOS::FileManagement::ModuleFileIO;

class MockWatcherCallback : public IWatcherCallback {
public:
    MOCK_METHOD(bool, IsStrictEquals, (const std::shared_ptr<IWatcherCallback> &other), (const, override));
    MOCK_METHOD(
        void, InvokeCallback, (const std::string &fileName, uint32_t event, uint32_t cookie), (const, override));
    MOCK_METHOD(std::string, GetClassName, (), (const, override));
};

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_MOCK_WATCHER_CALLBACK_H