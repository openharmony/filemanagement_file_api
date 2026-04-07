/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_NAPI_FILEMAPPING_NAPI_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_NAPI_FILEMAPPING_NAPI_H

#include "n_exporter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

using namespace OHOS::FileManagement::LibN;

class FileMappingNapi final : public NExporter {
public:
    inline static const std::string className_ = "FileMapping";

    bool Export() override;

#ifdef WIN_PLATFORM
    std::string GetNExporterName() override;
#else
    std::string GetClassName() override;
#endif

    static napi_value Constructor(napi_env env, napi_callback_info info);

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    static napi_value SetPosition(napi_env env, napi_callback_info info);
    static napi_value GetPosition(napi_env env, napi_callback_info info);
    static napi_value Capacity(napi_env env, napi_callback_info info);
    static napi_value SetLimit(napi_env env, napi_callback_info info);
    static napi_value GetLimit(napi_env env, napi_callback_info info);
    static napi_value Flip(napi_env env, napi_callback_info info);
    static napi_value Remaining(napi_env env, napi_callback_info info);
    static napi_value Read(napi_env env, napi_callback_info info);
    static napi_value Write(napi_env env, napi_callback_info info);
    static napi_value Msync(napi_env env, napi_callback_info info);
    static napi_value MsyncSync(napi_env env, napi_callback_info info);
    static napi_value Unmap(napi_env env, napi_callback_info info);
    static napi_value UnmapSync(napi_env env, napi_callback_info info);
#endif

    FileMappingNapi(napi_env env, napi_value exports);
    ~FileMappingNapi() override;
};

const std::string PROCEDURE_MSYNC_NAME = "fs.FileMapping.Msync";
const std::string PROCEDURE_UNMAP_NAME = "fs.FileMapping.Unmap";

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_FILEMAPPING_NAPI_FILEMAPPING_NAPI_H
