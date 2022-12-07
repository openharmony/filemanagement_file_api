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

#include "remote_uri.h"
#include <string>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "hap_token_info.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleRemoteUri {

using namespace std;

bool RemoteUri::IsMediaUri(const string &path)
{
    string::size_type posDataShare = path.find(SCHEME_TAG);
    if (posDataShare == string::npos) {
        return false;
    }
    string scheme = path.substr(0, posDataShare);
    if (scheme != SCHEME) {
        return false;
    }

    string::size_type pathSlashPos = path.find(PATH_SYMBOL);
    if (pathSlashPos == string::npos) {
        return false;
    }

    string pathNoScheme = path.substr(pathSlashPos);
    if (pathNoScheme.empty() || pathNoScheme.length() <= MEDIA.length()) {
        return false;
    }

    char s1 = pathNoScheme[0];
    char s2 = pathNoScheme[1];
    if (s1 != PATH_SYMBOL[0] || s2 != PATH_SYMBOL[0]) {
        return false;
    }
    
    string str = pathNoScheme.substr(2);
    if (str.find(PATH_SYMBOL) == string::npos) {
        return false;
    }

    int position = str.find_first_of(PATH_SYMBOL);
    int len = position + 1;
    if (str.length() == len) {
        return false;
    }

    string s = str.substr(len);
    if (s.empty() || s.length() < MEDIA.length()) {
        return false;
    }

    string media = str.substr(len, MEDIA.length());
    if (media != MEDIA) {
        return false;
    }

    return true;
}

static bool IsAllDigits(string fdStr)
{
    for (size_t i = 0; i < fdStr.size(); i++) {
        if (!isdigit(fdStr[i])) {
            return false;
        }
    }
    return true;
}

static string GetCallingPkgName()
{
    uint32_t pid = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::HapTokenInfo tokenInfo = Security::AccessToken::HapTokenInfo();
    Security::AccessToken::AccessTokenKit::GetHapTokenInfo(pid, tokenInfo);
    return tokenInfo.bundleName;
}

void RemoteUri::RemoveFd(int fd)
{
    auto iter = fdFromBinder.find(fd);
    if (iter != fdFromBinder.end()) {
        fdFromBinder.erase(iter);
    }
}

bool RemoteUri::IsRemoteUri(const string& path, int &fd, const int& flags)
{
    string::size_type posDatashare = path.find(SCHEME_TAG);
    string::size_type posFragment = path.find(FRAGMENT_TAG);
    string::size_type posFd = path.find(FD_TAG);
    if (posDatashare == string::npos || posFragment == string::npos ||
        posFd == string::npos) {
        return false;
    }
   
    string scheme = path.substr(0, posDatashare);
    if (scheme != SCHEME) {
        return false;
    }

    string fragment = path.substr(posFragment + 1, REMOTE_URI_TAG.size());
    if (fragment == REMOTE_URI_TAG) {
        string fdStr = path.substr(posFd + 1);
        if (IsAllDigits(fdStr)) {
            fd = stoi(fdStr.c_str());
            if (fd < 0 || flags != O_RDONLY) {
                fd = -1;
            }
            RemoveFd(fd);
            return true;
        }
        fd = -1;
        return true;
    }
    return false;
}

int RemoteUri::ConvertUri(const int &fd, string &remoteUri)
{
    if (fd < 0) {
        return -EINVAL;
    }

    if (fdFromBinder.size() == MAX_URI_SIZE) {
        close(*fdFromBinder.begin());
        fdFromBinder.erase(fdFromBinder.begin());
    }
    fdFromBinder.emplace(fd);
    
    string pkgName = GetCallingPkgName();
    remoteUri = SCHEME + ":///" + pkgName + "/" + FRAGMENT_TAG +
                            REMOTE_URI_TAG + FD_TAG + to_string(fd);
    return 0;
}

int RemoteUri::OpenRemoteUri(const string &remoteUri)
{
    int fd = -1;
    (void)IsRemoteUri(remoteUri, fd);
    
    return fd;
}
} // namespace ModuleRemoteUri
} // namespace DistributedFS
} // namespace OHOS
