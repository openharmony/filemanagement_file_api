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

#ifndef INTERFACES_TEST_UNITTEST_JS_MOD_FS_CLASS_STREAM_MOCK_C_MOCK_H
#define INTERFACES_TEST_UNITTEST_JS_MOD_FS_CLASS_STREAM_MOCK_C_MOCK_H

#include <cstdio>
#include <gmock/gmock.h>
#include <sys/stat.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {

class ICMock {
public:
    virtual ~ICMock() = default;
    virtual int fseek(FILE *, long, int) = 0;
    virtual long ftell(FILE *) = 0;
};

class CMock : public ICMock {
public:
    MOCK_METHOD(int, fseek, (FILE *, long, int), (override));
    MOCK_METHOD(long, ftell, (FILE *), (override));

public:
    static std::shared_ptr<CMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<CMock> cMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_JS_MOD_FS_CLASS_STREAM_MOCK_C_MOCK_H