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

#include <disconnectdfs_core.h>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include "filemgmt_libhilog.h"
#include "distributed_file_daemon_manager.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace fs = std::filesystem;

FsResult<void> DisConnectDfsCore::DisConnectDfsExec(const std::string &networkId)
{
    if (networkId == "") {
        return FsResult<void>::Error(E_PARAMS);
    }

    int result = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().
            CloseP2PConnectionEx(networkId);
    if (result != ERRNO_NOERR) {
        HILOGE("Fail to Closep2pconnection");
        return FsResult<void>::Error(result);
    }
    return FsResult<void>::Success();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS