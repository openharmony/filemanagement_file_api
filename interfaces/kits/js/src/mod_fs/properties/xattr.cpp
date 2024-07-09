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

#include "xattr.h"

#include <memory>
#include <optional>
#include <sys/xattr.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "n_error.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
using namespace std;
constexpr size_t MAX_XATTR_SIZE = 4096;

static bool IsIllegalXattr(const char *path, const char *key)
{
    if (path == nullptr || key == nullptr) {
        return true;
    }
    bool isIllegalKey = strnlen(key, MAX_XATTR_SIZE) > MAX_XATTR_SIZE;
    if (isIllegalKey) {
        HILOGE("key is too long %{public}d", strnlen(key, MAX_XATTR_SIZE));
    }
    return isIllegalKey;
}

static bool IsIllegalXattr(const char *path, const char *key, const char *value)
{
    if (value == nullptr || IsIllegalXattr(path, key)) {
        return true;
    }
    bool isIllegalValue = strnlen(value, MAX_XATTR_SIZE) > MAX_XATTR_SIZE;
    if (isIllegalValue) {
        HILOGE("value is too long %{public}d", strnlen(value, MAX_XATTR_SIZE));
    }
    return isIllegalValue;
}

static int32_t GetXattrCore(const char *path,
                            const char *key,
                            std::shared_ptr<string> result)
{
    ssize_t xAttrSize = getxattr(path, key, nullptr, 0);
    if (xAttrSize == -1 || xAttrSize == 0) {
        *result = "";
        return ERRNO_NOERR;
    }
    auto xattrValue = CreateUniquePtr<char[]>(static_cast<long>(xAttrSize) + 1);
    xAttrSize = getxattr(path, key, xattrValue.get(), static_cast<size_t>(xAttrSize));
    if (xAttrSize == -1) {
        return errno;
    }
    xattrValue[xAttrSize] = '\0';
    *result = std::string(xattrValue.get());
    return ERRNO_NOERR;
}

napi_value Xattr::SetSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    std::unique_ptr<char[]> value;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid xattr key");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, value, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::THIRD)]).ToUTF8String();
    if (!succ || IsIllegalXattr(path.get(), key.get(), value.get())) {
        HILOGE("Invalid xattr value");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    if (setxattr(path.get(), key.get(), value.get(), strnlen(value.get(), MAX_XATTR_SIZE), 0) < 0) {
        HILOGE("setxattr fail, errno is %{public}d", errno);
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value Xattr::GetSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8StringPath();
    if (!succ || IsIllegalXattr(path.get(), key.get())) {
        HILOGE("Invalid xattr key");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto result = make_shared<std::string>();
    int32_t ret = GetXattrCore(path.get(), key.get(), result);
    if (ret != ERRNO_NOERR) {
        HILOGE("Invalid getxattr");
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUTF8String(env, *result).val_;
}

napi_value Xattr::GetAsync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!succ || IsIllegalXattr(path.get(), key.get())) {
        HILOGE("Invalid xattr key");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto result = make_shared<std::string>();
    auto cbExec = [path = path.get(), key = key.get(), result]() -> NError {
        int ret = GetXattrCore(path, key, result);
        return NError(ret);
    };
    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return {NVal::CreateUTF8String(env, *result)};
    };
    static const std::string PROCEDURE_NAME = "GetXattr";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar)
        .Schedule(PROCEDURE_NAME, cbExec, cbComplete)
        .val_;
}


napi_value Xattr::SetAsync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    std::unique_ptr<char[]> value;
    tie(succ, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!succ) {
        HILOGE("Invalid path");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid xattr key");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    tie(succ, value, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::THIRD)]).ToUTF8String();
    if (!succ || IsIllegalXattr(path.get(), key.get(), value.get())) {
        HILOGE("Invalid xattr value");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto cbExec = [path = path.get(), key = key.get(), value = value.get()]() -> NError {
        if (setxattr(path, key, value, strnlen(value, MAX_XATTR_SIZE), 0)) {
            HILOGE("setxattr fail, errno is %{public}d", errno);
            return NError(errno);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };
    static const std::string PROCEDURE_NAME = "SetXattr";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar)
        .Schedule(PROCEDURE_NAME, cbExec, cbComplete)
        .val_;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS