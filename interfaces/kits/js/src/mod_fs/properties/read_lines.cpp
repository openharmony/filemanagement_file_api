/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "read_lines.h"

#include <unistd.h>

#include "class_readeriterator/readeriterator_n_exporter.h"
#include "class_readeriterator/readeriterator_entity.h"
#include "common_func.h"
#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "rust_file.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;

static int GetFileSize(const string &path, int64_t &offset) {
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::fs_req_cleanup)*> stat_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::fs_req_cleanup };

    if (!stat_req) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }

    int ret = uv_fs_stat(nullptr, stat_req.get(), path.c_str(), nullptr);
    if (ret < 0) {
        HILOGE("Failed to get file stat by path");
        return ret;
    }

    offset = stat_req->statbuf.st_size;
    return ERRNO_NOERR;
}

static NVal InstantiateReaderIterator(napi_env env, void *iterator, int64_t offset)
{
    if (iterator == nullptr) {
        HILOGE("Invalid argument iterator");
        NError(EINVAL).ThrowErr(env);
        return NVal();
    }

    napi_value objReaderIterator = NClass::InstantiateClass(env, ReaderIteratorNExporter::className_, {});
    if (!objReaderIterator) {
        HILOGE("Failed to instantiate class ReaderIterator");
        NError(UNKROWN_ERR).ThrowErr(env);
        return NVal();
    }

    auto readerIteratorEntity = NClass::GetEntityOf<ReaderIteratorEntity>(env, objReaderIterator);
    if (!readerIteratorEntity) {
        HILOGE("Failed to get readerIteratorEntity");
        NError(EIO).ThrowErr(env);
        return NVal();
    }

    readerIteratorEntity->iterator = iterator;
    readerIteratorEntity->offset = offset;

    return { env, objReaderIterator };
}

struct ReaderIteratorArg
{
    void *iterator = nullptr;
    int64_t offset = 0;
};

napi_value ReadLines::Async(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, tmp, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto arg = std::make_shared<ReaderIteratorArg>();
    auto cbExec = [arg = arg, path = tmp.get()]() -> NError {
        arg->iterator = ::ReaderIterator(path);
        GetFileSize(string(path), arg->offset);
        if (errno != 0) {
            HILOGE("Failed to ReadLines");
            return NError(errno);
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return InstantiateReaderIterator(env, arg->iterator, arg->offset);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_READLINES_NAME, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_READLINES_NAME, cbExec, cbCompl).val_;
    }
}

napi_value ReadLines::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, path, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        HILOGE("Invalid path from JS first argument");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    void *iterator = ::ReaderIterator(path.get());
    if (errno != 0) {
        HILOGE("Failed to ReadLinesSync");
        NError(errno).ThrowErr(env);
        return nullptr;
    }

    int64_t offset = 0;
    int ret = GetFileSize(string(path.get()), offset);
    if (ret != 0) {
        HILOGE("Failed to get file size");
        return nullptr;
    }

    return InstantiateReaderIterator(env, iterator, offset).val_;
}

} // ModuleFileIO
} // FileManagement
} // OHOS