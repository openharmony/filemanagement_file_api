/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "fileio_native.h"

#include <sys/xattr.h>

#include "file_uri.h"
#include "filemgmt_libhilog.h"

using namespace OHOS;
using namespace OHOS::FileManagement;

namespace {
    enum Location {
        LOCAL = 1,
        CLOUD = 2,
        LOCAL_AND_CLOUD = 3
    };

    const size_t MAX_ATTR_NAME = 64;
    const std::string CLOUD_LOCATION_ATTR = "user.cloud.location";

    bool CheckLocation(const std::string &location)
    {
        if (!std::all_of(location.begin(), location.end(), ::isdigit)) {
            return false;
        }
        int fileLocation = atoi(location.c_str());
        if (fileLocation < Location::LOCAL || fileLocation > Location::LOCAL_AND_CLOUD) {
            return false;
        }
        return true;
    }

    int GetLocationFromPath(const std::string &path)
    {
        std::unique_ptr<char[]> value = std::make_unique<char[]>(MAX_ATTR_NAME);
        if (value == nullptr) {
            HILOGE("Failed to request heap memory");
            return -ENOMEM;
        }
        ssize_t size = 0;
        size = getxattr(path.c_str(), CLOUD_LOCATION_ATTR.c_str(), value.get(), MAX_ATTR_NAME);
        Location defaultLocation = LOCAL;
        if (size <= 0) {
            if (errno == ENOENT) {
                return -ENOENT;
            } else if (errno != ENODATA && errno != EOPNOTSUPP) {
                HILOGE("Failed to getxattr, errno: %{public}d", errno);
            }
            return static_cast<int>(defaultLocation);
        }
        std::string location = std::string(value.get(), static_cast<size_t>(size));
        if (!CheckLocation(location)) {
            HILOGE("Invalid location from getxattr, location: %{public}s", location.c_str());
            return static_cast<int>(defaultLocation);
        }
        defaultLocation = static_cast<Location>(atoi(location.c_str()));
        return static_cast<int>(defaultLocation);
    }
}

int GetFileLocation(char *uri, int uriLength, int *location)
{
    if (uri == nullptr || location == nullptr || uriLength <= 0 || uriLength > PATH_MAX) {
        return -EINVAL;
    }
    std::string uriStr(uri, uriLength);
    AppFileService::ModuleFileUri::FileUri fileUri(uriStr);
    int ret = GetLocationFromPath(fileUri.GetRealPath());
    if (ret > 0) {
        *location = ret;
        return 0;
    }
    return ret;
}
