/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_MOCK_DFS_MOCK_H
#define INTERFACES_TEST_UNITTEST_COMMON_MOCK_DFS_MOCK_H

#include "distributed_file_daemon_manager.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::Storage::DistributedFile;

class DfsMock final : public DistributedFileDaemonManager {
public:
    MOCK_METHOD(int32_t, PrepareSession,
        (const std::string &srcUri, const std::string &dstUri, const std::string &srcDeviceId,
            const sptr<IRemoteObject> &listener, HmdfsInfo &info),
        (override));

    int32_t ConnectDfs(const std::string &networkId) override
    {
        return 0;
    }
    int32_t DisconnectDfs(const std::string &networkId) override
    {
        return 0;
    }
    int32_t OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj) override
    {
        return 0;
    }
    int32_t CloseP2PConnectionEx(const std::string &networkId) override
    {
        return 0;
    }
    int32_t CancelCopyTask(const std::string &sessionName) override
    {
        return 0;
    }
    int32_t CancelCopyTask(const std::string &srcUri, const std::string &dstUri)
    {
        return 0;
    }
    int32_t GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus)
    {
        return 0;
    }
    int32_t UpdateDfsSwitchStatus(int32_t switchStatus)
    {
        return 0;
    }
    int32_t GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
    {
        return 0;
    }
    int32_t PushAsset(
        int32_t userId, const sptr<AssetObj> &assetObj, const sptr<IAssetSendCallback> &sendCallback) override
    {
        return 0;
    }
    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }
    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override
    {
        return 0;
    }
    int32_t GetSize(const std::string &uri, bool isSrcUri, uint64_t &size) override
    {
        return 0;
    }
    int32_t IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory) override
    {
        return 0;
    }
    int32_t Copy(const std::string &srcUri, const std::string &destUri, ProcessCallback processCallback) override
    {
        return 0;
    }
    int32_t Cancel(const std::string &srcUri, const std::string &destUri) override
    {
        return 0;
    }
    int32_t Cancel() override
    {
        return 0;
    }
    int32_t RegisterFileDfsListener(const std::string &instanceId, const sptr<IFileDfsListener> &listener) override
    {
        return 0;
    }
    int32_t UnregisterFileDfsListener(const std::string &instanceId) override
    {
        return 0;
    }
    int32_t IsSameAccountDevice(const std::string &networkId, bool &isSameAccount) override
    {
        return 0;
    }

    DfsMock() = default;
    ~DfsMock() = default;

public:
    static std::shared_ptr<DfsMock> GetMock();
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();

private:
    static thread_local std::shared_ptr<DfsMock> dfsMock;
    static thread_local bool mockable;
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_MOCK_DFS_MOCK_H