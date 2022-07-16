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

#ifndef FILEMGMT_LIBN_N_FUNC_ARG_H
#define FILEMGMT_LIBN_N_FUNC_ARG_H

#include <functional>
#include <map>
#include <memory>

#include "n_napi.h"
#include "n_val.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
enum NARG_CNT {
    ZERO = 0,
    ONE,
    TWO,
    THREE,
    FOUR,
};

enum NARG_POS {
    FIRST = 0,
    SECOND,
    THIRD,
    FOURTH,
};

class NFuncArg final {
public:
    NFuncArg(napi_env env, napi_callback_info info);
    virtual ~NFuncArg();

    bool InitArgs(size_t argc);
    bool InitArgs(size_t minArgc, size_t maxArgc);
    size_t GetArgc() const;
    napi_value GetThisVar() const;
    napi_value operator[](size_t idx) const;
    napi_value GetArg(size_t argPos) const;

private:
    napi_env env_ = nullptr;
    napi_callback_info info_ = nullptr;
    size_t argc_ = 0;
    std::unique_ptr<napi_value[]> argv_ = {nullptr};
    napi_value thisVar_ = nullptr;

    bool InitArgs(std::function<bool()> argcChecker);
    void SetArgc(size_t argc);
    void SetThisVar(napi_value thisVar);
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_FUNC_ARG_H