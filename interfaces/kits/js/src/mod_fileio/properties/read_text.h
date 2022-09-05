/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_PROPERTIES_READ_TEXT_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_PROPERTIES_READ_TEXT_H

#include "../../common/napi/n_async/n_ref.h"
#include "../../common/napi/n_val.h"
#include "../../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
struct AsyncReadTextArg {
    NRef _refReadBuf;
    std::unique_ptr<char[]> buf;
    ssize_t len = 0;
    explicit AsyncReadTextArg(NVal refReadBuf) : _refReadBuf(refReadBuf) {};
    ~AsyncReadTextArg() = default;
};

class ReadText final {
public:
static UniError AsyncExec(const std::string &path, std::shared_ptr<AsyncReadTextArg> arg, ssize_t position,
        bool hasLen, ssize_t len);
    static napi_value Async(napi_env env, napi_callback_info info);
    static napi_value Sync(napi_env env, napi_callback_info info);
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif