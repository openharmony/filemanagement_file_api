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

#ifndef FILEMANAGEMENT_FILE_API_COPY_H
#define FILEMANAGEMENT_FILE_API_COPY_H

#include <set>
#include <sys/inotify.h>
#include <thread>
#include <chrono>

#include "bundle_mgr_client_impl.h"
#include "common_func.h"
#include "filemgmt_libn.h"
#include "n_async/n_ref.h"
#include "task_signal.h"
#include "class_tasksignal/task_signal_entity.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::AppExecFwk;
using namespace DistributedFS::ModuleTaskSignal;
struct ReceiveInfo {
    std::string path;                         // dir name
    std::map<std::string, uint64_t> fileList; // filename, proceededSize
};
struct JsCallbackObject {
    napi_env env = nullptr;
    LibN::NRef nRef;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    std::vector<std::pair<int, std::shared_ptr<ReceiveInfo>>> wds;
    uint64_t totalSize = 0;
    uint64_t progressSize = 0;
    uint64_t maxProgressSize = 0;
    std::thread notifyHandler;
    explicit JsCallbackObject(napi_env env, LibN::NVal jsVal) : env(env), nRef(jsVal) {}

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

    ~JsCallbackObject()
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
    napi_env env;
    NVal listener;
    NVal copySignal;
    std::shared_ptr<TaskSignal> taskSignal = nullptr;
    std::set<std::string> filePaths;
    int exceptionCode = ERRNO_NOERR;    // notify copy thread or listener thread has exceptions.
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

struct UvEntry {
    std::shared_ptr<JsCallbackObject> callback;
    std::shared_ptr<FileInfos> fileInfos;
    uint64_t progressSize = 0;
    uint64_t totalSize = 0;
    UvEntry(const std::shared_ptr<JsCallbackObject> &cb, std::shared_ptr<FileInfos> fileInfos)
        : callback(cb), fileInfos(fileInfos)
    {
    }
    explicit UvEntry(const std::shared_ptr<JsCallbackObject> &cb) : callback(cb) {}
};

class Copy final {
public:
    static napi_value Async(napi_env env, napi_callback_info info);
    static std::map<FileInfos, std::shared_ptr<JsCallbackObject>> jsCbMap_;
    static void UnregisterListener(std::shared_ptr<FileInfos> infos);
    static std::recursive_mutex mutex_;

private:
    // operator of napi
    static tuple<bool, std::string> ParseJsOperand(napi_env env, NVal pathOrFdFromJsArg);
    static tuple<bool, NVal> GetListenerFromOptionArg(napi_env env, const NFuncArg &funcArg);
    static tuple<bool, NVal> GetCopySignalFromOptionArg(napi_env env, const NFuncArg &funcArg);
    static void CheckOrCreatePath(const std::string &destPath);
    static int ParseJsParam(napi_env env, NFuncArg &funcArg, std::shared_ptr<FileInfos> &fileInfos);

    // operator of local listener
    static int ExecLocal(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);
    static void CopyComplete(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);
    static void WaitNotifyFinished(std::shared_ptr<JsCallbackObject> callback);
    static void ReadNotifyEvent(std::shared_ptr<FileInfos> infos);
    static int SubscribeLocalListener(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);
    static std::shared_ptr<JsCallbackObject> RegisterListener(napi_env env, const std::shared_ptr<FileInfos> &infos);
    static void OnFileReceive(std::shared_ptr<FileInfos> infos);
    static void GetNotifyEvent(std::shared_ptr<FileInfos> infos);
    static void StartNotify(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);
    static uv_work_t *GetUVwork(std::shared_ptr<FileInfos> infos);
    static void ReceiveComplete(uv_work_t *work, int stat);
    static std::shared_ptr<JsCallbackObject> GetRegisteredListener(std::shared_ptr<FileInfos> infos);
    static void CloseNotifyFd(std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);

    // operator of file
    static int RecurCopyDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfos> infos);
    static tuple<int, uint64_t> GetFileSize(const std::string &path);
    static uint64_t GetDirSize(std::shared_ptr<FileInfos> infos, const std::string &path);
    static int CopyFile(const string &src, const string &dest, std::shared_ptr<FileInfos> infos);
    static int MakeDir(const string &path);
    static int CopySubDir(const string &srcPath, const string &destPath, std::shared_ptr<FileInfos> infos);
    static int CopyDirFunc(const string &src, const string &dest, std::shared_ptr<FileInfos> infos);
    static tuple<int, std::shared_ptr<FileInfos>> CreateFileInfos(
        const std::string &srcUri, const std::string &destUri, const NVal &listener, NVal copySignal);
    static int ExecCopy(std::shared_ptr<FileInfos> infos);

    // operator of file size
    static int UpdateProgressSize(const std::string &filePath,
                                  std::shared_ptr<ReceiveInfo> receivedInfo,
                                  std::shared_ptr<JsCallbackObject> callback);
    static tuple<bool, int, bool> HandleProgress(
        inotify_event *event, std::shared_ptr<FileInfos> infos, std::shared_ptr<JsCallbackObject> callback);
    static std::shared_ptr<ReceiveInfo> GetReceivedInfo(int wd, std::shared_ptr<JsCallbackObject> callback);
    static bool CheckFileValid(const std::string &filePath, std::shared_ptr<FileInfos> infos);

    // operator of uri or path
    static bool IsValidUri(const std::string &uri);
    static bool IsRemoteUri(const std::string &uri);
    static bool IsDirectory(const std::string &path);
    static bool IsFile(const std::string &path);
    static std::string ConvertUriToPath(const std::string &uri);
    static std::string GetRealPath(const std::string& path);
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_COPY_H