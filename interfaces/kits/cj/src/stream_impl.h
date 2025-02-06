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

#ifndef OHOS_FILE_FS_STREAM_IMPL_H
#define OHOS_FILE_FS_STREAM_IMPL_H

#include "ffi_remote_data.h"
#include "cj_common_ffi.h"
#include "macro.h"
#include "uni_error.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

std::tuple<int, std::unique_ptr<char[]>, void *, size_t, int64_t> GetWriteArg(const std::string& buffer, int64_t length,
    int64_t offset, const std::string& encode);
std::tuple<int, std::unique_ptr<char[]>, size_t, int64_t> GetReadArg(size_t bufLen, int64_t length, int64_t offset);

class StreamImpl : public OHOS::FFI::FFIData {
public:
    explicit StreamImpl(std::unique_ptr<FILE, decltype(&fclose)> fp) : fp_(std::move(fp)) {}

    int Close();

    int Flush();

    std::tuple<int, int64_t> ReadCur(uint8_t* buffer, size_t buLen, int64_t length);

    std::tuple<int, int64_t> Read(uint8_t* buffer, size_t buLen, int64_t length, int64_t offset);

    std::tuple<int, int64_t> WriteCur(const std::string& buffer, int64_t length, const std::string& encode);

    std::tuple<int, int64_t> Write(const std::string& buffer, int64_t length, int64_t offset,
        const std::string& encode);

    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }

    std::unique_ptr<FILE, decltype(&fclose)> GetRealFp() { return std::move(fp_); }

private:
    std::unique_ptr<FILE, decltype(&fclose)> fp_;

    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType = OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("StreamImpl");
        return &runtimeType;
    }
};

}
}
}
#endif // OHOS_FILE_FS_STREAM_IMPL_H