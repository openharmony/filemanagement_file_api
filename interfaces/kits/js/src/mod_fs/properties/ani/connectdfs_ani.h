


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

#ifndef FILEMANAGEMENT_FILE_API_CONNECTDFS_ANI_H
#define FILEMANAGEMENT_FILE_API_CONNECTDFS_ANI_H

#include <ani.h>
#include <string>
#include "dfs_listener/file_dfs_listener_stub.h"
#include "distributed_file_daemon_manager.h"
#include "filemgmt_libfs.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace std;

class ANIDfsListener : public FileDfsListenerStub {
public:
    void OnStatus(const std::string &networkId, int32_t status);
    void SetConnectDfsEnv(ani_env *env);
    void SetConnectDfsCBRef(ani_ref ref);
    void SetConnectDfsPromiseRef(ani_ref promiseDeferred);

private:
    ani_env *env_ = nullptr;
    ani_ref onStatusRef_ = nullptr;
    ani_ref promiseDeferred_ = nullptr;
    ani_ref objectlistener = nullptr;
};
class ConnectDfsAni final {
public:
    static void ConnectDfsSync(ani_env *env, [[maybe_unused]] ani_class clazz, ani_string networkId,
                               ani_object listener);
};

}
}
}
}
#endif //FILEMANAGEMENT_FILE_API_CONNECTDFS_ANI_H