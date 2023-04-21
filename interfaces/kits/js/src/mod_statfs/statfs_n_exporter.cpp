/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "statfs_n_exporter.h"

#include <sys/statvfs.h>
#include <tuple>

namespace OHOS {
namespace FileManagement {
namespace ModuleStatfs {
using namespace FileManagement::LibN;

napi_value GetFrSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env, "Failed get info");
        return nullptr;
    }
    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bavail);
    return NVal::CreateInt64(env, freeSize).val_;
}

napi_value GetFrSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                      static_cast<unsigned long long>(diskInfo.f_bavail);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetFrSize";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetBSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, diskInfo.f_bsize).val_;
}

napi_value GetBSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = diskInfo.f_bsize;
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetBSize";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetBAvailSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, diskInfo.f_bavail).val_;
}

napi_value GetBAvail(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = diskInfo.f_bavail;
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetBAvail";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetBlocksSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, diskInfo.f_blocks).val_;
}

napi_value GetBlocks(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = diskInfo.f_blocks;
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetBlocks";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetBFreeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, diskInfo.f_bfree).val_;
}

napi_value GetBFree(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = diskInfo.f_bfree;
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetBFree";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetFreeBytesSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    unsigned long long freeSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                  static_cast<unsigned long long>(diskInfo.f_bfree);
    return NVal::CreateInt64(env, freeSize).val_;
}

napi_value GetFreeBytes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                      static_cast<unsigned long long>(diskInfo.f_bfree);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    static const std::string PROCEDURE_NAME = "GetFreeBytes";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}

napi_value GetTotalBytesSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    struct statvfs diskInfo;
    int ret = statvfs(path.get(), &diskInfo);
    if (ret != 0) {
        NError(errno).ThrowErr(env);
        return nullptr;
    }
    unsigned long long totalSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                                   static_cast<unsigned long long>(diskInfo.f_blocks);
    return NVal::CreateInt64(env, totalSize).val_;
}

napi_value GetTotalBytes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> path;
    tie(succ, path, std::ignore) = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(EINVAL).ThrowErr(env, "Invalid path");
        return nullptr;
    }

    auto resultSize = std::make_shared<unsigned long long>();
    std::string pathString(path.get());
    auto cbExec = [pathString, resultSize]() -> NError {
        struct statvfs diskInfo;
        int ret = statvfs(pathString.c_str(), &diskInfo);
        if (ret != 0) {
            return NError(errno);
        }
        *resultSize = static_cast<unsigned long long>(diskInfo.f_bsize) *
                      static_cast<unsigned long long>(diskInfo.f_blocks);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    static const std::string PROCEDURE_NAME = "GetTotalBytes";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::SECOND]);
    return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
}
} // namespace ModuleStatfs
} // namespace FileManagement
} // namespace OHOS
