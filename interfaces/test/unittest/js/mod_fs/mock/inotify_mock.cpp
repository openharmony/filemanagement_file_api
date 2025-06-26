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

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<InotifyMock> InotifyMock::inotifyMock = nullptr;

std::shared_ptr<InotifyMock> InotifyMock::GetMock()
{
    if (inotifyMock == nullptr) {
        inotifyMock = std::make_shared<InotifyMock>();
    }
    return inotifyMock;
}

void InotifyMock::DestroyMock()
{
    inotifyMock = nullptr;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int inotify_init()
{
    return InotifyMock::GetMock()->inotify_init();
}

int inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
    return InotifyMock::GetMock()->inotify_add_watch(fd, pathname, mask);
}

int inotify_rm_watch(int fd, int wd)
{
    return InotifyMock::GetMock()->inotify_rm_watch(fd, wd);
}

} // extern "C"