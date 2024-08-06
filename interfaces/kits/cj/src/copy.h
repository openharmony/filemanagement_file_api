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

#ifndef OHOS_FILE_FS_COPY_H
#define OHOS_FILE_FS_COPY_H

#include <chrono>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sys/inotify.h>
#include <thread>
#include "utils.h"
#include "cj_lambda.h"
#include "ffi_remote_data.h"
#include "macro.h"
#include "task_signal.h"
#include "task_signal_impl.h"
#include "uni_error.h"

extern "C" {
struct CProgress {
    uint64_t processedSize;
    uint64_t totalSize;
};
}

namespace OHOS {
namespace CJSystemapi {
const uint64_t MAX_VALUE = 0x7FFFFFFFFFFFFFFF;
struct ReceiveInfo {
    std::string path;
    std::map<std::string, uint64_t> fileList;
};

struct CjCallbackObject {
    int64_t callbackId = 0;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    std::function<void(CProgress)> callback;
    std::vector<std::pair<int, std::shared_ptr<ReceiveInfo>>> wds;
    uint64_t totalSize = 0;
    uint64_t progressSize = 0;
    uint64_t maxProgressSize = 0;
    int32_t errorCode = 0;
    std::thread notifyHandler;
    explicit CjCallbackObject(int64_t id) : callbackId(id)
    {
        if (callbackId == 0) {
            callback = nullptr;
            return;
        }
        callback = CJLambda::Create(reinterpret_cast<void(*)(CProgress)>(callbackId));
    }
    void CloseFd()
    {
        if (eventFd != -1) {
            close(eventFd);
            eventFd = -1;
        }
        if (notifyFd == -1) {
            return;
        }
        for (auto item : wds) {
            inotify_rm_watch(notifyFd, item.first);
        }
        close(notifyFd);
        notifyFd = -1;
    }
    ~CjCallbackObject()
    {
        CloseFd();
    }
};

struct FileInfos {
    std::string srcUri;
    std::string destUri;
    std::string srcPath;
    std::string destPath;
    std::chrono::steady_clock::time_point notifyTime;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    bool run = true;
    bool hasListener = false;
    int64_t listenerId = 0;
    int64_t copySignalId = 0;
    int64_t exceptionCode = FileFs::ERRNO_NOERR;
    std::shared_ptr<DistributedFS::ModuleTaskSignal::TaskSignal> taskSignal = nullptr;
    std::set<std::string> filePaths;
    bool operator==(const FileInfos &infos) const
    {
        return (srcUri == infos.srcUri && destUri == infos.destUri);
    }
    bool operator<(const FileInfos &infos) const
    {
        if (srcUri == infos.srcUri) {
            return destUri < infos.destUri;
        }
        return srcUri < infos.srcUri;
    }
};

class CopyInfo : public OHOS::FFI::FFIData {
    friend class CopyImpl;
public:
    CopyInfo(int64_t listener, int64_t signal) : listenerId(listener), signalId(signal) {}
    ~CopyInfo();
private:
    int64_t listenerId = 0;
    int64_t signalId = 0;
};

class CopyImpl final {
public:
    static std::map<FileInfos, std::shared_ptr<CjCallbackObject>> cjCbMap_;
    static std::recursive_mutex mutex_;
    static void UnregisterListener(std::shared_ptr<FileInfos> fileInfos);
    static void Copy(const char* srcUri, const char* destUri, sptr<CopyInfo> info);
private:
    static int UpdateProgressSize(const std::string &filePath,
                                  std::shared_ptr<ReceiveInfo> receivedInfo,
                                  std::shared_ptr<CjCallbackObject> callback);
    static void StartNotify(std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback);
    static bool IsRemoteUri(const std::string& uri);
    static int64_t DoCopy(std::shared_ptr<FileInfos> infos, std::shared_ptr<CjCallbackObject> callback);
    static int64_t ExecLocal(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback);
    static void CloseNotifyFd(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback);
    static void WaitNotifyFinished(std::shared_ptr<CjCallbackObject>& callback);
    static void CopyComplete(std::shared_ptr<FileInfos>& infos, std::shared_ptr<CjCallbackObject>& callback);
    static std::shared_ptr<FileInfos> InitCjFileInfo(
        const std::string& srcUri, const std::string& destUri, sptr<CopyInfo> info);
    static std::shared_ptr<CjCallbackObject> RegisterListener(std::shared_ptr<FileInfos>& infos);
    static bool IsFile(const std::string &path);
    static void CheckOrCreatePath(const std::string &destPath);
    static int64_t SubscribeLocalListener(std::shared_ptr<FileInfos>& infos,
                                          std::shared_ptr<CjCallbackObject>& callback);
    static int ExecCopy(std::shared_ptr<FileInfos> infos);
    static int CopyFile(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos);
    static bool IsDirectory(const std::string &path);
    static std::tuple<int, uint64_t> GetFileSize(const std::string &path);
    static int MakeDir(const std::string &path);
    static int CopySubDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos);
    static int CopyDirFunc(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos);
    static uint64_t GetDirSize(std::shared_ptr<FileInfos> infos, std::string path);
    static int RecurCopyDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos);
    static void GetNotifyEvent(std::shared_ptr<FileInfos> infos);
    static bool CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfos> infos);
    static void OnFileReceive(std::shared_ptr<FileInfos> infos);
    static std::shared_ptr<CjCallbackObject> GetRegisteredListener(std::shared_ptr<FileInfos> infos);
    static std::shared_ptr<ReceiveInfo> GetReceivedInfo(int wd, std::shared_ptr<CjCallbackObject> callback);
    static void ReadNotifyEvent(std::shared_ptr<FileInfos> infos);
    static std::string GetRealPath(const std::string& path);
    static std::tuple<bool, int, bool> HandleProgress(inotify_event *event,
                                                 std::shared_ptr<FileInfos> infos,
                                                 std::shared_ptr<CjCallbackObject> callback);
    static void ReceiveComplete(CProgress data,
                                std::shared_ptr<FileInfos> infos,
                                std::shared_ptr<CjCallbackObject> callback);
};
}
}

#endif // OHOS_FILE_FS_COPY_H
