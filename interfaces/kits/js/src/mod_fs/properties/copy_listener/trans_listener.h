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

#ifndef FILEMANAGEMENT_FILE_API_TRANS_LISTENER_H
#define FILEMANAGEMENT_FILE_API_TRANS_LISTENER_H

#include <condition_variable>

#include "copy.h"
#include "distributed_file_daemon_manager.h"
#include "file_trans_listener_stub.h"
#include "file_uri.h"

constexpr int NONE = 0;
constexpr int SUCCESS = 1;
constexpr int FAILED = 2;
namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
class TransListener : public Storage::DistributedFile::FileTransListenerStub {
public:
    int32_t OnFileReceive(uint64_t totalBytes, uint64_t processedBytes) override;
    int32_t OnFinished(const std::string &sessionName) override;
    int32_t OnFailed(const std::string &sessionName) override;
    static NError CopyFileFromSoftBus(const std::string &srcUri,
                                      const std::string &destUri,
                                      std::shared_ptr<FileInfos> fileInfos,
                                      std::shared_ptr<JsCallbackObject> callback);

private:
    static std::string GetNetworkIdFromUri(const std::string &uri);
    static void CallbackComplete(uv_work_t *work, int stat);
    static void RmDir(const std::string &path);
    static std::string CreateDfsCopyPath();
    static std::string GetFileName(const std::string &path);
    static int32_t CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
        const std::string &sandboxPath);
    static int32_t PrepareCopySession(const std::string &srcUri,
                                      const std::string &destUri,
                                      TransListener* transListener,
                                      Storage::DistributedFile::HmdfsInfo &info,
                                      std::string &disSandboxPath);

    std::mutex cvMutex_;
    std::condition_variable cv_;
    int copyEvent_ = NONE;
    std::mutex callbackMutex_;
    std::shared_ptr<JsCallbackObject> callback_;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_TRANS_LISTENER_H