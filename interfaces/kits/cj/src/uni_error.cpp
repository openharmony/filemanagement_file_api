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

#include "uni_error.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {

int ConvertUVCode2ErrCode(int errCode)
{
    if (errCode >= 0) {
        return errCode;
    }
    auto uvCode = std::string(uv_err_name(errCode));
    if (uvCode2ErrCodeTable.find(uvCode) != uvCode2ErrCodeTable.end()) {
        return uvCode2ErrCodeTable.at(uvCode);
    }
    return UNKNOWN_ERR;
}

int GetErrorCode(int errCode)
{
    int32_t code;
    int genericCode = ConvertUVCode2ErrCode(errCode);
    if (errCodeTable.find(genericCode) != errCodeTable.end()) {
        code = errCodeTable.at(genericCode);
    } else {
        code = errCodeTable.at(UNKNOWN_ERR);
    }
    return code;
}

}
}
}