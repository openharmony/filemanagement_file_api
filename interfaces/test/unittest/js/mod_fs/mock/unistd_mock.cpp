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

static int (*real_access)(const char *filename, int amode) = nullptr;
static int (*real_close)(int fd) = nullptr;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = nullptr;

int access(const char *filename, int amode)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->access(filename, amode);
    }

    real_access = (int (*)(const char *, int))dlsym(RTLD_NEXT, "access");
    if (!real_access) {
        GTEST_LOG_(ERROR) << "Failed to resolve real access" << dlerror();
        return -1;
    }
    return real_access(filename, amode);
}

int close(int fd)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->close(fd);
    }

    real_close = (int (*)(int))dlsym(RTLD_NEXT, "close");
    if (!real_close) {
        GTEST_LOG_(ERROR) << "Failed to resolve real close" << dlerror();
        return -1;
    }
    return real_close(fd);
}

ssize_t read(int fd, void *buf, size_t count)
{
    if (UnistdMock::IsMockable()) {
        return UnistdMock::GetMock()->read(fd, buf, count);
    }

    real_read = (ssize_t(*)(int, void *, size_t))dlsym(RTLD_NEXT, "read");
    if (!real_read) {
        GTEST_LOG_(ERROR) << "Failed to resolve real read" << dlerror();
        return 0;
    }
    return real_read(fd, buf, count);
}

} // extern "C"
#endif