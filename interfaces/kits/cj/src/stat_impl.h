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

#include "cj_file_utils.h"
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

constexpr int S_PREMISSION = 00000777;

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
const size_t MAX_ATTR_NAME = 64;
const std::string CLOUD_LOCATION_ATTR = "user.cloud.location";
enum Location {
    LOCAL = 1 << 0,
    CLOUD = 1 << 1
};
#endif

class StatImpl : public OHOS::FFI::FFIData {
public:
    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }

    explicit StatImpl(uv_stat_t stat) : real_(std::move(stat)) {}
#ifdef WIN_PLATFORM
    inline static const int S_IFSOCK = 0140000;
#endif
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    RetDataI32 GetLocation();

    void SetFileInfo(std::shared_ptr<FileInfo> info)
    {
        fileInfo_ = info;
    }
#endif

    int64_t GetIno() const;
    int64_t GetMode() const;
    int64_t GetUid() const;
    int64_t GetGid() const;
    int64_t GetSize() const;
    int64_t GetAtime() const;
    int64_t GetMtime() const;
    int64_t GetCtime() const;
    bool IsBlockDevice();
    bool IsCharacterDevice();
    bool IsDirectory();
    bool IsFIFO();
    bool IsFile();
    bool IsSocket();
    bool IsSymbolicLink();
    bool CheckStatMode(mode_t mode);

private:
    uv_stat_t real_;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    std::shared_ptr<FileInfo> fileInfo_ = nullptr;
#endif

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