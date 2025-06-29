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

#include "inotify_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<InotifyMock> InotifyMock::inotifyMock = nullptr;
thread_local bool InotifyMock::mockable = false;

std::shared_ptr<InotifyMock> InotifyMock::GetMock()
{
    if (inotifyMock == nullptr) {
        inotifyMock = std::make_shared<InotifyMock>();
    }
    return inotifyMock;
}

void InotifyMock::EnableMock()
{
    mockable = true;
}

void InotifyMock::DisableMock()
{
    inotifyMock = nullptr;
    mockable = false;
}

bool InotifyMock::IsMockable()
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

static int (*real_inotify_init)() = nullptr;
static int (*real_inotify_add_watch)(int, const char *, uint32_t) = nullptr;
static int (*real_inotify_rm_watch)(int, int) = nullptr;

int inotify_init()
{
    if (InotifyMock::IsMockable()) {
        return InotifyMock::GetMock()->inotify_init();
    }

    real_inotify_init = (int (*)())dlsym(RTLD_NEXT, "inotify_init");
    if (!real_inotify_init) {
        GTEST_LOG_(ERROR) << "Failed to resolve real inotify_init" << dlerror();
        return -1;
    }
    return real_inotify_init();
}

int inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
    if (InotifyMock::IsMockable()) {
        return InotifyMock::GetMock()->inotify_add_watch(fd, pathname, mask);
    }

    real_inotify_add_watch = (int (*)(int, const char *, uint32_t))dlsym(RTLD_NEXT, "inotify_add_watch");
    if (!real_inotify_add_watch) {
        GTEST_LOG_(ERROR) << "Failed to resolve real inotify_add_watch" << dlerror();
        return -1;
    }
    return real_inotify_add_watch(fd, pathname, mask);
}

int inotify_rm_watch(int fd, int wd)
{
    if (InotifyMock::IsMockable()) {
        return InotifyMock::GetMock()->inotify_rm_watch(fd, wd);
    }

    real_inotify_rm_watch = (int (*)(int, int))dlsym(RTLD_NEXT, "inotify_rm_watch");
    if (!real_inotify_rm_watch) {
        GTEST_LOG_(ERROR) << "Failed to resolve real inotify_rm_watch" << dlerror();
        return -1;
    }
    return real_inotify_rm_watch(fd, wd);
}

} // extern "C"
#endif