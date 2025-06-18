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

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace Test {

PollMock &GetPollMock()
{
    return PollMock::GetMock();
}

} // namespace Test
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

extern "C" {
using namespace OHOS::FileManagement::ModuleFileIO::Test;

int poll(struct pollfd *fds, nfds_t n, int timeout)
{
    return GetPollMock().poll(fds, n, timeout);
}

} // extern "C"