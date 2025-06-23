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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_SYSTEM_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_SYSTEM_MOCK_H

#include <sys/file.h>
#include <vector>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO {

class System {
public:
    static inline std::shared_ptr<System> ins = nullptr;

public:
    virtual ~System() = default;
    virtual int setxattr(const char *path, const char *name, const void *value, size_t size, int flags) = 0;
    virtual int getxattr(const char *path, const char *name, void *value, size_t size) = 0;
    virtual int fgetxattr(int filedes, const char *name, void *value, size_t size) = 0;
    virtual int flock(int fd, int operation) = 0;
};

class SystemMock : public System {
public:
    MOCK_METHOD5(setxattr, int(const char *path, const char *name, const void *value, size_t size, int flags));
    MOCK_METHOD4(getxattr, int(const char *path, const char *name, void *value, size_t size));
    MOCK_METHOD4(fgetxattr, int(int filedes, const char *name, void *value, size_t size));
    MOCK_METHOD2(flock, int(int fd, int operation));
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_FS_PROPERTIES_MOCK_SYSTEM_MOCK_H