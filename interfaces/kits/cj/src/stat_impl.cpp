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

#include "stat_impl.h"
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <sys/xattr.h>
#endif
#include "macro.h"
#include "uni_error.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {
using namespace std;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
RetDataI32 StatImpl::GetLocation()
{
    auto value = static_cast<char*>(malloc(MAX_ATTR_NAME));
    if (value == nullptr) {
        return { .code = GetErrorCode(ENOMEM), .data = LOCAL };
    }
    ssize_t size = 0;
    if (fileInfo_->isPath) {
        size = getxattr(fileInfo_->path.get(), CLOUD_LOCATION_ATTR.c_str(), value, MAX_ATTR_NAME);
    } else {
        size = fgetxattr(fileInfo_->fdg->GetFD(), CLOUD_LOCATION_ATTR.c_str(), value, MAX_ATTR_NAME);
    }
    Location defaultLocation = LOCAL;
    if (size <= 0) {
        if (errno != ENODATA && errno != EOPNOTSUPP) {
            LOGE("Getxattr value failed, errno is %{public}d", errno);
        }
        free(value);
        return { .code = SUCCESS_CODE, .data = defaultLocation };
    }
    std::string location = string(value, static_cast<size_t>(size));
    free(value);
    if (!std::all_of(location.begin(), location.end(), ::isdigit)) {
        LOGE("Getxattr location is not all digit!");
        return { .code = SUCCESS_CODE, .data = defaultLocation };
    }
    defaultLocation = static_cast<Location>(atoi(location.c_str()));
    return { .code = SUCCESS_CODE, .data = defaultLocation };
}
#endif
}
}
}
