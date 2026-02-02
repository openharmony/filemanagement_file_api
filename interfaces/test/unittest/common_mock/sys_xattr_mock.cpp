/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "sys_xattr_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<SysXattrMock> SysXattrMock::xattrMock = nullptr;
thread_local bool SysXattrMock::mockable = false;

std::shared_ptr<SysXattrMock> SysXattrMock::GetMock()
{
    if (xattrMock == nullptr) {
        xattrMock = std::make_shared<SysXattrMock>();
    }
    return xattrMock;
}

void SysXattrMock::EnableMock()
{
    mockable = true;
}

void SysXattrMock::DisableMock()
{
    xattrMock = nullptr;
    mockable = false;
}

bool SysXattrMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using SysXattrMock = OHOS::FileManagement::ModuleFileIO::Test::SysXattrMock;

int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
    if (SysXattrMock::IsMockable()) {
        return SysXattrMock::GetMock()->setxattr(path, name, value, size, flags);
    }

    static int (*realSetxattr)(const char *, const char *, const void *, size_t, int) = []() {
        auto func = (int (*)(const char *, const char *, const void *, size_t, int))dlsym(RTLD_NEXT, "setxattr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real setxattr: " << dlerror();
        }
        return func;
    }();

    if (!realSetxattr) {
        return -1;
    }

    return realSetxattr(path, name, value, size, flags);
}

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
    if (SysXattrMock::IsMockable()) {
        return SysXattrMock::GetMock()->getxattr(path, name, value, size);
    }

    static ssize_t (*realGetxattr)(const char *, const char *, void *, size_t) = []() {
        auto func = (ssize_t(*)(const char *, const char *, void *, size_t))dlsym(RTLD_NEXT, "getxattr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real getxattr: " << dlerror();
        }
        return func;
    }();

    if (!realGetxattr) {
        return -1;
    }

    return realGetxattr(path, name, value, size);
}

ssize_t fgetxattr(int filedes, const char *name, void *value, size_t size)
{
    if (SysXattrMock::IsMockable()) {
        return SysXattrMock::GetMock()->fgetxattr(filedes, name, value, size);
    }

    static ssize_t (*realFgetxattr)(int, const char *, void *, size_t) = []() {
        auto func = (ssize_t(*)(int, const char *, void *, size_t))dlsym(RTLD_NEXT, "fgetxattr");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fgetxattr: " << dlerror();
        }
        return func;
    }();

    if (!realFgetxattr) {
        return -1;
    }

    return realFgetxattr(filedes, name, value, size);
}

} // extern "C"
#endif