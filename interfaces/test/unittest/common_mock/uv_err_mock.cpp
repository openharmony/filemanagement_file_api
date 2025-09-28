/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "uv_err_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<UvErrMock> UvErrMock::uvErrMock = nullptr;
thread_local bool UvErrMock::mockable = false;

std::shared_ptr<UvErrMock> UvErrMock::GetMock()
{
    if (uvErrMock == nullptr) {
        uvErrMock = std::make_shared<UvErrMock>();
    }
    return uvErrMock;
}

void UvErrMock::EnableMock()
{
    mockable = true;
}

void UvErrMock::DisableMock()
{
    uvErrMock = nullptr;
    mockable = false;
}

bool UvErrMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

const char *uv_err_name(int err)
{
    if (UvErrMock::IsMockable()) {
        return UvErrMock::GetMock()->uv_err_name(err);
    }

    static const char *(*realUvErrName)(int) = []() {
        auto func = (const char *(*)(int))dlsym(RTLD_NEXT, "uv_err_name");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_err_name: " << dlerror();
        }
        return func;
    }();

    if (!realUvErrName) {
        return "";
    }

    return realUvErrName(err);
}
} // extern "C"
#endif