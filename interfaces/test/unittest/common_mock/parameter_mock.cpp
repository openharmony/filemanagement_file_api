/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "parameter_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleEnvironment {
namespace Test {

thread_local std::shared_ptr<ParameterMock> ParameterMock::parameterMock = nullptr;
thread_local bool ParameterMock::mockable = false;

std::shared_ptr<ParameterMock> ParameterMock::GetMock()
{
    if (parameterMock == nullptr) {
        parameterMock = std::make_shared<ParameterMock>();
    }
    return parameterMock;
}

void ParameterMock::EnableMock()
{
    mockable = true;
}

void ParameterMock::DisableMock()
{
    parameterMock = nullptr;
    mockable = false;
}

bool ParameterMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleEnvironment
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using ParameterMock = OHOS::FileManagement::ModuleEnvironment::Test::ParameterMock;

int GetParameter(const char *key, const char *def, char *value, uint32_t len)
{
    if (ParameterMock::IsMockable()) {
        return ParameterMock::GetMock()->GetParameter(key, def, value, len);
    }

    static int (*realGetParameter)(const char *, const char *, char *, uint32_t) = []() {
        auto func = (int (*)(const char *, const char *, char *, uint32_t))dlsym(RTLD_NEXT, "GetParameter");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real GetParameter: " << dlerror();
        }
        return func;
    }();

    if (!realGetParameter) {
        return -1;
    }

    return realGetParameter(key, def, value, len);
}

} // extern "C"
#endif