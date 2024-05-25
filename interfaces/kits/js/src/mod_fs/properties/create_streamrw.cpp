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

#include "create_streamrw.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
const string READ_STREAM_CLASS = "ReadStream";
const string WRITE_STREAM_CLASS = "WriteStream";

static napi_value CreateStream(napi_env env, napi_callback_info info, const string &streamName)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    size_t argc = funcArg.GetArgc();
    const char moduleName[] = "@ohos.file.streamrw";
    napi_value streamrw;
    napi_load_module(env, moduleName, &streamrw);
    napi_value constructor = nullptr;
    napi_get_named_property(env, streamrw, streamName.c_str(), &constructor);
    napi_value streamObj = nullptr;
    napi_status status;
    if (argc == NARG_CNT::ONE) {
        napi_value argv[NARG_CNT::ONE] = {funcArg[NARG_POS::FIRST]};
        status = napi_new_instance(env, constructor, argc, argv, &streamObj);
    } else {
        napi_value argv[NARG_CNT::TWO] = {funcArg[NARG_POS::FIRST], funcArg[NARG_POS::SECOND]};
        status = napi_new_instance(env, constructor, argc, argv, &streamObj);
    }
    if (status != napi_ok) {
        HILOGE("create stream obj fail");
        return nullptr;
    }
    return NVal(env, streamObj).val_;
}

napi_value CreateStreamRw::Read(napi_env env, napi_callback_info info)
{
    return CreateStream(env, info, READ_STREAM_CLASS);
}

napi_value CreateStreamRw::Write(napi_env env, napi_callback_info info)
{
    return CreateStream(env, info, WRITE_STREAM_CLASS);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS