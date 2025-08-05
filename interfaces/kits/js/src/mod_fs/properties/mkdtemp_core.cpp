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

#include "mkdtemp_core.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

FsResult<string> MkdtempCore::DoMkdtemp(const string &path)
{
    unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup) *> mkdtempReq = { new uv_fs_t, FsUtils::FsReqCleanup };
    if (!mkdtempReq) {
        HILOGE("Failed to request heap memory.");
        return FsResult<string>::Error(ENOMEM);
    }

    int ret = uv_fs_mkdtemp(nullptr, mkdtempReq.get(), const_cast<char *>(path.c_str()), nullptr);
    if (ret < 0) {
        HILOGE("Failed to create a temporary directory with path");
        return FsResult<string>::Error(ret);
    }

    return FsResult<string>::Success(move(mkdtempReq->path));
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
