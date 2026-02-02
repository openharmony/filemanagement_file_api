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

#include "eventfd_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<EventfdMock> EventfdMock::eventfdMock = nullptr;
thread_local bool EventfdMock::mockable = false;

std::shared_ptr<EventfdMock> EventfdMock::GetMock()
{
    if (eventfdMock == nullptr) {
        eventfdMock = std::make_shared<EventfdMock>();
    }

    return eventfdMock;
}

void EventfdMock::EnableMock()
{
    mockable = true;
}

void EventfdMock::DisableMock()
{
    eventfdMock = nullptr;
    mockable = false;
}

bool EventfdMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using EventfdMock = OHOS::FileManagement::ModuleFileIO::Test::EventfdMock;

int eventfd(unsigned int count, int flags)
{
    if (EventfdMock::IsMockable()) {
        return EventfdMock::GetMock()->eventfd(count, flags);
    }

    static int (*realEventfd)(unsigned int, int) = []() {
        auto func = (int (*)(unsigned int, int))dlsym(RTLD_NEXT, "eventfd");
        if (!func) {
            GTEST_LOG_(ERROR) << "Failed to resolve real eventfd: " << dlerror();
        }
        return func;
    }();

    if (!realEventfd) {
        return -1;
    }

    return realEventfd(count, flags);
}

} // extern "C"
#endif