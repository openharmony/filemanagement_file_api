/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_TEST_SWAPFS_STORAGE_MANAGER_MOCK_H
#define OHOS_FILEMANAGEMENT_FILE_API_TEST_SWAPFS_STORAGE_MANAGER_MOCK_H

#include <functional>
#include <utility>

#include "storage_manager_proxy.h"

namespace OHOS::FileManagement::Swapfs::Test {
class TestStorageManagerProxy : public OHOS::StorageManager::StorageManagerProxy {
public:
    using GetFreeSizeHandler = std::function<int32_t(int64_t &)>;

    explicit TestStorageManagerProxy(GetFreeSizeHandler handler)
        : StorageManagerProxy(OHOS::sptr<OHOS::IRemoteObject>()), handler_(std::move(handler))
    {
    }

    int32_t GetFreeSize(int64_t &freeSize) override
    {
        return handler_(freeSize);
    }

private:
    GetFreeSizeHandler handler_;
};

inline OHOS::sptr<OHOS::StorageManager::IStorageManager> CreateStorageManagerProxyMock(int64_t freeSize)
{
    return new TestStorageManagerProxy([freeSize](int64_t &value) {
        value = freeSize;
        return 0;
    });
}
} // namespace OHOS::FileManagement::Swapfs::Test

#endif
