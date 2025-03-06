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

#ifndef FILEMANAGEMENT_FS_UTILS_H
#define FILEMANAGEMENT_FS_UTILS_H

#include <cstddef>
#include <cstdint>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <optional>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>

#include "fd_guard.h"
#include "uv.h"

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include "iremote_broker.h"
#endif

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

constexpr int32_t RDONLY = UV_FS_O_RDONLY;
constexpr int32_t WRONLY = UV_FS_O_WRONLY;
constexpr int32_t RDWR = UV_FS_O_RDWR;
constexpr int32_t CREATE = UV_FS_O_CREAT;
constexpr int32_t TRUNC = UV_FS_O_TRUNC;
constexpr int32_t APPEND = UV_FS_O_APPEND;
constexpr int32_t NONBLOCK = UV_FS_O_NONBLOCK;
constexpr int32_t DIRECTORY = UV_FS_O_DIRECTORY;
constexpr int32_t NOFOLLOW = UV_FS_O_NOFOLLOW;
constexpr int32_t SYNC = UV_FS_O_SYNC;

constexpr uint32_t MODE_EXIST = 00;
constexpr uint32_t MODE_WRITE = 02;
constexpr uint32_t MODE_READ = 04;
constexpr uint32_t MODE_READ_WRITE = 06;

constexpr uint32_t USR_READ_ONLY = 00;
constexpr uint32_t USR_WRITE_ONLY = 01;
constexpr uint32_t USR_RDWR = 02;
constexpr uint32_t USR_CREATE = 0100;
constexpr uint32_t USR_TRUNC = 01000;
constexpr uint32_t USR_APPEND = 02000;
constexpr uint32_t USR_NONBLOCK = 04000;
constexpr uint32_t USR_DIRECTORY = 0200000;
constexpr uint32_t USR_NOFOLLOW = 0400000;
constexpr uint32_t USR_SYNC = 04010000;

const double NS = 1e9;
const double MS = 1e3;

struct FileInfo {
    bool isPath = false;
    unique_ptr<char[]> path = { nullptr };
    unique_ptr<DistributedFS::FDGuard> fdg = { nullptr };
};

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
class FileIoToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.fileio.open");

    FileIoToken() = default;
    virtual ~FileIoToken() noexcept = default;
};
#endif

class FsUtils {
public:
    static tuple<bool, size_t> GetActualLen(size_t bufLen, size_t bufOff, const optional<int64_t> &length = nullopt);
    static uint32_t ConvertFlags(const uint32_t &flags);
    static void FsReqCleanup(uv_fs_t *req);
    static string GetModeFromFlags(const uint32_t &flags);
    static bool CheckPublicDirPath(const string &sandboxPath);
    static string Decode(const string &uri);
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // FILEMANAGEMENT_FS_UTILS_H