/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef REMOTE_URI_H
#define REMOTE_URI_H

#include <string>
#include <fcntl.h>

namespace OHOS {
namespace DistributedFS {
namespace ModuleRemoteUri {
const std::string FRAGMENT_TAG = "#";
const std::string FD_TAG = "=";
const std::string REMOTE_URI_TAG = "fdFromBinder";
const std::string SCHEME_TAG = ":";
const std::string SCHEME = "datashare";
class RemoteUri {
public:
    RemoteUri() {}
    static bool IsRemoteUri(const std::string& path, int &fd, const int& flags = O_RDONLY);
    static int ConvertUri(const int &fd, std::string &remoteUri);
    static int OpenRemoteUri(const std::string &remoteUri);
    ~RemoteUri() {}
};
} // namespace ModuleRemoteUri
} // namespace DistributedFS
} // namespace OHOS

#endif