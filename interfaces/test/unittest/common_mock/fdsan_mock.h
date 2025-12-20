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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_FDSAN_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_FDSAN_MOCK_H

#include <cstdio>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IFdsanMock {
public:
    virtual ~IFdsanMock() = default;
    virtual uint64_t fdsan_get_owner_tag(int fd) = 0;
    virtual int fdsan_close_with_tag(int fd, uint64_t expected_tag) = 0;
};

class FdsanMock : public IFdsanMock {
public:
    MOCK_METHOD(uint64_t, fdsan_get_owner_tag, (int), (override));
    MOCK_METHOD(int, fdsan_close_with_tag, (int, uint64_t), (override));

public:
    static std::shared_ptr<FdsanMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<FdsanMock> fdsanMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_FDSAN_MOCK_H