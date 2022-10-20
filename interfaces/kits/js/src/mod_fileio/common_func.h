/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_COMMON_FUNC_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_COMMON_FUNC_H

#include "../common/napi/uni_header.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
constexpr int64_t INVALID_POSITION = std::numeric_limits<decltype(INVALID_POSITION)>::max();
constexpr int RDONLY = 00;
constexpr int WRONLY = 01;
constexpr int RDWR = 02;
constexpr int CREATE = 0100;
constexpr int TRUNC = 01000;
constexpr int APPEND = 02000;
constexpr int NONBLOCK = 04000;
constexpr int DIRECTORY = 0200000;
constexpr int NOFOLLOW = 0400000;
constexpr int SYNC = 04010000;

void InitOpenMode(napi_env env, napi_value exports);

struct CommonFunc {
    static int ConvertJsFlags(int &flags);
    static std::tuple<bool, void *, int64_t, bool, int64_t, int> GetReadArg(napi_env env,
                                                                            napi_value readBuf,
                                                                            napi_value option);
    static std::tuple<bool, void *, int64_t, bool, int64_t> GetReadArgV9(napi_env env,
                                                                         napi_value readBuf,
                                                                         napi_value option);
    static std::tuple<bool, std::unique_ptr<char[]>, void *, int64_t, bool, int64_t> GetWriteArg(napi_env env,
                                                                                                 napi_value argWBuf,
                                                                                                 napi_value argOption);
    static std::tuple<bool, std::unique_ptr<char[]>, void *, int64_t, bool, int64_t> GetWriteArgV9(napi_env env,
        napi_value argWBuf, napi_value argOption);
    static std::tuple<bool, std::unique_ptr<char[]>, std::unique_ptr<char[]>> GetCopyPathArg(napi_env env,
        napi_value srcPath, napi_value dstPath);
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif