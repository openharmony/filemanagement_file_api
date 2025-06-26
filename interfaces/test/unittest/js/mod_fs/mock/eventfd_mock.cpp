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

#include "eventfd_mock.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<EventfdMock> EventfdMock::eventfdMock = nullptr;

std::shared_ptr<EventfdMock> EventfdMock::GetMock()
{
    if (eventfdMock == nullptr) {
        eventfdMock = std::make_shared<EventfdMock>();
    }
    return eventfdMock;
}

void EventfdMock::DestroyMock()
{
    eventfdMock = nullptr;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int eventfd(unsigned int count, int flags)
{
    return EventfdMock::GetMock()->eventfd(count, flags);
}

} // extern "C"