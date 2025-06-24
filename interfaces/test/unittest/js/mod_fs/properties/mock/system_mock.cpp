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

#include "system_mock.h"

using namespace OHOS::FileManagement::ModuleFileIO;

extern "C" {
int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
{
    return System::ins->setxattr(path, name, value, size, flags);
}

int getxattr(const char *path, const char *name, void *value, size_t size)
{
    return System::ins->getxattr(path, name, value, size);
}

int fgetxattr(int filedes, const char *name, void *value, size_t size)
{
    return System::ins->fgetxattr(filedes, name, value, size);
}

int flock(int fd, int operation)
{
    return System::ins->flock(fd, operation);
}
}