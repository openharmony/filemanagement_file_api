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

#include "fdatasync_mock.h"
#include "n_class.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

std::shared_ptr<FdatasyncMock>FdatasyncMock::fdatasyncMock = nullptr;
bool FdatasyncMock::mockable = false;

std::shared_ptr<FdatasyncMock> FdatasyncMock::GetMock()
{
    if (fdatasyncMock == nullptr) {
        fdatasyncMock = std::make_shared<FdatasyncMock>();
    }
    return fdatasyncMock;
}

void FdatasyncMock::EnableMock()
{
    mockable = true;
}

void FdatasyncMock::DisableMock()
{
    fdatasyncMock = nullptr;
    mockable = false;
}

bool FdatasyncMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

using namespace OHOS::FileManagement::ModuleFileIO::Test;

NVal NVal::CreateUndefined(napi_env env)
{
    if (FdatasyncMock::IsMockable()) {
        return FdatasyncMock::GetMock()->CreateUndefined(env);
    }

    static NVal (*realCreateUndefined)(napi_env) = []() {
        auto func = (NVal(*)(napi_env))dlsym(RTLD_NEXT, "CreateUndefined");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real CreateUndefined: " << dlerror();
        }
        return func;
    }();

    if (!realCreateUndefined) {
        return {nullptr, nullptr};
    }

    return realCreateUndefined(env);
}