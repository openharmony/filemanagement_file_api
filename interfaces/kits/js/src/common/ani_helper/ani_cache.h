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

#ifndef INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_CACHE_H
#define INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_CACHE_H

#pragma once

#include <ani.h>
#include <map>
#include <mutex>
#include <string>

#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO::ANI {
using namespace std;
class AniCache {
public:
    static AniCache& GetInstance();
    tuple<ani_status, ani_class> GetClass(ani_env *env, const string &name);
    tuple<ani_status, ani_enum> GetEnum(ani_env *env, const string &name);
    tuple<ani_status, ani_method> GetMethod(ani_env *env, const string &clazzName, const string &methodName,
        const string& methodSignature);
    tuple<ani_status, ani_enum_item> GetEnumIndex(ani_env *env, const string &enumName, int index);
    AniCache(const AniCache&) = delete;
    AniCache& operator=(const AniCache&) = delete;
private:
    AniCache() noexcept
    {
        HILOGI("create AniCache");
    }

    map<string, ani_ref> clazzMap;
    std::mutex exClassMapLock;
};

} // OHOS::FileManagement::ModuleFileIO::ANI

#endif // INTERFACES_KITS_JS_SRC_COMMON_ANI_HELPER_ANI_CACHE_H