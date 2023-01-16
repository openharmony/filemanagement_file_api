/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "stat_entity.h"
#include "stat_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <securec.h>

#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static napi_value CheckStatMode(napi_env env, napi_callback_info info, mode_t mode)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    bool check = (statEntity->stat_.st_mode & S_IFMT) == mode;
    return NVal::CreateBool(env, check).val_;
}

napi_value StatNExporter::IsBlockDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFBLK);
}

napi_value StatNExporter::IsCharacterDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFCHR);
}

napi_value StatNExporter::IsDirectory(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFDIR);
}

napi_value StatNExporter::IsFIFO(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFIFO);
}

napi_value StatNExporter::IsFile(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFREG);
}

napi_value StatNExporter::IsSocket(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFSOCK);
}

napi_value StatNExporter::IsSymbolicLink(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFLNK);
}

napi_value StatNExporter::GetIno(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateBigInt64(env, statEntity->stat_.st_ino).val_;
}

napi_value StatNExporter::GetMode(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_mode & S_PERMISSION).val_;
}

napi_value StatNExporter::GetUid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_uid).val_;
}

napi_value StatNExporter::GetGid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_gid).val_;
}

napi_value StatNExporter::GetSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_size).val_;
}

napi_value StatNExporter::GetAtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_atim.tv_sec).val_;
}

napi_value StatNExporter::GetMtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_mtim.tv_sec).val_;
}

napi_value StatNExporter::GetCtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        HILOGE("Failed to get stat entity");
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_ctim.tv_sec).val_;
}

napi_value StatNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<StatEntity> statEntity = make_unique<StatEntity>();
    if (!NClass::SetEntityFor<StatEntity>(env, funcArg.GetThisVar(), move(statEntity))) {
        HILOGE("Failed to set stat entity");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool StatNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("isBlockDevice", IsBlockDevice),
        NVal::DeclareNapiFunction("isCharacterDevice", IsCharacterDevice),
        NVal::DeclareNapiFunction("isDirectory", IsDirectory),
        NVal::DeclareNapiFunction("isFIFO", IsFIFO),
        NVal::DeclareNapiFunction("isFile", IsFile),
        NVal::DeclareNapiFunction("isSocket", IsSocket),
        NVal::DeclareNapiFunction("isSymbolicLink", IsSymbolicLink),

        NVal::DeclareNapiGetter("ino", GetIno),
        NVal::DeclareNapiGetter("mode", GetMode),
        NVal::DeclareNapiGetter("uid", GetUid),
        NVal::DeclareNapiGetter("gid", GetGid),
        NVal::DeclareNapiGetter("size", GetSize),
        NVal::DeclareNapiGetter("atime", GetAtime),
        NVal::DeclareNapiGetter("mtime", GetMtime),
        NVal::DeclareNapiGetter("ctime", GetCtime),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, StatNExporter::Constructor,
        std::move(props));
    if (!succ) {
        HILOGE("Failed to define class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        HILOGE("Failed to save class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string StatNExporter::GetClassName()
{
    return StatNExporter::className_;
}

StatNExporter::StatNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

StatNExporter::~StatNExporter() {}
} // namespace OHOS::FileManagement::ModuleFileIO