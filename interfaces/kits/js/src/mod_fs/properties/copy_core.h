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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_CORE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_CORE_H

#include <chrono>
#include <condition_variable>
#include <set>
#include <thread>
#include <sys/inotify.h>

#include "bundle_mgr_client_impl.h"
#include "filemgmt_libfs.h"
#include "filemgmt_libhilog.h"
#include "fs_task_signal.h"
#include "i_progress_listener.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::AppExecFwk;
using namespace DistributedFS::ModuleTaskSignal;

struct CopyOptions {
    shared_ptr<IProgressListener> progressListener;
    FsTaskSignal* copySignal = nullptr;
};

struct ReceiveInfo {
    std::string path;                         // dir name
    std::map<std::string, uint64_t> fileList; // filename, proceededSize
};

struct FsCallbackObject {
    std::shared_ptr<IProgressListener> listener = nullptr;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    std::vector<std::pair<int, std::shared_ptr<ReceiveInfo>>> wds;
    uint64_t totalSize = 0;
    uint64_t progressSize = 0;
    uint64_t maxProgressSize = 0;
    int32_t errorCode = 0;
    std::thread notifyHandler;
    std::mutex readMutex;
    std::condition_variable cv;
    std::mutex cvLock;
    bool reading = false;
    bool closed = false;
    explicit FsCallbackObject(std::shared_ptr<IProgressListener> listener) : listener(listener) {}

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

    ~FsCallbackObject()
    {
        CloseFd();
    }
};

struct FsFileInfos {
    std::string srcUri;
    std::string destUri;
    std::string srcPath;
    std::string destPath;
    bool isFile = false;
    std::chrono::steady_clock::time_point notifyTime;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    bool run = true;
    bool hasListener = false;
    std::shared_ptr<IProgressListener> listener = nullptr;
    TaskSignal* taskSignal = nullptr;
    std::set<std::string> filePaths;
    int exceptionCode = ERRNO_NOERR; // notify copy thread or listener thread has exceptions.
    bool operator==(const FsFileInfos &infos) const
    {
        return (srcUri == infos.srcUri && destUri == infos.destUri);
    }
    bool operator<(const FsFileInfos &infos) const
    {
        if (srcUri == infos.srcUri) {
            return destUri < infos.destUri;
        }
        return srcUri < infos.srcUri;
    }
};

struct FsUvEntry {
    std::shared_ptr<FsCallbackObject> callback;
    std::shared_ptr<FsFileInfos> fileInfos;
    uint64_t progressSize = 0;
    uint64_t totalSize = 0;
    FsUvEntry(const std::shared_ptr<FsCallbackObject> &cb, std::shared_ptr<FsFileInfos> fileInfos)
        : callback(cb), fileInfos(fileInfos) {}

    explicit FsUvEntry(const std::shared_ptr<FsCallbackObject> &cb) : callback(cb) {}
};

class CopyCore final {
public:
    static FsResult<void> DoCopy(const string &src, const string &dest, std::optional<CopyOptions> &options);

private:
    // operator of params
    static bool ValidOperand(std::string uriStr);
    static int CheckOrCreatePath(const std::string &destPath);
    static bool ValidParams(const string &src, const string &dest);

    // operator of local listener
    static std::shared_ptr<FsCallbackObject> RegisterListener(const std::shared_ptr<FsFileInfos> &infos);
    static std::shared_ptr<FsCallbackObject> GetRegisteredListener(std::shared_ptr<FsFileInfos> infos);
    static void UnregisterListener(std::shared_ptr<FsFileInfos> fileInfos);
    static int ExecLocal(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static void CopyComplete(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static void WaitNotifyFinished(std::shared_ptr<FsCallbackObject> callback);
    static void ReadNotifyEvent(std::shared_ptr<FsFileInfos> infos);
    static void ReadNotifyEventLocked(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static int SubscribeLocalListener(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static void OnFileReceive(std::shared_ptr<FsFileInfos> infos);
    static void GetNotifyEvent(std::shared_ptr<FsFileInfos> infos);
    static void StartNotify(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static FsUvEntry *GetUVEntry(std::shared_ptr<FsFileInfos> infos);
    static void ReceiveComplete(std::shared_ptr<FsUvEntry> entry);
    static void CloseNotifyFd(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static void CloseNotifyFdLocked(std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);

    // operator of file
    static int RecurCopyDir(const string &srcPath, const string &destPath, std::shared_ptr<FsFileInfos> infos);
    static tuple<int, uint64_t> GetFileSize(const std::string &path);
    static uint64_t GetDirSize(std::shared_ptr<FsFileInfos> infos, std::string path);
    static int CopyFile(const string &src, const string &dest, std::shared_ptr<FsFileInfos> infos);
    static int MakeDir(const string &path);
    static int CopySubDir(const string &srcPath, const string &destPath, std::shared_ptr<FsFileInfos> infos);
    static int CopyDirFunc(const string &src, const string &dest, std::shared_ptr<FsFileInfos> infos);
    static tuple<int, std::shared_ptr<FsFileInfos>> CreateFileInfos(
        const std::string &srcUri, const std::string &destUri, const std::optional<CopyOptions> &options);
    static int ExecCopy(std::shared_ptr<FsFileInfos> infos);

    // operator of file size
    static int UpdateProgressSize(const std::string &filePath, std::shared_ptr<ReceiveInfo> receivedInfo,
        std::shared_ptr<FsCallbackObject> callback);
    static tuple<bool, int, bool> HandleProgress(
        inotify_event *event, std::shared_ptr<FsFileInfos> infos, std::shared_ptr<FsCallbackObject> callback);
    static std::shared_ptr<ReceiveInfo> GetReceivedInfo(int wd, std::shared_ptr<FsCallbackObject> callback);
    static bool CheckFileValid(const std::string &filePath, std::shared_ptr<FsFileInfos> infos);

    // operator of uri or path
    static bool IsValidUri(const std::string &uri);
    static bool IsRemoteUri(const std::string &uri);
    static bool IsDirectory(const std::string &path);
    static bool IsFile(const std::string &path);
    static bool IsMediaUri(const std::string &uriPath);
    static std::string ConvertUriToPath(const std::string &uri);
    static std::string GetRealPath(const std::string &path);

    static std::recursive_mutex mutex_;
    static std::map<FsFileInfos, std::shared_ptr<FsCallbackObject>> callbackMap_;
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_CORE_H