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

#include "xattr_core.h"

#include <memory>
#include <optional>
#include <sys/xattr.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
constexpr size_t MAX_XATTR_SIZE = 4096;

static bool IsIllegalXattr(const char *key, const char *value)
{
    bool isIllegalKey = strnlen(key, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalKey) {
        HILOGE("Key is too long");
    }
    bool isIllegalValue = strnlen(value, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalValue) {
        HILOGE("value is too long");
    }
    return isIllegalKey || isIllegalValue;
}

static int32_t GetXattrCore(const char *path, const char *key, std::shared_ptr<string> result)
{
    ssize_t xAttrSize = getxattr(path, key, nullptr, 0);
    if (xAttrSize == -1 || xAttrSize == 0) {
        *result = "";
        return ERRNO_NOERR;
    }
    auto xattrValue = CreateUniquePtr<char[]>(static_cast<long>(xAttrSize) + 1);
    xAttrSize = getxattr(path, key, xattrValue.get(), static_cast<size_t>(xAttrSize));
    if (xAttrSize == -1) {
        return errno;
    }
    xattrValue[xAttrSize] = '\0';
    *result = std::string(xattrValue.get());
    return ERRNO_NOERR;
}

FsResult<void> XattrCore::DoSetXattr(const string &path, const string &key, const string &value)
{
    if (IsIllegalXattr(key.c_str(), value.c_str())) {
        HILOGE("Invalid xattr value");
        return FsResult<void>::Error(EINVAL);
    }
    if (setxattr(path.c_str(), key.c_str(), value.c_str(), strnlen(value.c_str(), MAX_XATTR_SIZE), 0) < 0) {
        HILOGE("Setxattr fail, errno is %{public}d", errno);
        return FsResult<void>::Error(errno);
    }
    return FsResult<void>::Success();
}

FsResult<string> XattrCore::DoGetXattr(const string &path, const string &key)
{
    auto result = make_shared<std::string>();
    int32_t ret = GetXattrCore(path.c_str(), key.c_str(), result);
    if (ret != ERRNO_NOERR) {
        HILOGE("Invalid getxattr");
        return FsResult<string>::Error(ret);
    }
    return FsResult<string>::Success(move(*result));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS