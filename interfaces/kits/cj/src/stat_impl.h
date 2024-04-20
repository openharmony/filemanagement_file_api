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

#ifndef OHOS_FILE_FS_STAT_IMPL_H
#define OHOS_FILE_FS_STAT_IMPL_H

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "fd_guard.h"
#include "utils.h"
#include "uv.h"

#include <sys/stat.h>
#include <uv.h>

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

class StatImpl : public OHOS::FFI::FFIData {
public:
    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }

    explicit StatImpl(uv_stat_t stat) : real_(std::move(stat)) {}

    inline int64_t GetIno() const
    {
        return static_cast<int64_t>(real_.st_ino);
    }

    inline int64_t GetMode() const
    {
        return static_cast<int64_t>(real_.st_mode);
    }

    inline int64_t GetUid() const
    {
        return static_cast<int64_t>(real_.st_uid);
    }

    inline int64_t GetGid() const
    {
        return static_cast<int64_t>(real_.st_gid);
    }

    inline int64_t GetSize() const
    {
        return static_cast<int64_t>(real_.st_size);
    }

    inline int64_t GetAtime() const
    {
        return static_cast<int64_t>(real_.st_atim.tv_sec);
    }

    inline int64_t GetMtime() const
    {
        return static_cast<int64_t>(real_.st_mtim.tv_sec);
    }

    inline int64_t GetCtime() const
    {
        return static_cast<int64_t>(real_.st_ctim.tv_sec);
    }

    bool IsBlockDevice()
    {
        return CheckStatMode(S_IFBLK);
    }

    bool IsCharacterDevice()
    {
        return CheckStatMode(S_IFCHR);
    }

    bool IsDirectory()
    {
        return CheckStatMode(S_IFDIR);
    }

    bool IsFIFO()
    {
        return CheckStatMode(S_IFIFO);
    }

    bool IsFile()
    {
        return CheckStatMode(S_IFREG);
    }

    bool IsSocket()
    {
        return CheckStatMode(S_IFSOCK);
    }

    bool IsSymbolicLink()
    {
        return CheckStatMode(S_IFLNK);
    }

    bool CheckStatMode(mode_t mode)
    {
        bool check = (real_.st_mode & S_IFMT) == mode;
        return check;
    }

private:
    uv_stat_t real_;

    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType = OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("StatImpl");
        return &runtimeType;
    }
};

}
}
}
#endif // OHOS_FILE_FS_STAT_IMPL_H