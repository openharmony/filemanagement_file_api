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

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "accesstoken_kit_mock.h"
#include "swapfs.h"
#include "swapfs_c_api.h"
#include "swapfs_manager.h"
#include "swapfs_session_cleaner.h"
#include "swapfs_storage_manager_mock.h"
#include "tokenid_kit_mock.h"

namespace {
using OHOS::FileManagement::Swapfs::MIN_AVAILABLE_DEVICE_SPACE_BYTES;
using OHOS::FileManagement::Swapfs::ProxySwapControlProvider;
using OHOS::FileManagement::Swapfs::SwapControlProvider;
using OHOS::FileManagement::Swapfs::SwapfsSessionCleaner;
using OHOS::FileManagement::Swapfs::Test::CreateStorageManagerProxyMock;

constexpr const char *TEST_BASE_DIR = "/data/swapfs_test";
constexpr const char *TEST_SWAP_BASE = "/data/swapfs_test/swapfs_ut";
constexpr const char *TEST_SWAP_ROOT = "/data/swapfs_test/swapfs_ut/swapfs";
constexpr const char *TEST_SWAP_ROOT_FILE = "/data/swapfs_test/swapfs_root_file_ut";
constexpr mode_t TEST_DIR_MODE = S_IRWXU;
constexpr mode_t TEST_FILE_MODE = S_IRUSR | S_IWUSR;
constexpr uint32_t DESTROY_WAIT_SPINS = 10000;
std::vector<OH_SwapfsManager *> g_activeManagers;

bool StartsWith(const std::string &value, const char *prefix)
{
    return value.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

class SwapfsTest : public testing::Test {
protected:
    static void SetUpTestSuite()
    {
        mkdir(TEST_BASE_DIR, TEST_DIR_MODE);
        OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::EnableMock();
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::EnableMock();
        auto tokenIdMock = OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::GetMock();
        ON_CALL(*tokenIdMock, IsSystemAppByFullTokenID(testing::_))
            .WillByDefault(testing::Return(true));
        auto accessTokenMock =
            OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::GetMock();
        ON_CALL(*accessTokenMock, VerifyAccessToken(testing::_, testing::_))
            .WillByDefault(testing::Return(0));
    }

    static void TearDownTestSuite()
    {
        OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::DisableMock();
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::DisableMock();
    }

    void SetUp() override
    {
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE);
        ASSERT_EQ(mkdir(TEST_SWAP_BASE, TEST_DIR_MODE), 0);
    }

    void TearDown() override
    {
        for (auto *manager : g_activeManagers) {
            EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
        }
        g_activeManagers.clear();
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE);
        (void)unlink(TEST_SWAP_ROOT_FILE);
    }
};

int CreateTrackedManager(const OH_SwapfsConfig *config, OH_SwapfsManager **manager)
{
    int ret = OH_Swapfs_CreateManager(config, manager);
    if (ret == SWAPFS_E_OK && manager != nullptr && *manager != nullptr) {
        g_activeManagers.emplace_back(*manager);
    }
    return ret;
}

int DestroyTrackedManager(OH_SwapfsManager *manager)
{
    int ret = OH_Swapfs_DestroyManager(manager);
    if (ret == SWAPFS_E_OK) {
        auto iter = std::remove(g_activeManagers.begin(), g_activeManagers.end(), manager);
        g_activeManagers.erase(iter, g_activeManagers.end());
    }
    return ret;
}

// Route test calls through the tracker so fatal assertions cannot leak live managers.
#define OH_Swapfs_CreateManager CreateTrackedManager
#define OH_Swapfs_DestroyManager DestroyTrackedManager

OH_SwapfsManager *CreateTestManager(const OH_SwapfsConfig *config)
{
    OH_SwapfsManager *manager = nullptr;
    int ret = SwapfsNativeCreateManager(config, &manager);
    if (ret != SWAPFS_E_OK) {
        return nullptr;
    }
    auto provider = std::make_unique<ProxySwapControlProvider>();
    provider->proxy_ = CreateStorageManagerProxyMock(
        static_cast<int64_t>(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 64ULL * 1024ULL * 1024ULL));
    manager->impl.control_ = std::move(provider);
    g_activeManagers.emplace_back(manager);
    return manager;
}

class BlockingStatsProvider final : public SwapControlProvider {
public:
    int Init(uint64_t, uint32_t) override
    {
        return SWAPFS_E_OK;
    }

    int ReserveSwapOut(uint64_t) override
    {
        return SWAPFS_E_OK;
    }

    void CancelReservedSwapOut(uint64_t) override {}
    void OnSwapOutCommitted(uint64_t, uint64_t) override {}
    void OnEntryRemoved(uint64_t, uint64_t) override {}
    void OnAllEntriesRemoved() override {}

    void FillStats(OH_SwapfsStats &) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        entered_ = true;
        cv_.notify_all();
        cv_.wait(lock, [this] { return released_; });
    }

    void WaitUntilEntered()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return entered_; });
    }

    void Release()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        released_ = true;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool entered_ = false;
    bool released_ = false;
};

