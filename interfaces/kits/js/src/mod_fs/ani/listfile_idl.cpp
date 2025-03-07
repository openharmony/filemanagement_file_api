/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#define KOALA_INTEROP_MODULE NotSpecifiedInteropModule
#include "fileio.h"
#include "filemgmt_libhilog.h"
#include "listfile_core.h"

using namespace std;
using namespace OHOS::FileManagement::ModuleFileIO;

OH_NativePointer GlobalScope_ohos_file_fs_listFileSyncImpl(const OH_String* path, const Opt_ListFileOptions* options) {
    string srcPath(path->chars, path->length);
    HILOGE("start zhouxinzhouxin %s", srcPath.c_str());

    auto ret = ListFileCore::DoListFile(srcPath);
    if (ret.IsSuccess()) {
        HILOGE("Success:zhouxinzhouxin DoListFile");
        auto fileList = ret.GetData().value();
        for (int i = 0; i < fileList.size(); i++) {
            HILOGE("List%d: %s", i, fileList[i].c_str());
        }
        HILOGE("Success:zhouxinzhouxin print result");

        void *res = static_cast<void*>(&fileList);
        return res;
    }

    return nullptr;
}