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

#include "fdsan_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<FdsanMock> FdsanMock::fdsanMock = nullptr;
thread_local bool FdsanMock::mockable = false;

std::shared_ptr<FdsanMock> FdsanMock::GetMock()
{
    if (fdsanMock == nullptr) {
        fdsanMock = std::make_shared<FdsanMock>();
    }
    return fdsanMock;
}

void FdsanMock::EnableMock()
{
    mockable = true;
}

void FdsanMock::DisableMock()
{
    fdsanMock = nullptr;
    mockable = false;
}

bool FdsanMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

uint64_t fdsan_get_owner_tag(int fd)
{
    if (FdsanMock::IsMockable()) {
        return FdsanMock::GetMock()->fdsan_get_owner_tag(fd);
    }

    static uint64_t (*realFdsanGetOwnerTag)(int) = []() {
        auto func = (uint64_t (*)(int))dlsym(RTLD_NEXT, "fdsan_get_owner_tag");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fdsan_get_owner_tag: " << dlerror();
        }
        return func;
    }();

    if (!realFdsanGetOwnerTag) {
        return -1;
    }

    return realFdsanGetOwnerTag(fd);
}

int fdsan_close_with_tag(int fd, uint64_t expected_tag)
{
    if (FdsanMock::IsMockable()) {
        return FdsanMock::GetMock()->fdsan_close_with_tag(fd, expected_tag);
    }

    static int (*realFdsanCloseWithTag)(int, uint64_t) = []() {
        auto func = (int (*)(int, uint64_t))dlsym(RTLD_NEXT, "fdsan_close_with_tag");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fdsan_close_with_tag: " << dlerror();
        }
        return func;
    }();

    if (!realFdsanCloseWithTag) {
        return -1;
    }

    return realFdsanCloseWithTag(fd, expected_tag);
}

} // extern "C"
#endif