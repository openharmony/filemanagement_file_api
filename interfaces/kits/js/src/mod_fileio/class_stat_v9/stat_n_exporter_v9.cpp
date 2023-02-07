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

#include "stat_n_exporter_v9.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>

#include "securec.h"

#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "stat_entity_v9.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

constexpr int S_PERMISSION = 00000777;

static napi_value CheckStatMode(napi_env env, napi_callback_info info, mode_t mode)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    bool check = (statEntity->stat_.st_mode & S_IFMT) == mode;
    return NVal::CreateBool(env, check).val_;
}

napi_value StatNExporterV9::IsBlockDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFBLK);
}

napi_value StatNExporterV9::IsCharacterDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFCHR);
}

napi_value StatNExporterV9::IsDirectory(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFDIR);
}

napi_value StatNExporterV9::IsFIFO(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFIFO);
}

napi_value StatNExporterV9::IsFile(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFREG);
}

napi_value StatNExporterV9::IsSocket(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFSOCK);
}

napi_value StatNExporterV9::IsSymbolicLink(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFLNK);
}

napi_value StatNExporterV9::GetIno(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_ino).val_;
}

napi_value StatNExporterV9::GetMode(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_mode & S_PERMISSION).val_;
}

napi_value StatNExporterV9::GetUid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_uid).val_;
}

napi_value StatNExporterV9::GetGid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_gid).val_;
}

napi_value StatNExporterV9::GetSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_size).val_;
}

napi_value StatNExporterV9::GetBlksize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_blksize).val_;
}

napi_value StatNExporterV9::GetAtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_atim.tv_sec).val_;
}

napi_value StatNExporterV9::GetMtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_mtim.tv_sec).val_;
}

napi_value StatNExporterV9::GetCtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntityV9>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_ctim.tv_sec).val_;
}

napi_value StatNExporterV9::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    unique_ptr<StatEntityV9> statEntity = make_unique<StatEntityV9>();
    if (!NClass::SetEntityFor<StatEntityV9>(env, funcArg.GetThisVar(), move(statEntity))) {
        UniError(EIO, true).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool StatNExporterV9::Export()
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
        NVal::DeclareNapiGetter("blksize", GetBlksize),
        NVal::DeclareNapiGetter("atime", GetAtime),
        NVal::DeclareNapiGetter("mtime", GetMtime),
        NVal::DeclareNapiGetter("ctime", GetCtime),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, StatNExporterV9::Constructor,
        std::move(props));
    if (!succ) {
        UniError(EIO, true).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO, true).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string StatNExporterV9::GetClassName()
{
    return StatNExporterV9::className_;
}

StatNExporterV9::StatNExporterV9(napi_env env, napi_value exports) : NExporter(env, exports) {}

StatNExporterV9::~StatNExporterV9() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS