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

#ifndef OHOS_FILE_FS_READERITERATOR_ENTITY_H
#define OHOS_FILE_FS_READERITERATOR_ENTITY_H

#include "macro.h"
#include "ffi_remote_data.h"
#include "rust_file.h"
#include "uni_error.h"
#include "securec.h"
#include "cj_common_ffi.h"
#include "readeriterator_entity.h"

namespace OHOS {
namespace CJSystemapi {
class ReadIteratorImpl : public OHOS::FFI::FFIData {
public:
    explicit ReadIteratorImpl(std::shared_ptr<OHOS::FileManagement::ModuleFileIO::ReaderIteratorEntity> entity);
    std::tuple<int32_t, bool, char*> Next();

    OHOS::FFI::RuntimeType* GetRuntimeType() override { return GetClassType(); }
private:
    std::shared_ptr<OHOS::FileManagement::ModuleFileIO::ReaderIteratorEntity> entity_;

    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType =
            OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("ReadIteratorImpl");
        return &runtimeType;
    }
};
}
}

#endif // OHOS_FILE_FS_READERITERATOR_ENTITY_H