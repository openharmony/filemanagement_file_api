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

#include "fs_stat.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
#include <sys/xattr.h>
#endif

#include "file_fs_trace.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

bool FsStat::CheckStatMode(mode_t mode)
{
    if (FileApiDebug::isLogEnabled) {
        HILOGD("Mode is %{public}lo", static_cast<unsigned long>(entity->stat_.st_mode & S_IFMT));
    }
    return (entity->stat_.st_mode & S_IFMT) == mode;
}

bool FsStat::IsBlockDevice()
{
    return CheckStatMode(S_IFBLK);
}

bool FsStat::IsCharacterDevice()
{
    return CheckStatMode(S_IFCHR);
}

bool FsStat::IsDirectory()
{
    return CheckStatMode(S_IFDIR);
}

bool FsStat::IsFIFO()
{
    return CheckStatMode(S_IFIFO);
}

bool FsStat::IsFile()
{
    return CheckStatMode(S_IFREG);
}

bool FsStat::IsSocket()
{
    return CheckStatMode(S_IFSOCK);
}

bool FsStat::IsSymbolicLink()
{
    return CheckStatMode(S_IFLNK);
}

int64_t FsStat::GetIno()
{
    return entity->stat_.st_ino;
}

int64_t FsStat::GetMode()
{
    return entity->stat_.st_mode & S_PERMISSION;
}

int64_t FsStat::GetUid()
{
    return entity->stat_.st_uid;
}

int64_t FsStat::GetGid()
{
    return entity->stat_.st_gid;
}

int64_t FsStat::GetSize()
{
    return entity->stat_.st_size;
}

int64_t FsStat::GetAtime()
{
    return static_cast<int64_t>(entity->stat_.st_atim.tv_sec);
}

int64_t FsStat::GetMtime()
{
    return static_cast<int64_t>(entity->stat_.st_mtim.tv_sec);
}

int64_t FsStat::GetCtime()
{
    return static_cast<int64_t>(entity->stat_.st_ctim.tv_sec);
}

int64_t FsStat::GetAtimeNs()
{
    return static_cast<uint64_t>(entity->stat_.st_atim.tv_sec * SECOND_TO_NANOSECOND + entity->stat_.st_atim.tv_nsec);
}

int64_t FsStat::GetMtimeNs()
{
    return static_cast<uint64_t>(entity->stat_.st_mtim.tv_sec * SECOND_TO_NANOSECOND + entity->stat_.st_mtim.tv_nsec);
}

int64_t FsStat::GetCtimeNs()
{
    return static_cast<uint64_t>(entity->stat_.st_ctim.tv_sec * SECOND_TO_NANOSECOND + entity->stat_.st_ctim.tv_nsec);
}

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
int32_t FsStat::GetLocation()
{
    std::unique_ptr<char[]> value = CreateUniquePtr<char[]>(MAX_ATTR_NAME);
    if (value == nullptr) {
        HILOGE("Getxattr memory out, errno is %{public}d", errno);
        return ENOMEM;
    }

    ssize_t size = 0;
    if (entity->fileInfo_->isPath) {
        size = getxattr(entity->fileInfo_->path.get(), CLOUD_LOCATION_ATTR.c_str(), value.get(), MAX_ATTR_NAME);
    } else {
        size = fgetxattr(entity->fileInfo_->fdg->GetFD(), CLOUD_LOCATION_ATTR.c_str(), value.get(), MAX_ATTR_NAME);
    }

    Location defaultLocation = LOCAL;
    if (size <= 0) {
        if (errno != ENODATA && errno != EOPNOTSUPP) {
            HILOGE("Getxattr value failed, errno is %{public}d", errno);
        }
        return static_cast<int32_t>(defaultLocation);
    }
    std::string location = string(value.get(), static_cast<size_t>(size));
    if (!std::all_of(location.begin(), location.end(), ::isdigit)) {
        HILOGE("Getxattr location is not all digit!");
        return static_cast<int32_t>(defaultLocation);
    }
    defaultLocation = static_cast<Location>(atoi(location.c_str()));
    return static_cast<int32_t>(defaultLocation);
}
#endif

FsStat *FsStat::Constructor()
{
    auto entity = CreateUniquePtr<StatEntity>();
    if (entity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return nullptr;
    }

    return new FsStat(move(entity));
}

} // namespace OHOS::FileManagement::ModuleFileIO