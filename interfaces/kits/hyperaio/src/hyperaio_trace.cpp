/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hyperaio_trace.h"

namespace OHOS {
namespace HyperAio {

HyperaioTrace::HyperaioTrace(const std::string& value, bool isShowLog) : value_(value)
{
    if (isShowLog) {
        HILOGI("%{public}s", value_.c_str());
    }
    StartTrace(HITRACE_TAG_OHOS, "[HyperAio]" + value);
}

void HyperaioTrace::End()
{
    if (!isFinished_) {
        FinishTrace(HITRACE_TAG_OHOS);
        isFinished_ = true;
    }
}

HyperaioTrace::~HyperaioTrace()
{
    End();
}
} // namespace HyperAio
} // namespace OHOS