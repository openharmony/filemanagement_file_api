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

#ifndef OHOS_FILE_FS_IMPL_H
#define OHOS_FILE_FS_IMPL_H

#include <filesystem>
#include <deque>
#include <string_view>

#include "stat_impl.h"
#include "stream_impl.h"
#include "macro.h"
#include "randomAccessFile_impl.h"
#include "readerIterator_impl.h"
#include "watcher_impl.h"
#include "file_impl.h"
#include "copy_dir.h"
#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "uni_error.h"
#include "rust_file.h"
#include "napi/native_api.h"

namespace OHOS {
namespace CJSystemapi {
constexpr int DIR_DEFAULT_PERM = 0770;

constexpr int FILE_DISMATCH = 0;
constexpr int FILE_MATCH = 1;

// use for moveDir
constexpr int DIRMODE_MIN = 0;
constexpr int DIRMODE_MAX = 3;

struct StreamEntity {
    std::unique_ptr<FILE, decltype(&fclose)> fp = { nullptr, fclose };
};

enum ModeOfMoveDir {
    DIRMODE_DIRECTORY_THROW_ERR = 0,
    DIRMODE_FILE_THROW_ERR,
    DIRMODE_FILE_REPLACE,
    DIRMODE_DIRECTORY_REPLACE
};
struct FileStruct {
    bool isFd = false;
    int fd;
    FileFs::FileEntity *fileEntity;
};
namespace FileFs {
class FileFsImpl {
public:
    static std::tuple<int32_t, sptr<StatImpl>> Stat(int32_t file);
    static std::tuple<int32_t, sptr<StatImpl>> Stat(std::string file);
    static std::tuple<int32_t, sptr<StreamImpl>> CreateStream(std::string path, std::string mode);
    static std::tuple<int32_t, sptr<StreamImpl>> FdopenStream(int32_t, std::string mode);
    static std::tuple<int32_t, sptr<StatImpl>> Lstat(std::string path);
    static std::tuple<int32_t, sptr<RandomAccessFileImpl>> CreateRandomAccessFileSync(std::string file,
            unsigned int mode);
    static std::tuple<int32_t, sptr<RandomAccessFileImpl>> CreateRandomAccessFileSync(sptr<FileEntity> entity,
            unsigned int mode);
    static int Mkdir(std::string path, bool recursion, bool isTwoArgs);
    static int Rmdir(std::string path);
    static int Rename(std::string oldPath, std::string newPath);
    static int Unlink(std::string path);
    static RetDataCArrConflictFiles MoveDir(std::string src, std::string dest, int32_t mode);
    static RetDataI64 Read(int32_t fd, char* buf, int64_t bufLen, size_t length, int64_t offset);
    static RetDataI64 ReadCur(int32_t fd, char* buf, int64_t bufLen, size_t length);
    static RetDataI64 Write(int32_t fd, void* buf, size_t length, int64_t offset, std::string encode);
    static RetDataI64 WriteCur(int32_t fd, void* buf, size_t length, std::string encode);
    static std::tuple<int32_t, bool> Access(std::string path);
    static int Truncate(std::string file, int64_t len);
    static int Truncate(int32_t file, int64_t len);
    static int Close(int32_t file);
    static int Close(sptr<OHOS::CJSystemapi::FileFs::FileEntity> file);
    static std::tuple<int32_t, sptr<ReadIteratorImpl>> ReadLines(char* file, std::string encoding);
    static RetDataCString ReadText(char* path, int64_t offset, bool hasLen, int64_t len, char* encoding);
    static int Utimes(std::string path, double mtime);
    static std::tuple<int32_t, sptr<WatcherImpl>> CreateWatcher(std::string path, uint32_t events,
            void (*callback)(CWatchEvent));
};
} // OHOS::FileManagement::ModuleFileIO
}
}

#endif // OHOS_FILE_FS_IMPL_H