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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_EVENTFD_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_EVENTFD_MOCK_H

#include <sys/eventfd.h>

#include <gmock/gmock.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

class IEventfdMock {
public:
    virtual ~IEventfdMock() = default;
    virtual int eventfd(unsigned int, int) = 0;
};

class EventfdMock : public IEventfdMock {
public:
    MOCK_METHOD(int, eventfd, (unsigned int, int), (override));

public:
    static std::shared_ptr<EventfdMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<EventfdMock> eventfdMock;
    static thread_local bool mockable;
};

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_EVENTFD_MOCK_H