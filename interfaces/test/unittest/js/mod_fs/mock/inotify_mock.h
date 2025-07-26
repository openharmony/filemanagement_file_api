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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_INOTIFY_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_INOTIFY_MOCK_H

#include <gmock/gmock.h>
#include <sys/inotify.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class IInotifyMock {
public:
    virtual ~IInotifyMock() = default;
    virtual int inotify_init() = 0;
    virtual int inotify_add_watch(int, const char *, uint32_t) = 0;
    virtual int inotify_rm_watch(int, int) = 0;
};

class InotifyMock : public IInotifyMock {
public:
    MOCK_METHOD(int, inotify_init, (), (override));
    MOCK_METHOD(int, inotify_add_watch, (int, const char *, uint32_t), (override));
    MOCK_METHOD(int, inotify_rm_watch, (int, int), (override));

public:
    static std::shared_ptr<InotifyMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<InotifyMock> inotifyMock;
    static thread_local bool mockable;
};

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_TEST_UNITTEST_JS_MOD_FS_MOCK_INOTIFY_MOCK_H