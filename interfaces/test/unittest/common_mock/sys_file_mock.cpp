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

#include "sys_file_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<SysFileMock> SysFileMock::fileMock = nullptr;
thread_local bool SysFileMock::mockable = false;

std::shared_ptr<SysFileMock> SysFileMock::GetMock()
{
    if (fileMock == nullptr) {
        fileMock = std::make_shared<SysFileMock>();
    }
    return fileMock;
}

void SysFileMock::EnableMock()
{
    mockable = true;
}

void SysFileMock::DisableMock()
{
    fileMock = nullptr;
    mockable = false;
}

bool SysFileMock::IsMockable()
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

int flock(int fd, int operation)
{
    if (SystemMock::IsMockable()) {
        return SystemMock::GetMock()->flock(fd, operation);
    }

    static int (*realFlock)(int, int) = []() {
        auto func = (int (*)(int, int))dlsym(RTLD_NEXT, "flock");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real flock: " << dlerror();
        }
        return func;
    }();

    if (!realFlock) {
        return -1;
    }

    return realFlock(fd, operation);
}

} // extern "C"
#endif