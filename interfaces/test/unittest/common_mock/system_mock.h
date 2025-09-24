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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_SYSTEM_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_SYSTEM_MOCK_H

#include <sys/xattr.h>
#include <sys/file.h>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual int setxattr(const char *path, const char *name, const void *value, size_t size, int flags) = 0;
    virtual ssize_t getxattr(const char *path, const char *name, void *value, size_t size) = 0;
    virtual ssize_t fgetxattr(int filedes, const char *name, void *value, size_t size) = 0;
    virtual int flock(int fd, int operation) = 0;
};

class SystemMock : public ISystem {
public:
    MOCK_METHOD(
        int, setxattr, (const char *path, const char *name, const void *value, size_t size, int flags), (override));
    MOCK_METHOD(ssize_t, getxattr, (const char *path, const char *name, void *value, size_t size), (override));
    MOCK_METHOD(ssize_t, fgetxattr, (int filedes, const char *name, void *value, size_t size), (override));
    MOCK_METHOD(int, flock, (int fd, int operation), (override));

public:
    static std::shared_ptr<SystemMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<SystemMock> systemMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_SYSTEM_MOCK_H