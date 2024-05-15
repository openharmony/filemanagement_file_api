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

#ifndef OHOS_FILE_FS_FILE_IMPL_H
#define OHOS_FILE_FS_FILE_IMPL_H

#include <sys/file.h>
#include <sys/stat.h>
#include "macro.h"
#include "fd_guard.h"
#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include <cinttypes>
#include <string>
#include <memory>

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

class FileEntity : public OHOS::FFI::FFIData {
public:
    std::unique_ptr<DistributedFS::FDGuard> fd_ = { nullptr };
    std::string path_;
    std::string uri_;
    ~FileEntity() override
    {
        if (!fd_.get()) {
            return;
        }
        int32_t fd = fd_.get()->GetFD();
        int ret = flock(fd, LOCK_UN);
        if (ret == 0) {
            struct stat buf;
            if (fstat(fd, &buf) == 0) {
                LOGI("Unlock succeeded inode = %{public}" PRIu64, buf.st_ino);
            } else {
                LOGI("Failed to get inode number");
            }
        }
    }
    FileEntity(std::unique_ptr<DistributedFS::FDGuard> fd_, std::string path_, std::string uri_)
        : fd_(std::move(fd_)), path_(path_), uri_(uri_) {}
    FileEntity() {};
    static std::tuple<int32_t, sptr<FileEntity>> Open(const char* path, int64_t mode);
    static std::tuple<int32_t, sptr<FileEntity>> Dup(int32_t fd);
    static int GetFD(int64_t id);
    static const char* GetPath(int64_t id);
    static const char* GetName(int64_t id);
    int TryLock(int64_t id, bool exclusive);
    int UnLock(int64_t id);
    RetDataCString GetParent();

    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }

private:
    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType = OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("FileEntity");
        return &runtimeType;
    }
};

}
}
}
#endif //OHOS_FILE_FS_FILE_IMPL_H