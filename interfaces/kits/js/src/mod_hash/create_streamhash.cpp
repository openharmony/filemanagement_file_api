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

#include "create_streamhash.h"

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
const string HASH_STREAM_CLASS = "HashStream";

napi_value CreateStreamHash::Hash(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    const char moduleName[] = "@ohos.file.streamhash";
    napi_value streamhash;
    napi_load_module(env, moduleName, &streamhash);
    napi_value constructor = nullptr;
    napi_get_named_property(env, streamhash, HASH_STREAM_CLASS.c_str(), &constructor);
    napi_value streamObj = nullptr;

    napi_value argv[NARG_CNT::ONE] = {funcArg[NARG_POS::FIRST]};
    napi_status status = napi_new_instance(env, constructor, NARG_CNT::ONE, argv, &streamObj);
    if (status != napi_ok) {
        HILOGE("create stream obj fail");
        return nullptr;
    }
    return NVal(env, streamObj).val_;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS