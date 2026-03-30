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

#include "file_filter_mock.h"

#include "file_filter_napi.h"

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<FileFilterMock> FileFilterMock::filterMock = nullptr;

std::shared_ptr<FileFilterMock> FileFilterMock::GetMock()
{
    if (filterMock == nullptr) {
        filterMock = std::make_shared<FileFilterMock>();
    }
    return filterMock;
}

void FileFilterMock::DisableMock()
{
    filterMock = nullptr;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

using FileFilterNapi = OHOS::FileManagement::ModuleFileIO::FileFilterNapi;
using FileFilterMock = OHOS::FileManagement::ModuleFileIO::Test::FileFilterMock;

std::thread::id FileFilterNapi::mainThreadId;
std::once_flag FileFilterNapi::initFlag;

bool FileFilterNapi::Filter(const std::string &name)
{
    return FileFilterMock::GetMock()->Filter(name);
}

FileFilterNapi::FileFilterNapi(napi_env env, napi_value filterObj, LibN::NVal callback)
    : env_(env), filterObj_(filterObj), nRef_(callback)
{}

FileFilterNapi::~FileFilterNapi() {}

void FileFilterNapi::InitMainThreadId() {}

bool FileFilterNapi::CallFilterFunction(const std::string &name)
{
    return true;
}

bool FileFilterNapi::AsyncCallFilterFunction(const std::string &name)
{
    return true;
}