bool WaitForDestroyStarted(OH_SwapfsManager *manager)
{
    for (uint32_t attempt = 0; attempt < DESTROY_WAIT_SPINS; ++attempt) {
        {
            std::lock_guard<std::mutex> lock(manager->impl.mutex_);
            if (manager->impl.shuttingDown_) {
                return true;
            }
        }
        std::this_thread::yield();
    }
    return false;
}

OH_SwapfsConfig MakeConfig(bool useDirectIo = false)
{
    OH_SwapfsConfig config;
    config.swapRootPath = TEST_SWAP_BASE;
    config.spaceLimitBytes = 64ULL * 1024ULL * 1024ULL;
    config.useDirectIo = useDirectIo;
    return config;
}

int CountSessionDirs(const std::string &root)
{
    DIR *dir = opendir(root.c_str());
    if (dir == nullptr) {
        return -1;
    }
    int count = 0;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (StartsWith(name, "session-") || StartsWith(name, ".creating-")) {
            ++count;
        }
    }
    (void)closedir(dir);
    return count;
}

HWTEST_F(SwapfsTest, Swapfs_AllFunctionsRejectNullManager_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    EXPECT_EQ(OH_Swapfs_CreateManager(&config, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_DestroyManager(nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_SwapOut(nullptr, nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_SwapIn(nullptr, nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_QueryData(nullptr, 1, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_GetStats(nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_RemoveData(nullptr, 1), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_RemoveAllData(nullptr), SWAPFS_E_INVAL);
}

HWTEST_F(SwapfsTest, Swapfs_NativeFunctionsRejectNullManager_0000, testing::ext::TestSize.Level1)
{
    EXPECT_EQ(SwapfsNativeCreateManager(nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeDestroyManager(nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeSwapOut(nullptr, nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeSwapIn(nullptr, nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeQueryData(nullptr, 1, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeGetStats(nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeRemoveData(nullptr, 1), SWAPFS_E_INVAL);
    EXPECT_EQ(SwapfsNativeRemoveAllData(nullptr), SWAPFS_E_INVAL);
}

HWTEST_F(SwapfsTest, Swapfs_CreateManager_SupportsExplicitAndDefaultQuota_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = nullptr;
    ASSERT_EQ(OH_Swapfs_CreateManager(&config, &manager), SWAPFS_E_OK);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);

    config.spaceLimitBytes = 0;
    manager = nullptr;
    ASSERT_EQ(OH_Swapfs_CreateManager(&config, &manager), SWAPFS_E_OK);
    OH_SwapfsStats stats {};
    ASSERT_EQ(OH_Swapfs_GetStats(manager, &stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.spaceLimitBytes, 1024ULL * 1024ULL * 1024ULL);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
}

HWTEST_F(SwapfsTest, Swapfs_CreateManager_RejectsFileRoot_0000, testing::ext::TestSize.Level1)
{
    (void)unlink(TEST_SWAP_ROOT_FILE);
    int fd = open(TEST_SWAP_ROOT_FILE, O_CREAT | O_CLOEXEC | O_WRONLY, TEST_FILE_MODE);
    ASSERT_GE(fd, 0);
    (void)close(fd);

    OH_SwapfsConfig config = MakeConfig();
    config.swapRootPath = TEST_SWAP_ROOT_FILE;
    OH_SwapfsManager *manager = nullptr;
    EXPECT_EQ(OH_Swapfs_CreateManager(&config, &manager), SWAPFS_E_PATH_UNAVAILABLE);
    EXPECT_EQ(manager, nullptr);
    (void)unlink(TEST_SWAP_ROOT_FILE);
}

HWTEST_F(SwapfsTest, Swapfs_DestroyManager_CleansSessionDirectory_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = nullptr;
    ASSERT_EQ(OH_Swapfs_CreateManager(&config, &manager), SWAPFS_E_OK);
    ASSERT_NE(manager, nullptr);
    ASSERT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 1);

    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
    EXPECT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 0);
}

HWTEST_F(SwapfsTest, Swapfs_DestroyManagerRejectsConcurrentApiCalls_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);

    auto provider = std::make_unique<BlockingStatsProvider>();
    auto *blockingProvider = provider.get();
    manager->impl.control_ = std::move(provider);
    OH_SwapfsStats blockingStats {};
    int statsRet = SWAPFS_E_OK;
    std::thread statsThread([manager, &blockingStats, &statsRet] {
        statsRet = SwapfsNativeGetStats(manager, &blockingStats);
    });

    blockingProvider->WaitUntilEntered();
    EXPECT_EQ(SwapfsNativeDestroyManager(manager), SWAPFS_E_BUSY);
    blockingProvider->Release();
    statsThread.join();
    EXPECT_EQ(statsRet, SWAPFS_E_OK);

    {
        std::lock_guard<std::mutex> lock(manager->impl.mutex_);
        manager->impl.activeOps_ = 1;
    }
    int destroyRet = SWAPFS_E_OK;
    std::thread destroyThread([manager, &destroyRet] {
        destroyRet = OH_Swapfs_DestroyManager(manager);
    });
    bool destroyStarted = WaitForDestroyStarted(manager);
    OH_SwapfsDataInfo info {};
    EXPECT_TRUE(destroyStarted);
    if (destroyStarted) {
        EXPECT_EQ(SwapfsNativeDestroyManager(manager), SWAPFS_E_BUSY);
        EXPECT_EQ(OH_Swapfs_QueryData(manager, 1, &info), SWAPFS_E_SHUTTING_DOWN);
    }
    manager->impl.EndOperation();
    destroyThread.join();
    EXPECT_EQ(destroyRet, SWAPFS_E_OK);

    OH_SwapfsStats stats {};
    EXPECT_EQ(SwapfsNativeDestroyManager(manager), SWAPFS_E_SHUTTING_DOWN);
    EXPECT_EQ(SwapfsNativeGetStats(manager, &stats), SWAPFS_E_SHUTTING_DOWN);
}

HWTEST_F(SwapfsTest, Swapfs_SwapOut_PreservesCapiGuardsAndDioAlignment_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);
    OH_SwapfsSwapOutRequest request { "data", 4 };
    uint64_t keyId = 0;
    EXPECT_EQ(OH_Swapfs_SwapOut(manager, nullptr, &keyId), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_SwapOut(manager, &request, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);

    config = MakeConfig(true);
    manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);
    char buffer[SWAPFS_DIO_ALIGNMENT];
    request = { buffer + 1, SWAPFS_DIO_ALIGNMENT - 1 };
    EXPECT_EQ(OH_Swapfs_SwapOut(manager, &request, &keyId), SWAPFS_E_DIO_ALIGN);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
}

HWTEST_F(SwapfsTest, Swapfs_SwapIn_PreservesCapiGuardsAndDioAlignment_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(OH_Swapfs_SwapIn(manager, nullptr, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);

    config = MakeConfig(true);
    manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);
    char buffer[SWAPFS_DIO_ALIGNMENT];
    OH_SwapfsSwapInRequest request { 1, buffer + 1, SWAPFS_DIO_ALIGNMENT - 1 };
    EXPECT_EQ(OH_Swapfs_SwapIn(manager, &request, nullptr), SWAPFS_E_DIO_ALIGN);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
}

HWTEST_F(SwapfsTest, Swapfs_QueryStatsAndRemove_PreserveCapiGuards_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);

    EXPECT_EQ(OH_Swapfs_QueryData(manager, 1, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_GetStats(manager, nullptr), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_RemoveData(manager, 0), SWAPFS_E_INVAL);
    EXPECT_EQ(OH_Swapfs_RemoveAllData(manager), SWAPFS_E_OK);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
}

HWTEST_F(SwapfsTest, Swapfs_CapiFullLifecycle_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    OH_SwapfsManager *manager = CreateTestManager(&config);
    ASSERT_NE(manager, nullptr);

    const std::string firstPayload = "c api lifecycle first";
    const std::string secondPayload = "c api lifecycle second";
    OH_SwapfsSwapOutRequest outRequest { firstPayload.data(), firstPayload.size() };
    uint64_t firstKey = 0;
    uint64_t secondKey = 0;
    ASSERT_EQ(OH_Swapfs_SwapOut(manager, &outRequest, &firstKey), SWAPFS_E_OK);
    outRequest = { secondPayload.data(), secondPayload.size() };
    ASSERT_EQ(OH_Swapfs_SwapOut(manager, &outRequest, &secondKey), SWAPFS_E_OK);

    OH_SwapfsDataInfo info {};
    ASSERT_EQ(OH_Swapfs_QueryData(manager, firstKey, &info), SWAPFS_E_OK);
    EXPECT_EQ(info.dataSize, firstPayload.size());
    OH_SwapfsStats stats {};
    ASSERT_EQ(OH_Swapfs_GetStats(manager, &stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 2);

    std::vector<char> output(firstPayload.size(), 0);
    OH_SwapfsSwapInRequest inRequest { firstKey, output.data(), output.size() };
    ASSERT_EQ(OH_Swapfs_SwapIn(manager, &inRequest, nullptr), SWAPFS_E_OK);
    EXPECT_EQ(std::string(output.begin(), output.end()), firstPayload);

    EXPECT_EQ(OH_Swapfs_RemoveData(manager, firstKey), SWAPFS_E_OK);
    EXPECT_EQ(OH_Swapfs_QueryData(manager, firstKey, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(OH_Swapfs_RemoveAllData(manager), SWAPFS_E_OK);
    ASSERT_EQ(OH_Swapfs_GetStats(manager, &stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 0);
    EXPECT_EQ(OH_Swapfs_DestroyManager(manager), SWAPFS_E_OK);
}

#undef OH_Swapfs_CreateManager
#undef OH_Swapfs_DestroyManager
} // namespace
