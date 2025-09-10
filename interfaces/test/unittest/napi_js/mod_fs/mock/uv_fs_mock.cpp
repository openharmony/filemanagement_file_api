/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "uv_fs_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

thread_local std::shared_ptr<UvfsMock> UvfsMock::uvfsMock = nullptr;
thread_local bool UvfsMock::mockable = false;

std::shared_ptr<UvfsMock> UvfsMock::GetMock()
{
    if (uvfsMock == nullptr) {
        uvfsMock = std::make_shared<UvfsMock>();
    }
    return uvfsMock;
}

void UvfsMock::EnableMock()
{
    mockable = true;
}

void UvfsMock::DisableMock()
{
    uvfsMock = nullptr;
    mockable = false;
}

bool UvfsMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int uv_fs_close(uv_loop_t *loop, uv_fs_t *req, uv_file file, uv_fs_cb cb)
{
    if (UvfsMock::IsMockable()) {
        return UvfsMock::GetMock()->uv_fs_close(loop, req, file, cb);
    }

    static int (*realUvFsClose)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb) = []() {
        auto func = (int (*)(uv_loop_t *, uv_fs_t *, uv_file, uv_fs_cb))dlsym(RTLD_NEXT, "uv_fs_close");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real uv_fs_close: " << dlerror();
        }
        return func;
    }();

    if (!realUvFsClose) {
        return -1;
    }

    return realUvFsClose(loop, req, file, cb);
}
#endif