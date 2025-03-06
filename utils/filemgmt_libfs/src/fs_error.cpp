/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "fs_error.h"
#include <memory>
#include "uv.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

static int ConvertUVCode2ErrCode(int errCode)
{
    if (errCode >= 0) {
        return errCode;
    }
    auto uvCode = string_view(uv_err_name(errCode));
    if (uvCode2ErrCodeTable.find(uvCode) != uvCode2ErrCodeTable.end()) {
        return uvCode2ErrCodeTable.at(uvCode);
    }
    return UNKNOWN_ERR;
}

FsError::FsError(int errCode)
{
    int genericCode = ConvertUVCode2ErrCode(errCode);
    auto it = errCodeTable.find(genericCode);
    if (it != errCodeTable.end()) {
        errno_ = it->second.first;
        errMsg_ = it->second.second;
    } else {
        errno_ = errCodeTable.at(UNKNOWN_ERR).first;
        errMsg_ = errCodeTable.at(UNKNOWN_ERR).second + ", errno is " + to_string(abs(errCode));
    }
}

int FsError::GetErrNo() const
{
    return errno_;
}

const std::string &FsError::GetErrMsg() const
{
    return errMsg_;
}

FsError::operator bool() const
{
    return errno_ != ERRNO_NOERR;
}

} // namespace OHOS::FileManagement::ModuleFileIO
