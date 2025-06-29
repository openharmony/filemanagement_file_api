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

#include "poll_mock.h"

#include <dlfcn.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

thread_local std::shared_ptr<PollMock> PollMock::pollMock = nullptr;
thread_local bool PollMock::mockable = false;

std::shared_ptr<PollMock> PollMock::GetMock()
{
    if (pollMock == nullptr) {
        pollMock = std::make_shared<PollMock>();
    }
    return pollMock;
}

void PollMock::EnableMock()
{
    mockable = true;
}

void PollMock::DisableMock()
{
    pollMock = nullptr;
    mockable = false;
}

bool PollMock::IsMockable()
{
    return mockable;
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

static int (*real_poll)(struct pollfd *fds, nfds_t n, int timeout) = nullptr;

int poll(struct pollfd *fds, nfds_t n, int timeout)
{
    if (PollMock::IsMockable()) {
        return PollMock::GetMock()->poll(fds, n, timeout);
    }

    real_poll = (int (*)(struct pollfd *, nfds_t, int))dlsym(RTLD_NEXT, "poll");
    if (!real_poll) {
        GTEST_LOG_(ERROR) << "Failed to resolve real poll" << dlerror();
        return -1;
    }
    return real_poll(fds, n, timeout);
}

} // extern "C"
#endif