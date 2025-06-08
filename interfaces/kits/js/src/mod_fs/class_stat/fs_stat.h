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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_FS_STAT_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_FS_STAT_H

#include "filemgmt_libfs.h"
#include "fs_stat_entity.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

const int64_t SECOND_TO_NANOSECOND = 1e9;
constexpr int S_PERMISSION = 00000777;
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
const size_t MAX_ATTR_NAME = 64;
const std::string CLOUD_LOCATION_ATTR = "user.cloud.location";
#endif
class FsStat final {
public:
    static FsStat *Constructor();

    StatEntity *GetStatEntity() const
    {
        return entity.get();
    }

    bool IsBlockDevice();
    bool IsCharacterDevice();
    bool IsDirectory();
    bool IsFIFO();
    bool IsFile();
    bool IsSocket();
    bool IsSymbolicLink();

    int64_t GetIno();
    int64_t GetMode();
    int64_t GetUid();
    int64_t GetGid();
    int64_t GetSize();
    int64_t GetAtime();
    int64_t GetMtime();
    int64_t GetCtime();
    int64_t GetAtimeNs();
    int64_t GetMtimeNs();
    int64_t GetCtimeNs();
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
    int32_t GetLocation();
#endif

private:
    unique_ptr<StatEntity> entity;
    bool CheckStatMode(mode_t mode);
    explicit FsStat(unique_ptr<StatEntity> entity) : entity(move(entity)) {};
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STAT_FS_STAT_H