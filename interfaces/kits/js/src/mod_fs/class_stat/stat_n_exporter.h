/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;

constexpr int S_PERMISSION = 00000777;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
const size_t MAX_ATTR_NAME = 64;
const std::string CLOUD_LOCATION_ATTR = "user.cloud.location";
#endif
class StatNExporter final : public NExporter {
public:
    inline static const std::string className_ = "Stat";

    bool Export() override;
#ifdef WIN_PLATFORM
    inline static const int S_IFSOCK = 0140000;
    std::string GetNExporterName() override;
#else
    std::string GetClassName() override;
#endif

    static napi_value Constructor(napi_env env, napi_callback_info info);

    static napi_value IsBlockDevice(napi_env env, napi_callback_info info);
    static napi_value IsCharacterDevice(napi_env env, napi_callback_info info);
    static napi_value IsDirectory(napi_env env, napi_callback_info info);
    static napi_value IsFIFO(napi_env env, napi_callback_info info);
    static napi_value IsFile(napi_env env, napi_callback_info info);
    static napi_value IsSocket(napi_env env, napi_callback_info info);
    static napi_value IsSymbolicLink(napi_env env, napi_callback_info info);

    static napi_value GetIno(napi_env env, napi_callback_info info);
    static napi_value GetMode(napi_env env, napi_callback_info info);
    static napi_value GetUid(napi_env env, napi_callback_info info);
    static napi_value GetGid(napi_env env, napi_callback_info info);
    static napi_value GetSize(napi_env env, napi_callback_info info);
    static napi_value GetAtime(napi_env env, napi_callback_info info);
    static napi_value GetMtime(napi_env env, napi_callback_info info);
    static napi_value GetCtime(napi_env env, napi_callback_info info);
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    static napi_value GetLocation(napi_env env, napi_callback_info info);
#endif

    StatNExporter(napi_env env, napi_value exports);
    ~StatNExporter() override;
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_N_EXPORTER_H