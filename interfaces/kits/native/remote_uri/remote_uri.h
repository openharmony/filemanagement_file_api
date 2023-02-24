/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <set>
#include "uri.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleRemoteUri {

using namespace std;

const string FRAGMENT_TAG = "#";
const string FD_TAG = "=";
const string REMOTE_URI_TAG = "fdFromBinder";
const string SCHEME_TAG = ":";
const string SCHEME = "datashare";
const string SCHEME_FILE = "file";
const string ZERO_FD = "0";
const string MEDIA = "/media/";
const string PATH_SYMBOL = "/";
const int MAX_URI_SIZE  = 128;
class RemoteUri : public OHOS::Uri {
    static set<int>fdFromBinder;
    static void RemoveFd(int fd);
public:
    explicit RemoteUri(const std::string& uriString): Uri(uriString) {}
    static bool IsRemoteUri(const string& path, int &fd, const int& flags = O_RDONLY);
    static int ConvertUri(const int &fd, string &remoteUri);
    static int OpenRemoteUri(const string &remoteUri);
    static bool IsMediaUri(const string &uriString);
    static bool IsFileUri(const string &uriString);
    ~RemoteUri() {}
};
std::set<int>RemoteUri::fdFromBinder;
} // namespace ModuleRemoteUri
} // namespace DistributedFS
} // namespace OHOS

#endif