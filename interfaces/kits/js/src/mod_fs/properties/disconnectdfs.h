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

#ifndef FILEMANAGEMENT_FILE_API_DISCONNECTDFS_H
#define FILEMANAGEMENT_FILE_API_DISCONNECTDFS_H

#include "bundle_mgr_client_impl.h"
#include <chrono>
#include "common_func.h"
#include "filemgmt_libn.h"
#include "n_async/n_ref.h"
#include <set>
#include <sys/inotify.h>
#include <thread>

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::AppExecFwk;
const std::string PROCEDURE_DISCONNECTDFS_NAME = "FileFSDisconnectDfs";

class DisconnectDfs final {
public:
    static napi_value Async(napi_env env, napi_callback_info info);

private:
    // operator of napi
    static tuple<bool, std::string> ParseJsOperand(napi_env env, NVal paramFromJsArg);
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_DISCONNECTDFS_H