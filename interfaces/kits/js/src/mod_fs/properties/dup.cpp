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
#include "dup.h"

#include <memory>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

#include "class_file/file_entity.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

napi_value Dup::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [succ, srcFd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!succ || srcFd < 0) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int dstFd = dup(srcFd);
    if (dstFd < 0) {
        HILOGE("Failed to dup fd, errno: %{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> readlink_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };
    if (!readlink_req) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }
    string path = "/proc/self/fd/" + to_string(dstFd);
    int ret = uv_fs_readlink(nullptr, readlink_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to readlink fd, ret: %{public}d", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return CommonFunc::InstantiateFile(env, dstFd, string(static_cast<const char *>(readlink_req->ptr)), false).val_;
}
} // namespace OHOS::FileManagement::ModuleFileIO