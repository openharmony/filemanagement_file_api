/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_RANDOMACCESSFILE_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_RANDOMACCESSFILE_RANDOMACCESSFILE_N_EXPORTER_H

#include "filemgmt_libhilog.h"
#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;

class RandomAccessFileNExporter final : public NExporter {
public:
    inline static const std::string className_ = "RandomAccessFile";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);

    static napi_value SetFilePointerSync(napi_env env, napi_callback_info info);
    static napi_value WriteSync(napi_env env, napi_callback_info info);
    static napi_value ReadSync(napi_env env, napi_callback_info info);
    static napi_value CloseSync(napi_env env, napi_callback_info info);

    static napi_value Write(napi_env env, napi_callback_info info);
    static napi_value Read(napi_env env, napi_callback_info info);

    static napi_value GetReadStream(napi_env env, napi_callback_info info);
    static napi_value GetWriteStream(napi_env env, napi_callback_info info);

    static napi_value GetFD(napi_env env, napi_callback_info info);
    static napi_value GetFPointer(napi_env env, napi_callback_info info);

    RandomAccessFileNExporter(napi_env env, napi_value exports);
    ~RandomAccessFileNExporter() override;
};
const std::string readProcedureName = "FileIORandomAccessFileRead";
const std::string writeProcedureName = "FileIORandomAccessFileWrite";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif