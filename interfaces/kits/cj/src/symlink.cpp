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

#include "symlink.h"
#include "file_fs_impl.h"

#include <cstring>
#include <fcntl.h>
#include <tuple>
#include <unistd.h>

namespace OHOS {
namespace CJSystemapi {

using namespace std;

int SymlinkImpl::Symlink(const std::string& target, const std::string& srcPath)
{
    LOGI("FS_TEST::SymlinkImpl::Symlink start");

    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> symlink_req = {
        new uv_fs_t, CommonFunc::FsReqCleanup };
    if (!symlink_req) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int ret = uv_fs_symlink(nullptr, symlink_req.get(), target.c_str(), srcPath.c_str(), 0, nullptr);
    if (ret < 0) {
        LOGE("Failed to create a link for old path");
        return ret;
    }

    LOGI("FS_TEST::SymlinkImpl::Symlink success");
    return ret;
}

} // namespace CJSystemapi
} // namespace OHOS
