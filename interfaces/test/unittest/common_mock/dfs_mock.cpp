/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#include "dfs_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<DfsMock> DfsMock::dfsMock = nullptr;
thread_local bool DfsMock::mockable = false;

std::shared_ptr<DfsMock> DfsMock::GetMock()
{
    if (dfsMock == nullptr) {
        dfsMock = std::make_shared<DfsMock>();
    }
    return dfsMock;
}

void DfsMock::EnableMock()
{
    mockable = true;
}

void DfsMock::DisableMock()
{
    dfsMock = nullptr;
    mockable = false;
}

bool DfsMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

namespace OHOS::Storage::DistributedFile {
using DfsMock = OHOS::FileManagement::ModuleFileIO::Test::DfsMock;

DistributedFileDaemonManager &DistributedFileDaemonManager::GetInstance()
{
    if (DfsMock::IsMockable()) {
        return *DfsMock::GetMock();
    }

    static DistributedFileDaemonManager &(*realDfsGetInstance)() = []() {
        auto func = (DistributedFileDaemonManager &(*)())
            dlsym(RTLD_NEXT, "_ZN4OHOS7Storage15DistributedFile28DistributedFileDaemonManager11GetInstanceEv");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real DistributedFileDaemonManager::GetInstance: " << dlerror();
        }
        return func;
    }();

    if (!realDfsGetInstance) {
        return *DfsMock::GetMock();
    }

    return realDfsGetInstance();
}
} // namespace OHOS::Storage::DistributedFile
