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

#ifndef FILEMANAGEMENT_FILE_API_ANI_DFS_LISTENER_H
#define FILEMANAGEMENT_FILE_API_ANI_DFS_LISTENER_H

#include "ani.h"
#include "file_dfs_listener_stub.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

class AniDfsListener : public FileDfsListenerStub {
public:
    void OnStatus(const std::string &networkId, int32_t status, const std::string &path, int32_t type) override;
    bool PrepareData(const ani_env* env, ani_object aniListeners);
    ~AniDfsListener();
private:
    ani_ref listenerRef_ = nullptr;
    ani_ref onStatusRef_ = nullptr;
    ani_vm *vm_ = nullptr;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_ANI_DFS_LISTENER_H