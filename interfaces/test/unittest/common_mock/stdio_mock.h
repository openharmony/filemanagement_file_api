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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDIO_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDIO_MOCK_H

#include <cstdio>

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class IStdioMock {
public:
    virtual ~IStdioMock() = default;
    virtual int fseek(FILE *, long, int) = 0;
    virtual long ftell(FILE *) = 0;
    virtual int remove(const char *) = 0;
    virtual int rename(const char *, const char *) = 0;
};

class StdioMock : public IStdioMock {
public:
    MOCK_METHOD(int, fseek, (FILE *, long, int), (override));
    MOCK_METHOD(long, ftell, (FILE *), (override));
    MOCK_METHOD(int, remove, (const char *), (override));
    MOCK_METHOD(int, rename, (const char *, const char *), (override));

public:
    static std::shared_ptr<StdioMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<StdioMock> stdioMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_STDIO_MOCK_H