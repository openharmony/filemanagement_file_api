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

#include "unistd_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<UnistdMock> UnistdMock::unistdMock = nullptr;
thread_local bool UnistdMock::mockable = false;

std::shared_ptr<UnistdMock> UnistdMock::GetMock()
{
    if (unistdMock == nullptr) {
        unistdMock = std::make_shared<UnistdMock>();
    }

    return unistdMock;
}

void UnistdMock::EnableMock()
{
    mockable = true;
}

void UnistdMock::DisableMock()
{
    unistdMock = nullptr;
    mockable = false;
}

bool UnistdMock::IsMockable()
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

int access(const char *filename, int amode)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->access(filename, amode);
    }

    static int (*realAccess)(const char *filename, int amode) = []() {
        auto func = (int (*)(const char *, int))dlsym(RTLD_NEXT, "access");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real access: " << dlerror();
        }
        return func;
    }();

    if (!realAccess) {
        return -1;
    }

    return realAccess(filename, amode);
}

int close(int fd)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->close(fd);
    }

    static int (*realClose)(int fd) = []() {
        auto func = (int (*)(int))dlsym(RTLD_NEXT, "close");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real close: " << dlerror();
        }
        return func;
    }();

    if (!realClose) {
        return -1;
    }

    return realClose(fd);
}

ssize_t read(int fd, void *buf, size_t count)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->read(fd, buf, count);
    }

    static ssize_t (*realRead)(int fd, void *buf, size_t count) = []() {
        auto func = (ssize_t(*)(int, void *, size_t))dlsym(RTLD_NEXT, "read");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real read: " << dlerror();
        }
        return func;
    }();

    if (!realRead) {
        return 0;
    }

    return realRead(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->write(fd, buf, count);
    }

    static ssize_t (*realWrite)(int fd, const void *buf, size_t count) = []() {
        auto func = (ssize_t(*)(int, const void *, size_t))dlsym(RTLD_NEXT, "write");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real write: " << dlerror();
        }
        return func;
    }();

    if (!realWrite) {
        return 0;
    }

    return realWrite(fd, buf, count);
}

} // extern "C"
#endif
