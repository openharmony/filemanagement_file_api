/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "securitylabel_core.h"

#include "filemgmt_libhilog.h"
#include "security_label.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
using namespace std;
FsResult<void> DoSetSecurityLabel(const string &path, const string &dataLevel)
{
    if (DATA_LEVEL.find(dataLevel) == DATA_LEVEL.end()) {
        HILOGE("Invalid Argument of dataLevelEnum");
        return FsResult<void>::Error(EINVAL);
    }

    bool ret = SecurityLabel::SetSecurityLabel(path, dataLevel);
    if (!ret) {
        return FsResult<void>::Error(errno);
    }

    return FsResult<void>::Success();
}

} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS