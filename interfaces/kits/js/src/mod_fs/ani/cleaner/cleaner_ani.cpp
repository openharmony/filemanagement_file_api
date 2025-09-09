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

#include "cleaner_ani.h"

#include "filemgmt_libhilog.h"
#include "fs_atomicfile.h"
#include "fs_file.h"
#include "fs_randomaccessfile.h"
#include "fs_reader_iterator.h"
#include "fs_stat.h"
#include "fs_stream.h"
#include "fs_watcher.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;
using namespace OHOS::FileManagement::ModuleFileIO::ANI;

static void DoCleanByClassType(ani_env *env, ani_long ptr, ani_ref classTypeObj)
{
    auto [succ, classType] = TypeConverter::ToUTF8String(env, static_cast<ani_string>(classTypeObj));
    if (!succ) {
        HILOGE("Clean ParseString failed.");
        return;
    }
    if (classType == "File") {
        delete reinterpret_cast<FsFile *>(ptr);
    } else if (classType == "RandomAccessFile") {
        delete reinterpret_cast<FsRandomAccessFile *>(ptr);
    } else if (classType == "ReaderIterator") {
        delete reinterpret_cast<FsReaderIterator *>(ptr);
    } else if (classType == "Stat") {
        delete reinterpret_cast<FsStat *>(ptr);
    } else if (classType == "Stream") {
        delete reinterpret_cast<FsStream *>(ptr);
    } else if (classType == "AtomicFile") {
        delete reinterpret_cast<FsAtomicFile *>(ptr);
    } else {
        HILOGE("Clean unsupport className: %{public}s", classType.c_str());
    }
    return;
}

void CleanerAni::Clean(ani_env *env, ani_object object)
{
    if (env == nullptr) {
        HILOGE("env is nullptr");
        return;
    }
    ani_long ptr;
    ani_status status = env->Object_GetFieldByName_Long(object, "ptr", &ptr);
    if (status != ANI_OK) {
        HILOGE("Clean Object_GetFieldByName_Long status: %{public}d", status);
        return;
    }

    ani_ref classTypeObj = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, "classType", &classTypeObj)) != ANI_OK) {
        HILOGE("Clean Object_GetFieldByName_Ref status: %{public}d", status);
        return;
    }

    DoCleanByClassType(env, ptr, classTypeObj);
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS