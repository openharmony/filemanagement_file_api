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

#include "constants.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "securec.h"
#include "../../common/log.h"
#include "../../common/napi/n_class.h"
#include "../../common/napi/n_func_arg.h"
#include "../../common/uni_error.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

napi_value Constants::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool Constants::Export()
{
    // access
    napi_value F_OK_ = nullptr;
    napi_create_int32(exports_.env_, F_OK, &F_OK_);                       // 0 F_OK
    napi_value R_OK_ = nullptr;
    napi_create_int32(exports_.env_, R_OK, &R_OK_);                       // 4 R_OK
    napi_value W_OK_ = nullptr;
    napi_create_int32(exports_.env_, W_OK, &W_OK_);                       // 2 W_OK
    napi_value X_OK_ = nullptr;
    napi_create_int32(exports_.env_, X_OK, &X_OK_);                       // 1  X_OK
    // open
    napi_value O_RDONLY_ = nullptr;
    napi_create_int32(exports_.env_, O_RDONLY, &O_RDONLY_);               // 0 O_RDONLY
    napi_value O_WRONLY_ = nullptr;
    napi_create_int32(exports_.env_, O_WRONLY, &O_WRONLY_);               // 1 O_WRONLY
    napi_value O_RDWR_ = nullptr;
    napi_create_int32(exports_.env_, O_RDWR, &O_RDWR_);                   // 2 O_RDWR
    napi_value O_CREAT_ = nullptr;
    napi_create_int32(exports_.env_, O_CREAT, &O_CREAT_);                 // 0o100 O_CREAT
    napi_value O_EXCL_ = nullptr;
    napi_create_int32(exports_.env_, O_EXCL, &O_EXCL_);                   // 0o200 O_EXCL
    napi_value O_TRUNC_  = nullptr;
    napi_create_int32(exports_.env_, O_TRUNC, &O_TRUNC_);                 // 0o1000 O_TRUNC
    napi_value O_APPEND_ = nullptr;
    napi_create_int32(exports_.env_, O_APPEND, &O_APPEND_);               // 0o2000 O_APPEND
    napi_value O_NONBLOCK_ = nullptr;
    napi_create_int32(exports_.env_, O_NONBLOCK, &O_NONBLOCK_);           // 0o4000 O_NONBLOCK
    napi_value O_DIRECTORY_ = nullptr;
    napi_create_int32(exports_.env_, O_DIRECTORY, &O_DIRECTORY_);         // 0o200000 O_DIRECTORY
    napi_value O_NOFOLLOW_ = nullptr;
    napi_create_int32(exports_.env_, O_NOFOLLOW, &O_NOFOLLOW_);           // 0o400000 O_NOFOLLOW
    napi_value O_SYNC_ = nullptr;
    napi_create_int32(exports_.env_, O_SYNC, &O_SYNC_);                   // 0o4010000 O_SYNC

    // stat
    napi_value S_IFMT_ = nullptr;
    napi_create_int32(exports_.env_, S_IFMT, &S_IFMT_);                   // 0o170000 S_IFMT
    napi_value S_IFSOCK_ = nullptr;
    napi_create_int32(exports_.env_, S_IFSOCK, &S_IFSOCK_);               // 0o140000 S_IFSOCK
    napi_value S_IFLNK_ = nullptr;
    napi_create_int32(exports_.env_, S_IFLNK, &S_IFLNK_);                 // 0o120000 S_IFLNK
    napi_value S_IFREG_ = nullptr;
    napi_create_int32(exports_.env_, S_IFREG, &S_IFREG_);                 // 0o100000 S_IFREG
    napi_value S_IFBLK_ = nullptr;
    napi_create_int32(exports_.env_, S_IFBLK, &S_IFBLK_);                 // 0o060000 S_IFBLK
    napi_value S_IFDIR_ = nullptr;
    napi_create_int32(exports_.env_, S_IFDIR, &S_IFDIR_);                 // 0o040000 S_IFDIR
    napi_value S_IFCHR_  = nullptr;
    napi_create_int32(exports_.env_, S_IFCHR, &S_IFCHR_);                 // 0o020000 S_IFCHR
    napi_value S_IFIFO_  = nullptr;
    napi_create_int32(exports_.env_, S_IFIFO, &S_IFIFO_);                 // 0o010000 S_IFIFO
    napi_value S_IRWXU_  = nullptr;
    napi_create_int32(exports_.env_, S_IRWXU, &S_IRWXU_);                 // 0o0700 S_IRWXU
    napi_value S_IRUSR_  = nullptr;
    napi_create_int32(exports_.env_, S_IRUSR, &S_IRUSR_);                 // 0o0400 S_IRUSR
    napi_value S_IWUSR_  = nullptr;
    napi_create_int32(exports_.env_, S_IWUSR, &S_IWUSR_);                 // 0o0200 S_IWUSR
    napi_value S_IXUSR_  = nullptr;
    napi_create_int32(exports_.env_, S_IXUSR, &S_IXUSR_);                 // 0o0100 S_IXUSR
    napi_value S_IRWXG_  = nullptr;
    napi_create_int32(exports_.env_, S_IRWXG, &S_IRWXG_);                 // 0o0070 S_IRWXG
    napi_value S_IRGRP_  = nullptr;
    napi_create_int32(exports_.env_, S_IRGRP, &S_IRGRP_);                 // 0o0040 S_IRGRP
    napi_value S_IWGRP_  = nullptr;
    napi_create_int32(exports_.env_, S_IWGRP, &S_IWGRP_);                 // 0o0020 S_IWGRP
    napi_value S_IXGRP_  = nullptr;
    napi_create_int32(exports_.env_, S_IXGRP, &S_IXGRP_);                 // 0o0010 S_IXGRP
    napi_value S_IRWXO_  = nullptr;
    napi_create_int32(exports_.env_, S_IRWXO, &S_IRWXO_);                 // 0o0007 S_IRWXO
    napi_value S_IROTH_  = nullptr;
    napi_create_int32(exports_.env_, S_IROTH, &S_IROTH_);                 // 0o0004 S_IROTH
    napi_value S_IWOTH_  = nullptr;
    napi_create_int32(exports_.env_, S_IWOTH, &S_IWOTH_);                 // 0o0002 S_IWOTH
    napi_value S_IXOTH_  = nullptr;
    napi_create_int32(exports_.env_, S_IXOTH, &S_IXOTH_);                 // 0o0001 S_IXOTH

    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiStaticProperty("F_OK", F_OK_),
        NVal::DeclareNapiStaticProperty("R_OK", R_OK_),
        NVal::DeclareNapiStaticProperty("W_OK", W_OK_),
        NVal::DeclareNapiStaticProperty("X_OK", X_OK_),
        NVal::DeclareNapiStaticProperty("O_RDONLY", O_RDONLY_),
        NVal::DeclareNapiStaticProperty("O_WRONLY", O_WRONLY_),
        NVal::DeclareNapiStaticProperty("O_RDWR", O_RDWR_),
        NVal::DeclareNapiStaticProperty("O_CREAT", O_CREAT_),
        NVal::DeclareNapiStaticProperty("O_EXCL", O_EXCL_),
        NVal::DeclareNapiStaticProperty("O_TRUNC", O_TRUNC_),
        NVal::DeclareNapiStaticProperty("O_APPEND", O_APPEND_),
        NVal::DeclareNapiStaticProperty("O_NONBLOCK", O_NONBLOCK_),
        NVal::DeclareNapiStaticProperty("O_DIRECTORY", O_DIRECTORY_),
        NVal::DeclareNapiStaticProperty("O_NOFOLLOW", O_NOFOLLOW_),
        NVal::DeclareNapiStaticProperty("O_SYNC", O_SYNC_),
        NVal::DeclareNapiStaticProperty("S_IFMT", S_IFMT_),
        NVal::DeclareNapiStaticProperty("S_IFSOCK", S_IFSOCK_),
        NVal::DeclareNapiStaticProperty("S_IFLNK", S_IFLNK_),
        NVal::DeclareNapiStaticProperty("S_IFREG", S_IFREG_),
        NVal::DeclareNapiStaticProperty("S_IFBLK", S_IFBLK_),
        NVal::DeclareNapiStaticProperty("S_IFDIR", S_IFDIR_),
        NVal::DeclareNapiStaticProperty("S_IFCHR", S_IFCHR_),
        NVal::DeclareNapiStaticProperty("S_IFIFO", S_IFIFO_),
        NVal::DeclareNapiStaticProperty("S_IRWXU", S_IRWXU_),
        NVal::DeclareNapiStaticProperty("S_IRUSR", S_IRUSR_),
        NVal::DeclareNapiStaticProperty("S_IWUSR", S_IWUSR_),
        NVal::DeclareNapiStaticProperty("S_IXUSR", S_IXUSR_),
        NVal::DeclareNapiStaticProperty("S_IRWXG", S_IRWXG_),
        NVal::DeclareNapiStaticProperty("S_IRGRP", S_IRGRP_),
        NVal::DeclareNapiStaticProperty("S_IWGRP", S_IWGRP_),
        NVal::DeclareNapiStaticProperty("S_IXGRP", S_IXGRP_),
        NVal::DeclareNapiStaticProperty("S_IRWXO", S_IRWXO_),
        NVal::DeclareNapiStaticProperty("S_IROTH", S_IROTH_),
        NVal::DeclareNapiStaticProperty("S_IWOTH", S_IWOTH_),
        NVal::DeclareNapiStaticProperty("S_IXOTH", S_IXOTH_),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, Constants::Constructor, std::move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string Constants::GetClassName()
{
    return Constants::className_;
}

Constants::Constants(napi_env env, napi_value exports) : NExporter(env, exports) {}
Constants::~Constants() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
