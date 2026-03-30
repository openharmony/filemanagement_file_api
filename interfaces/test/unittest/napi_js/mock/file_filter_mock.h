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

#ifndef INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_FILE_FILTER_MOCK_H
#define INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_FILE_FILTER_MOCK_H

#include "i_file_filter.h"

#include <gmock/gmock.h>
#include <node_api.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

class FileFilterMock : public IFileFilter {
public:
    MOCK_METHOD(bool, Filter, (const std::string &name), (override));
    MOCK_METHOD(bool, HasException, (), (const));
    MOCK_METHOD(napi_value, HandleException, (napi_env env), ());

public:
    static std::shared_ptr<FileFilterMock> GetMock();
    static void DisableMock();

private:
    static thread_local std::shared_ptr<FileFilterMock> filterMock;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_NAPI_JS_MOCK_FILE_FILTER_MOCK_H