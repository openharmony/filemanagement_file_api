/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_FILEMANAGEMENT_FILEAPI_INTERFACES_INNERKITS_SECURITY_LABEL_H
#define FOUNDATION_FILEMANAGEMENT_FILEAPI_INTERFACES_INNERKITS_SECURITY_LABEL_H

#include <string>

namespace OHOS {
namespace FileIO {
class SecurityLabel {
public:
    static bool SetSecurityLabel(std::string path, std::string dataLevel)
    {
        return true;
    }
    static std::string GetSecurityLabel(std::string path)
    {
        return "data.security.level";
    };
};
} // namespace FileIO
} // namespace OHOS

#endif