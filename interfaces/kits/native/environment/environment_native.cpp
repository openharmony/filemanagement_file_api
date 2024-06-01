/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "environment_native.h"

#include <string>
#include <unistd.h>

#include "accesstoken_kit.h"
#include "filemgmt_libhilog.h"
#include "ipc_skeleton.h"
#include "parameter.h"
#include "securec.h"
#include "tokenid_kit.h"

using namespace OHOS;
using namespace OHOS::FileManagement;
namespace {
const std::string STORAGE_PATH = "/storage/Users/";
const std::string READ_WRITE_DOWNLOAD_PERMISSION = "ohos.permission.READ_WRITE_DOWNLOAD_DIRECTORY";
const std::string READ_WRITE_DESKTOP_PERMISSION = "ohos.permission.READ_WRITE_DESKTOP_DIRECTORY";
const std::string READ_WRITE_DOCUMENTS_PERMISSION = "ohos.permission.READ_WRITE_DOCUMENTS_DIRECTORY";
const std::string DOWNLOAD_PATH = "/Download";
const std::string DESKTOP_PATH = "/Desktop";
const std::string DOCUMENTS_PATH = "/Documents";
const std::string DEFAULT_USERNAME = "currentUser";
const std::string FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER = "const.filemanager.full_mount.enable";
const int DEVICE_NOT_SUPPORTED = 801;

static std::string GetUserName()
{
    return DEFAULT_USERNAME;
}

static std::string GetPublicPath(const std::string &directoryName)
{
    return STORAGE_PATH + GetUserName() + directoryName;
}

static bool CheckFileManagerFullMountEnable()
{
    char value[] = "false";
    int retSystem = GetParameter(FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER.c_str(), "false", value, sizeof(value));
    return (retSystem > 0) && (!std::strcmp(value, "true"));
}
}

int GetUserDir(char *path, char **result)
{
    if (!CheckFileManagerFullMountEnable()) {
        HILOGD("Failed to enable the parameter: %{public}s", FILE_MANAGER_FULL_MOUNT_ENABLE_PARAMETER.c_str());
        return -DEVICE_NOT_SUPPORTED;
    }
    std::string dirPath = "";
    dirPath = GetPublicPath(path);
    *result = (char *) malloc((dirPath.length() + 1) * sizeof(char));
    if (*result == nullptr) {
        return -ENOMEM;
    }
    int ret = strcpy_s(*result, (dirPath.length() + 1) * sizeof(char), dirPath.c_str());
    if (ret != 0) {
        HILOGE("Failed to copy memory");
        return -ENOMEM;
    }
    return ret;
}
