/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILE_FS_UTILS_H
#define OHOS_FILE_FS_UTILS_H

#include "uv.h"
#include "fd_guard.h"
#include <string>

namespace OHOS {
namespace CJSystemapi {
    
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

struct CommonFunc {
    static unsigned int ConvertCjFlags(unsigned int &flags);
    static void FsReqCleanup(uv_fs_t* req);
    static std::string GetModeFromFlags(unsigned int flags);
};

struct ConflictFiles {
    std::string srcFiles;
    std::string destFiles;
    ConflictFiles(const std::string& src, const std::string& dest) : srcFiles(src), destFiles(dest) {}
    ~ConflictFiles() = default;
};

struct CConflictFiles {
    char* srcFiles;
    char* destFiles;
};

struct CArrConflictFiles {
    CConflictFiles* head;
    int64_t size;
};

struct RetDataCArrConflictFiles {
    int code;
    CArrConflictFiles data;
};
}
}

#endif // OHOS_FILE_FS_UTILS_H