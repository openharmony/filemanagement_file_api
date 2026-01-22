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

#include "stdio_mock.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

thread_local std::shared_ptr<StdioMock> StdioMock::stdioMock = nullptr;
thread_local bool StdioMock::mockable = false;

std::shared_ptr<StdioMock> StdioMock::GetMock()
{
    if (stdioMock == nullptr) {
        stdioMock = std::make_shared<StdioMock>();
    }
    return stdioMock;
}

void StdioMock::EnableMock()
{
    mockable = true;
}

void StdioMock::DisableMock()
{
    stdioMock = nullptr;
    mockable = false;
}

bool StdioMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

#ifdef __cplusplus
extern "C" {
using StdioMock = OHOS::FileManagement::ModuleFileIO::Test::StdioMock;

int fseek(FILE *stream, long len, int offset)
{
    if (StdioMock::IsMockable()) {
        return StdioMock::GetMock()->fseek(stream, len, offset);
    }

    static int (*realFseek)(FILE *, long, int) = []() {
        auto func = (int (*)(FILE *, long, int))dlsym(RTLD_NEXT, "fseek");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real fseek: " << dlerror();
        }
        return func;
    }();

    if (!realFseek) {
        return -1;
    }

    return realFseek(stream, len, offset);
}

long ftell(FILE *stream)
{
    if (StdioMock::IsMockable()) {
        return StdioMock::GetMock()->ftell(stream);
    }

    static long (*realFtell)(FILE *) = []() {
        auto func = (long (*)(FILE *))dlsym(RTLD_NEXT, "ftell");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real ftell: " << dlerror();
        }
        return func;
    }();

    if (!realFtell) {
        return -1;
    }

    return realFtell(stream);
}

int remove(const char *path)
{
    if (StdioMock::IsMockable()) {
        return StdioMock::GetMock()->remove(path);
    }

    static int (*realRemove)(const char *) = []() {
        auto func = (int (*)(const char *))dlsym(RTLD_NEXT, "remove");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real remove: " << dlerror();
        }
        return func;
    }();

    if (!realRemove) {
        return 1;
    }

    return realRemove(path);
}

int rename(const char *oldName, const char *newName)
{
    if (StdioMock::IsMockable()) {
        return StdioMock::GetMock()->rename(oldName, newName);
    }

    static int (*realRename)(const char *, const char *) = []() {
        auto func = (int (*)(const char *, const char *))dlsym(RTLD_NEXT, "rename");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real rename: " << dlerror();
        }
        return func;
    }();

    if (!realRename) {
        return 1;
    }

    return realRename(oldName, newName);
}

} // extern "C"
#endif