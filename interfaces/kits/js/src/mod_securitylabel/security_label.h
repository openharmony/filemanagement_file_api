/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef SECURITY_LABEL_H
#define SECURITY_LABEL_H

#include <cerrno>
#include <memory>
#include <set>
#include <string>
#include <sys/xattr.h>

namespace OHOS {
namespace FileManagement {
namespace ModuleSecurityLabel {
const char XATTR_KEY[] = {"user.security"};
const std::string DEFAULT_DATA_LEVEL = "s3";
const std::set<std::string> DATA_LEVEL = {"s0", "s1", "s2", "s3", "s4"};
class SecurityLabel {
public:
    static bool SetSecurityLabel(const std::string &path, const std::string &dataLevel)
    {
        if (DATA_LEVEL.count(dataLevel) != 1) {
            return false;
        }
#ifdef IOS_PLATFORM
        if (setxattr(path.c_str(), XATTR_KEY, dataLevel.c_str(), dataLevel.size(), 0, 0) < 0) {
#else
        if (setxattr(path.c_str(), XATTR_KEY, dataLevel.c_str(), dataLevel.size(), 0) < 0) {
#endif
            return false;
        }
        return true;
    }

    static std::string GetSecurityLabel(const std::string &path)
    {
#ifdef IOS_PLATFORM
        auto xattrValueSize = getxattr(path.c_str(), XATTR_KEY, nullptr, 0, 0, 0);
#else
        auto xattrValueSize = getxattr(path.c_str(), XATTR_KEY, nullptr, 0);
#endif
        if (xattrValueSize == -1) {
            return "";
        }
        if (xattrValueSize == 0) {
            return DEFAULT_DATA_LEVEL;
        }
        std::unique_ptr<char[]> xattrValue = std::make_unique<char[]>((long)xattrValueSize + 1);
        if (xattrValue == nullptr) {
            return "";
        }
#ifdef IOS_PLATFORM
        xattrValueSize = getxattr(path.c_str(), XATTR_KEY, xattrValue.get(), xattrValueSize, 0, 0);
#else
        xattrValueSize = getxattr(path.c_str(), XATTR_KEY, xattrValue.get(), xattrValueSize);
#endif
        if (xattrValueSize == -1) {
            return "";
        }
        if (xattrValueSize == 0) {
            return DEFAULT_DATA_LEVEL;
        }
        return std::string(xattrValue.get());
    }
};
} // namespace ModuleSecurityLabel
} // namespace FileManagement
} // namespace OHOS
#endif