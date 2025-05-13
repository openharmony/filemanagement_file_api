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

#include "xattr.h"
#include <cstring>
#include <sys/xattr.h>
#include "macro.h"
#include "securec.h"

using namespace std;

namespace {
constexpr size_t MAX_XATTR_SIZE = 4096;

char* CreateCString(string str)
{
    auto ret = static_cast<char*>(malloc(str.size() + 1));
    if (ret == nullptr) {
        return nullptr;
    }
    if (strcpy_s(ret, str.size() + 1, str.c_str()) != 0) {
        free(ret);
        return nullptr;
    }
    return ret;
}

bool IsIllegalXattr(const char *key, const char *value)
{
    bool isIllegalKey = strnlen(key, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalKey) {
        LOGE("key is too loog");
    }
    bool isIllegalValue = strnlen(value, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalValue) {
        LOGE("value is too loog");
    }
    return isIllegalKey || isIllegalValue;
}
}

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

int32_t Xattr::SetSync(const char *path, const char *key, const char *value)
{
    if (IsIllegalXattr(key, value)) {
        return EINVAL;
    }
    if (setxattr(path, key, value, strnlen(value, MAX_XATTR_SIZE), 0) < 0) {
        LOGE("setxattr fail, errno is %{public}d", errno);
        return errno;
    }
    return SUCCESS_CODE;
}

tuple<int32_t, char*> Xattr::GetSync(const char *path, const char *key)
{
    ssize_t xAttrSize = getxattr(path, key, nullptr, 0);
    if (xAttrSize == -1 || xAttrSize == 0) {
        auto result = CreateCString("");
        if (result == nullptr) {
            return { ENOMEM, nullptr };
        }
        return { SUCCESS_CODE, result };
    }
    auto xAttrValue = static_cast<char*>(malloc(static_cast<long>(xAttrSize) + 1));
    if (xAttrValue == nullptr) {
        return { ENOMEM, nullptr };
    }
    xAttrSize = getxattr(path, key, xAttrValue, static_cast<size_t>(xAttrSize));
    if (xAttrSize == -1) {
        free(xAttrValue);
        return { errno, nullptr };
    }
    xAttrValue[xAttrSize] = '\0';
    return { SUCCESS_CODE, xAttrValue };
}

} // namespace FileFs
} // namespace CJSystemapi
} // namespace OHOS
