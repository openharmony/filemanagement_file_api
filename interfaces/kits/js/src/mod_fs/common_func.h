/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_COMMON_FUNC_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_COMMON_FUNC_H

#include "fd_guard.h"
#include "n_val.h"
#include "uv.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

constexpr int RDONLY = UV_FS_O_RDONLY;
constexpr int WRONLY = UV_FS_O_WRONLY;
constexpr int RDWR = UV_FS_O_RDWR;
constexpr int CREATE = UV_FS_O_CREAT;
constexpr int TRUNC = UV_FS_O_TRUNC;
constexpr int APPEND = UV_FS_O_APPEND;
constexpr int NONBLOCK = UV_FS_O_NONBLOCK;
constexpr int DIRECTORY = UV_FS_O_DIRECTORY;
constexpr int NOFOLLOW = UV_FS_O_NOFOLLOW;
constexpr int SYNC = UV_FS_O_SYNC;

constexpr unsigned int MODE_EXIST = 00;
constexpr unsigned int MODE_WRITE = 02;
constexpr unsigned int MODE_READ = 04;
constexpr unsigned int MODE_READ_WRITE = 06;

constexpr unsigned int USR_READ_ONLY = 00;
constexpr unsigned int USR_WRITE_ONLY = 01;
constexpr unsigned int USR_RDWR = 02;
constexpr unsigned int USR_CREATE = 0100;
constexpr unsigned int USR_TRUNC = 01000;
constexpr unsigned int USR_APPEND = 02000;
constexpr unsigned int USR_NONBLOCK = 04000;
constexpr unsigned int USR_DIRECTORY = 0200000;
constexpr unsigned int USR_NOFOLLOW = 0400000;
constexpr unsigned int USR_SYNC = 04010000;

const double NS = 1e9;
const double MS = 1e3;

struct FileInfo {
    bool isPath = false;
    std::unique_ptr<char[]> path = { nullptr };
    std::unique_ptr<DistributedFS::FDGuard> fdg = { nullptr };
};

void InitAccessModeType(napi_env env, napi_value exports);
void InitOpenMode(napi_env env, napi_value exports);
void InitWhenceType(napi_env env, napi_value exports);

struct CommonFunc {
    static unsigned int ConvertJsFlags(unsigned int &flags);
    static LibN::NVal InstantiateStat(napi_env env, const uv_stat_t &buf);
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    static LibN::NVal InstantiateStat(napi_env env, const uv_stat_t &buf, std::shared_ptr<FileInfo> fileInfo);
#endif
#ifndef WIN_PLATFORM
    static LibN::NVal InstantiateFile(napi_env env, int fd, const std::string &pathOrUri, bool isUri);
    static LibN::NVal InstantiateStream(napi_env env, std::unique_ptr<FILE, decltype(&fclose)> fp);
#endif
    static std::tuple<bool, void *, size_t, int64_t> GetReadArg(napi_env env,
                                                                       napi_value readBuf,
                                                                       napi_value option);
    static std::tuple<bool, std::unique_ptr<char[]>, void *, size_t, int64_t> GetWriteArg(napi_env env,
                                                                                                 napi_value argWBuf,
                                                                                                 napi_value argOption);
    static std::tuple<bool, std::unique_ptr<char[]>, std::unique_ptr<char[]>> GetCopyPathArg(napi_env env,
                                                                                             napi_value srcPath,
                                                                                             napi_value dstPath);
    static void fs_req_cleanup(uv_fs_t* req);
    static std::string GetModeFromFlags(unsigned int flags);
    static bool CheckPublicDirPath(const std::string &sandboxPath);
    static std::string Decode(const std::string &uri);
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_COMMON_FUNC_H