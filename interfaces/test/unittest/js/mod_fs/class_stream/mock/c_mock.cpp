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

#include "c_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<CMock> CMock::cMock = nullptr;
thread_local bool CMock::mockable = false;

std::shared_ptr<CMock> CMock::GetMock()
{
    if (cMock == nullptr) {
        cMock = std::make_shared<CMock>();
    }
    return cMock;
}

void CMock::EnableMock()
{
    mockable = true;
}

void CMock::DisableMock()
{
    cMock = nullptr;
    mockable = false;
}

bool CMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int fseek(FILE *stream, long len, int offset)
{
    if (CMock::IsMockable()) {
        return CMock::GetMock()->fseek(stream, len, offset);
    }

    static int (*realFseek)(FILE *, long, int) = []() {
        auto func = (int (*)(FILE *, long, int))dlsym(RTLD_NEXT, "fseek");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fseek: " << dlerror();
        }
        return func;
    }();

    if (!realFseek) {
        return -1;
    }

    return realFseek(stream, len, offset);
}

long ftell(FILE *stream)
{
    if (CMock::IsMockable()) {
        return CMock::GetMock()->ftell(stream);
    }

    static long (*realFtell)(FILE *) = []() {
        auto func = (long (*)(FILE *))dlsym(RTLD_NEXT, "ftell");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ftell: " << dlerror();
        }
        return func;
    }();

    if (!realFtell) {
        return -1;
    }

    return realFtell(stream);
}
} // extern "C"
#endif