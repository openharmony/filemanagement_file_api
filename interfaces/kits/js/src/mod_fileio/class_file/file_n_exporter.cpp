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

#include "file_n_exporter.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "../../common/log.h"
#include "../../common/napi/n_async/n_async_work_callback.h"
#include "../../common/napi/n_async/n_async_work_promise.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"
#include "../common_func.h"
#include "file_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static FileEntity *GetFileEntity(napi_env env, napi_value raf_entity)
{
    auto rafEntity = NClass::GetEntityOf<FileEntity>(env, raf_entity);
    if (!rafEntity) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    if (!rafEntity->fd_) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    return rafEntity;
}

napi_value FileNExporter::GetFD(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }
    auto rafEntity = GetFileEntity(env, funcArg.GetThisVar());
    if (!rafEntity) {
        return nullptr;
    }
    return NVal::CreateInt32(env, rafEntity->fd_.get()->GetFD()).val_;
}

napi_value FileNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL, true).ThrowErr(env);
        return nullptr;
    }

    auto rafEntity = make_unique<FileEntity>();
    if (!NClass::SetEntityFor<FileEntity>(env, funcArg.GetThisVar(), move(rafEntity))) {
        UniError(EIO, true).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool FileNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiGetter("fd", GetFD),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className,
        FileNExporter::Constructor, move(props));
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

string FileNExporter::GetClassName()
{
    return FileNExporter::className_;
}

FileNExporter::FileNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}
FileNExporter::~FileNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS