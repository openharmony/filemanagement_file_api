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
#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include "accesstoken_kit_mock.h"
#include "swapfs_manager.h"
#include "swapfs_session_cleaner.h"
#include "swapfs_storage_manager_mock.h"
#include "swapfs_syscall_mock.h"
#include "tokenid_kit_mock.h"
#include "unique_fd.h"

namespace {
using OHOS::FileManagement::Swapfs::MIN_AVAILABLE_DEVICE_SPACE_BYTES;
using OHOS::FileManagement::Swapfs::ProxySwapControlProvider;
using OHOS::FileManagement::Swapfs::SwapControlProvider;
using OHOS::FileManagement::Swapfs::SwapKeyEntry;
using OHOS::FileManagement::Swapfs::SwapfsSessionCleaner;
using OHOS::FileManagement::Swapfs::SwapfsConfigInner;
using OHOS::FileManagement::Swapfs::SwapfsManager;
using OHOS::FileManagement::Swapfs::Test::CreateStorageManagerProxyMock;
using OHOS::FileManagement::Swapfs::Test::SwapfsSyscallMock;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetErrnoAndReturn;
using testing::StartsWith;
using testing::StrEq;

constexpr const char *TEST_BASE_DIR = "/data/swapfs_test";
constexpr const char *TEST_SWAP_BASE = "/data/swapfs_test/swapfs_manager_ut";
constexpr const char *TEST_SWAP_ROOT = "/data/swapfs_test/swapfs_manager_ut/swapfs";
constexpr const char *TEST_LAYOUT_ROOT = "/data/swapfs_test/swapfs_layout_ut";
constexpr const char *TEST_CREATE_ROOT = "/data/swapfs_test/swapfs_create_ut";
constexpr mode_t TEST_DIR_MODE = S_IRWXU;
constexpr mode_t TEST_FILE_MODE = S_IRUSR | S_IWUSR;
constexpr mode_t NON_OWNER_PERMS = S_IRWXG | S_IRWXO;

bool StartsWith(const std::string &value, const char *prefix)
{
    return value.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

void CreateFile(const std::string &path)
{
    OHOS::UniqueFd fd(open(path.c_str(), O_CREAT | O_CLOEXEC | O_WRONLY, TEST_FILE_MODE));
    ASSERT_GE(fd, 0);
}

bool Exists(const std::string &path)
{
    struct stat st {};
    return lstat(path.c_str(), &st) == 0;
}

OH_SwapfsConfig MakeConfig()
{
    OH_SwapfsConfig config;
    config.swapRootPath = TEST_SWAP_BASE;
    config.spaceLimitBytes = 64ULL * 1024ULL * 1024ULL;
    config.useDirectIo = false;
    return config;
}

SwapfsManager MakeManager()
{
    auto provider = std::make_unique<ProxySwapControlProvider>();
    provider->proxy_ = CreateStorageManagerProxyMock(
        static_cast<int64_t>(MIN_AVAILABLE_DEVICE_SPACE_BYTES + 64ULL * 1024ULL * 1024ULL));
    return SwapfsManager(std::move(provider));
}

class SwapfsManagerTest : public testing::Test {
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
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE, TEST_BASE_DIR);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_LAYOUT_ROOT, TEST_BASE_DIR);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_CREATE_ROOT, TEST_BASE_DIR);
        ASSERT_EQ(mkdir(TEST_SWAP_BASE, TEST_DIR_MODE), 0);
    }

    void TearDown() override
    {
        SwapfsSyscallMock::DisableMock();
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_SWAP_BASE, TEST_BASE_DIR);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_LAYOUT_ROOT, TEST_BASE_DIR);
        (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_CREATE_ROOT, TEST_BASE_DIR);
    }
};

class TestControlProvider final : public SwapControlProvider {
public:
    TestControlProvider(int initRet, int reserveRet) : initRet_(initRet), reserveRet_(reserveRet) {}

    int Init(uint64_t, uint32_t) override
    {
        return initRet_;
    }

    int ReserveSwapOut(uint64_t) override
    {
        return reserveRet_;
    }

    void CancelReservedSwapOut(uint64_t) override {}
    void OnSwapOutCommitted(uint64_t, uint64_t) override {}
    void OnEntryRemoved(uint64_t, uint64_t) override {}
    void OnAllEntriesRemoved() override {}
    void FillStats(OH_SwapfsStats &) override {}

private:
    int initRet_;
    int reserveRet_;
};

int CountSessionDirs(const std::string &root)
{
    DIR *dir = opendir(root.c_str());
    if (dir == nullptr) {
        return 0;
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

std::string FindSwapFile(const std::string &root, uint64_t keyId)
{
    DIR *dir = opendir(root.c_str());
    if (dir == nullptr) {
        return "";
    }
    std::string swapFile;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (!StartsWith(name, "session-")) {
            continue;
        }
        std::string candidate = root + "/" + name + "/data/" + std::to_string(keyId) + ".swap";
        if (access(candidate.c_str(), F_OK) == 0) {
            swapFile = candidate;
            break;
        }
    }
    (void)closedir(dir);
    return swapFile;
}

HWTEST_F(SwapfsManagerTest, Swapfs_SecondInitReturnsBusy_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    EXPECT_EQ(manager.Init(&config), SWAPFS_E_BUSY);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_NullControlProviderFallsBackToProxyProvider_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    std::unique_ptr<SwapControlProvider> provider;
    SwapfsManager manager(std::move(provider));

    EXPECT_EQ(manager.Init(&config), SWAPFS_E_OK);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_DestructorCleansInitializedSession_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    {
        auto manager = MakeManager();
        ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);
        ASSERT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 1);
    }

    EXPECT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 0);
}

HWTEST_F(SwapfsManagerTest, Swapfs_DestroyInvalidatesManagerState_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);

    OH_SwapfsDataInfo info;
    EXPECT_EQ(manager.QueryData(1, &info), SWAPFS_E_INVAL);
    OH_SwapfsStats stats;
    EXPECT_EQ(manager.GetStats(&stats), SWAPFS_E_INVAL);
    EXPECT_EQ(manager.RemoveData(1), SWAPFS_E_INVAL);
    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_INVAL);

    const std::string payload = "after destroy";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_INVAL);

    std::vector<char> output(payload.size(), 0);
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = 1;
    inReq.buffer = output.data();
    inReq.bufferSize = output.size();
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_INVAL);
}

HWTEST_F(SwapfsManagerTest, Swapfs_InitFailureCleansCreatedSession_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();

    auto provider = std::make_unique<TestControlProvider>(SWAPFS_E_IO_ERROR, SWAPFS_E_OK);
    OHOS::FileManagement::Swapfs::SwapfsManager manager(std::move(provider));
    EXPECT_EQ(manager.Init(&config), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 0);
}

// ============================ SwapOut / SwapIn ============================

HWTEST_F(SwapfsManagerTest, Swapfs_BufferedRoundTrip_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "manager buffered round trip";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);
    EXPECT_NE(keyId, 0);

    std::vector<char> output(payload.size(), 0);
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = keyId;
    inReq.buffer = output.data();
    inReq.bufferSize = output.size();
    uint64_t readSize = 0;
    EXPECT_EQ(manager.SwapIn(&inReq, &readSize), SWAPFS_E_OK);
    EXPECT_EQ(readSize, payload.size());
    EXPECT_TRUE(std::equal(output.begin(), output.end(), payload.begin()));

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_DioRoundTrip_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    config.useDirectIo = true;
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    char unalignedBuffer[SWAPFS_DIO_ALIGNMENT + 1] = {};
    OH_SwapfsSwapOutRequest unalignedRequest {
        unalignedBuffer + 1, SWAPFS_DIO_ALIGNMENT
    };
    uint64_t unalignedKeyId = 0;
    EXPECT_EQ(manager.SwapOut(&unalignedRequest, &unalignedKeyId), SWAPFS_E_DIO_ALIGN);

    void *outBuf = nullptr;
    ASSERT_EQ(posix_memalign(&outBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> outBufGuard(outBuf, &free);
    std::fill_n(static_cast<unsigned char *>(outBuf), SWAPFS_DIO_ALIGNMENT,
        static_cast<unsigned char>(0xCD));
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = outBuf;
    outReq.bufferSize = SWAPFS_DIO_ALIGNMENT - 1;
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_DIO_ALIGN);
    outReq.bufferSize = SWAPFS_DIO_ALIGNMENT;
    EXPECT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);

    void *inBuf = nullptr;
    ASSERT_EQ(posix_memalign(&inBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> inBufGuard(inBuf, &free);
    std::fill_n(static_cast<unsigned char *>(inBuf), SWAPFS_DIO_ALIGNMENT, 0);
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = keyId;
    inReq.buffer = inBuf;
    inReq.bufferSize = SWAPFS_DIO_ALIGNMENT;
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_OK);
    uint8_t *bytes = static_cast<uint8_t *>(inBuf);
    for (uint32_t i = 0; i < SWAPFS_DIO_ALIGNMENT; i++) {
        EXPECT_EQ(bytes[i], 0xCD);
    }

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_DioReadFallsBackToSyncWhenUringUnavailable_0000,
    testing::ext::TestSize.Level1)
{
    auto accessTokenMock =
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::GetMock();
    ON_CALL(*accessTokenMock, VerifyAccessToken(_, _)).WillByDefault(Return(-1));
    auto manager = MakeManager();
    SwapKeyEntry entry;
    entry.path = std::string(TEST_SWAP_ROOT) + "/missing.swap";
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] {};

    EXPECT_EQ(manager.ExecuteSwapInRead(entry, buffer, sizeof(buffer), true),
        SWAPFS_E_KEY_NOT_FOUND);

    ON_CALL(*accessTokenMock, VerifyAccessToken(_, _)).WillByDefault(Return(0));
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapInBufferTooSmall_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "buffer too small test payload data";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);

    std::vector<char> output(payload.size() - 1, 0);
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = keyId;
    inReq.buffer = output.data();
    inReq.bufferSize = output.size();
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_BUFFER_TOO_SMALL);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapInKeyNotFound_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    char buf[64];
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = 99999;
    inReq.buffer = buf;
    inReq.bufferSize = sizeof(buf);
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_KEY_NOT_FOUND);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapInReportsMissingBackingFile_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "missing backing file";
    OH_SwapfsSwapOutRequest outRequest { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&outRequest, &keyId), SWAPFS_E_OK);
    std::string swapFile = FindSwapFile(TEST_SWAP_ROOT, keyId);
    ASSERT_FALSE(swapFile.empty());
    ASSERT_EQ(unlink(swapFile.c_str()), 0);

    std::vector<char> output(payload.size(), 0);
    OH_SwapfsSwapInRequest inRequest { keyId, output.data(), output.size() };
    EXPECT_EQ(manager.SwapIn(&inRequest, nullptr), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapOperationsRejectInvalidParams_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(nullptr, &keyId), SWAPFS_E_INVAL);

    OH_SwapfsSwapOutRequest req;
    req.buffer = nullptr;
    req.bufferSize = 10;
    EXPECT_EQ(manager.SwapOut(&req, &keyId), SWAPFS_E_INVAL);

    req.buffer = "data";
    req.bufferSize = 0;
    EXPECT_EQ(manager.SwapOut(&req, &keyId), SWAPFS_E_INVAL);
    req.bufferSize = 4;
    EXPECT_EQ(manager.SwapOut(&req, nullptr), SWAPFS_E_INVAL);

    EXPECT_EQ(manager.SwapIn(nullptr, nullptr), SWAPFS_E_INVAL);

    char buf[16];
    OH_SwapfsSwapInRequest inReq { 0, buf, sizeof(buf) };
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_INVAL);

    inReq.keyId = 1;
    inReq.buffer = nullptr;
    inReq.bufferSize = 10;
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_INVAL);

    inReq.buffer = buf;
    inReq.bufferSize = 0;
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_INVAL);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapOutPropagatesReservationFailure_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto provider = std::make_unique<TestControlProvider>(SWAPFS_E_OK, SWAPFS_E_NOSPC);
    SwapfsManager manager(std::move(provider));
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "reservation rejected";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_NOSPC);
    EXPECT_EQ(keyId, 0);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

// ============================ QueryData / GetStats ============================

HWTEST_F(SwapfsManagerTest, Swapfs_QueryDataAfterSwapOut_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "query data after swapout payload";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);

    OH_SwapfsDataInfo info;
    ASSERT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_OK);
    EXPECT_EQ(info.keyId, keyId);
    EXPECT_EQ(info.dataSize, payload.size());
    EXPECT_EQ(info.status, OH_SWAPFS_KEY_STATUS_ACTIVE);
    EXPECT_TRUE(info.canSwapIn);

    EXPECT_EQ(manager.QueryData(99999, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.QueryData(0, &info), SWAPFS_E_INVAL);
    EXPECT_EQ(manager.QueryData(keyId, nullptr), SWAPFS_E_INVAL);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_GetStatsAfterSwapOut_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "getstats payload data here";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);

    OH_SwapfsStats stats;
    ASSERT_EQ(manager.GetStats(&stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 1);
    EXPECT_EQ(stats.totalDataSize, payload.size());
    EXPECT_EQ(stats.spaceLimitBytes, config.spaceLimitBytes);

    EXPECT_EQ(manager.GetStats(nullptr), SWAPFS_E_INVAL);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

// ============================ RemoveData / RemoveAllData ============================

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveDataMakesKeyUnavailable_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "remove data payload";
    OH_SwapfsSwapOutRequest outReq;
    outReq.buffer = payload.data();
    outReq.bufferSize = payload.size();
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&outReq, &keyId), SWAPFS_E_OK);

    EXPECT_EQ(manager.RemoveData(keyId), SWAPFS_E_OK);
    OH_SwapfsDataInfo info;
    EXPECT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.RemoveData(keyId), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.RemoveData(0), SWAPFS_E_INVAL);

    OH_SwapfsStats stats;
    ASSERT_EQ(manager.GetStats(&stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 0);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveDataRestoresEntryWhenUnlinkFails_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "unlink failure";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_OK);

    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Unlink(_)).WillOnce(Invoke([](const char *) {
        errno = ENOENT;
        return -1;
    }));
    EXPECT_EQ(manager.RemoveData(keyId), SWAPFS_E_KEY_NOT_FOUND);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_OK);
    EXPECT_EQ(info.status, OH_SWAPFS_KEY_STATUS_ACTIVE);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveAllDataClearsEntries_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string first = "removeall first payload";
    const std::string second = "removeall second payload data";
    OH_SwapfsSwapOutRequest outReq;
    uint64_t keyA = 0;
    uint64_t keyB = 0;
    outReq.buffer = first.data();
    outReq.bufferSize = first.size();
    ASSERT_EQ(manager.SwapOut(&outReq, &keyA), SWAPFS_E_OK);
    outReq.buffer = second.data();
    outReq.bufferSize = second.size();
    ASSERT_EQ(manager.SwapOut(&outReq, &keyB), SWAPFS_E_OK);

    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_OK);
    OH_SwapfsDataInfo info;
    EXPECT_EQ(manager.QueryData(keyA, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.QueryData(keyB, &info), SWAPFS_E_KEY_NOT_FOUND);

    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_OK);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveAllDataKeepsOnlyFailedEntry_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "remove all partial failure";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyA = 0;
    uint64_t keyB = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyA), SWAPFS_E_OK);
    ASSERT_EQ(manager.SwapOut(&request, &keyB), SWAPFS_E_OK);

    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Unlink(_))
        .Times(2)
        .WillRepeatedly(Invoke([keyA](const char *path) {
            std::string suffix = "/" + std::to_string(keyA) + ".swap";
            if (std::string(path).find(suffix) != std::string::npos) {
                errno = ENOENT;
                return -1;
            }
            return 0;
        }));
    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_KEY_NOT_FOUND);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(keyA, &info), SWAPFS_E_OK);
    EXPECT_EQ(manager.QueryData(keyB, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveAllDataPreservesFirstErrorAcrossFailures_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "remove all failure";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyA = 0;
    uint64_t keyB = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyA), SWAPFS_E_OK);
    ASSERT_EQ(manager.SwapOut(&request, &keyB), SWAPFS_E_OK);

    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Unlink(_))
        .Times(2)
        .WillRepeatedly(Invoke([](const char *) {
            errno = ENOENT;
            return -1;
        }));
    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_KEY_NOT_FOUND);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(keyA, &info), SWAPFS_E_OK);
    EXPECT_EQ(manager.QueryData(keyB, &info), SWAPFS_E_OK);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_MultipleKeysRoundTrip_0000, testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payloadA = "multi key payload A";
    const std::string payloadB = "multi key payload BB";
    OH_SwapfsSwapOutRequest outReq;
    uint64_t keyA = 0;
    uint64_t keyB = 0;
    outReq.buffer = payloadA.data();
    outReq.bufferSize = payloadA.size();
    ASSERT_EQ(manager.SwapOut(&outReq, &keyA), SWAPFS_E_OK);
    outReq.buffer = payloadB.data();
    outReq.bufferSize = payloadB.size();
    ASSERT_EQ(manager.SwapOut(&outReq, &keyB), SWAPFS_E_OK);
    EXPECT_NE(keyA, keyB);

    std::vector<char> bufA(payloadA.size(), 0);
    OH_SwapfsSwapInRequest inReq;
    inReq.keyId = keyA;
    inReq.buffer = bufA.data();
    inReq.bufferSize = bufA.size();
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_OK);
    EXPECT_TRUE(std::equal(bufA.begin(), bufA.end(), payloadA.begin()));

    EXPECT_EQ(manager.RemoveData(keyA), SWAPFS_E_OK);
    std::vector<char> bufB(payloadB.size(), 0);
    inReq.keyId = keyB;
    inReq.buffer = bufB.data();
    inReq.bufferSize = bufB.size();
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_OK);
    EXPECT_TRUE(std::equal(bufB.begin(), bufB.end(), payloadB.begin()));

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

// ============================ Error paths ============================

HWTEST_F(SwapfsManagerTest, Swapfs_SwapOutWriteFailureCancelsReservation_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "write fail test";
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Write(_, _, payload.size()))
        .Times(2)
        .WillRepeatedly(Invoke([](int, const void *, size_t) {
            errno = EIO;
            return static_cast<ssize_t>(-1);
        }));
    EXPECT_CALL(*mock, Unlink(_))
        .WillOnce(SetErrnoAndReturn(ENOENT, -1))
        .WillOnce(SetErrnoAndReturn(EIO, -1));
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 999;
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(keyId, 999);
    SwapfsSyscallMock::DisableMock();

    uint64_t retryKeyId = 0;
    EXPECT_EQ(manager.SwapOut(&request, &retryKeyId), SWAPFS_E_OK);
    EXPECT_NE(retryKeyId, 0);

    OH_SwapfsStats stats;
    ASSERT_EQ(manager.GetStats(&stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 1);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_SwapOutRenameFailureCleansTmpFile_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "rename fail test";
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Write(_, _, payload.size()))
        .WillOnce(Return(static_cast<ssize_t>(payload.size())));
    EXPECT_CALL(*mock, Fsync(_)).WillOnce(Return(0));
    EXPECT_CALL(*mock, Unlink(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mock, Rename(_, _))
        .WillOnce(Invoke([](const char *, const char *) {
            errno = EIO;
            return -1;
        }));
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_IO_ERROR);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsStats stats;
    ASSERT_EQ(manager.GetStats(&stats), SWAPFS_E_OK);
    EXPECT_EQ(stats.totalKeys, 0);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveSessionDirRejectsPathOutsideRoot_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_SWAP_BASE;
    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);

    manager.sessionPath_ = TEST_LAYOUT_ROOT;
    manager.RemoveSessionDir();
    EXPECT_TRUE(manager.sessionPath_.empty());
    EXPECT_TRUE(Exists(TEST_LAYOUT_ROOT));
}

HWTEST_F(SwapfsManagerTest, Swapfs_InitReturnsBusyWhenCleanupLockRemainsHeld_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    manager.ResolveConfig(&config, manager.config_);
    ASSERT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_OK);
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;
    ASSERT_EQ(cleaner.AcquireCleanupLock(rootFd, cleanupLock), SWAPFS_E_OK);

    EXPECT_EQ(manager.Init(&config), SWAPFS_E_BUSY);

    cleanupLock = OHOS::UniqueFd();
    EXPECT_EQ(manager.Init(&config), SWAPFS_E_OK);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_OperationsRejectWhenShuttingDown_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    manager.shuttingDown_ = true;
    const std::string payload = "shutdown reject";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_SHUTTING_DOWN);

    char buf[16];
    OH_SwapfsSwapInRequest inRequest { 1, buf, sizeof(buf) };
    EXPECT_EQ(manager.SwapIn(&inRequest, nullptr), SWAPFS_E_SHUTTING_DOWN);
    EXPECT_EQ(manager.RemoveData(1), SWAPFS_E_SHUTTING_DOWN);
    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_SHUTTING_DOWN);
    OH_SwapfsStats stats {};
    EXPECT_EQ(manager.GetStats(&stats), SWAPFS_E_SHUTTING_DOWN);
    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(1, &info), SWAPFS_E_SHUTTING_DOWN);
    manager.shuttingDown_ = false;

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_OperationsRejectInvalidKeyState_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "state invalid test";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_OK);

    manager.entries_[keyId].status = OH_SWAPFS_KEY_STATUS_REMOVING;
    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_KEY_STATE_INVALID);
    std::vector<char> buf(payload.size(), 0);
    OH_SwapfsSwapInRequest inReq { keyId, buf.data(), buf.size() };
    EXPECT_EQ(manager.SwapIn(&inReq, nullptr), SWAPFS_E_KEY_STATE_INVALID);
    EXPECT_EQ(manager.RemoveData(keyId), SWAPFS_E_KEY_STATE_INVALID);
    EXPECT_EQ(manager.activeOps_, 0u);

    manager.entries_[keyId].status = OH_SWAPFS_KEY_STATUS_ACTIVE;
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_FinishSwapInHandlesConcurrentEntryRemoval_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "deferred delete test";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_OK);

    manager.entries_[keyId].status = OH_SWAPFS_KEY_STATUS_REMOVING;
    manager.entries_[keyId].readCount = 1;
    auto mock = SwapfsSyscallMock::GetMock();
    auto *managerPtr = &manager;
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Unlink(_))
        .WillOnce(Invoke([managerPtr, keyId](const char *) {
            managerPtr->entries_.erase(keyId);
            return 0;
        }));
    manager.FinishSwapIn(keyId);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsDataInfo info;
    EXPECT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_KEY_NOT_FOUND);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_FinishSwapInDeferredDeleteUnlinkFailReverts_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    const std::string payload = "deferred unlink fail";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_OK);

    manager.entries_[keyId].status = OH_SWAPFS_KEY_STATUS_REMOVING;
    manager.entries_[keyId].readCount = 1;

    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Unlink(_))
        .WillOnce(Invoke([](const char *) {
            errno = EIO;
            return -1;
        }));
    manager.FinishSwapIn(keyId);
    SwapfsSyscallMock::DisableMock();

    OH_SwapfsDataInfo info;
    ASSERT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_OK);
    EXPECT_EQ(info.status, OH_SWAPFS_KEY_STATUS_ACTIVE);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_BusyStateGuardsRejectOperations_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);

    manager.activeOps_ = 1;
    EXPECT_EQ(manager.RemoveAllData(), SWAPFS_E_BUSY);
    manager.activeOps_ = 0;

    manager.removeAllInProgress_ = true;
    const std::string payload = "removeall in progress";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    EXPECT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_BUSY);
    manager.removeAllInProgress_ = false;

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_InitRejectsNonSystemApplication_0000,
    testing::ext::TestSize.Level1)
{
    auto tokenIdMock = OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::GetMock();
    ON_CALL(*tokenIdMock, IsSystemAppByFullTokenID(testing::_))
        .WillByDefault(testing::Return(false));
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();

    EXPECT_EQ(manager.Init(&config), 202);

    ON_CALL(*tokenIdMock, IsSystemAppByFullTokenID(testing::_))
        .WillByDefault(testing::Return(true));
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_ResolveConfigUsesDefaultsForMissingFields_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    SwapfsConfigInner inner;
    manager.ResolveConfig(nullptr, inner);
    EXPECT_EQ(inner.swapRootPath, OHOS::FileManagement::Swapfs::DEFAULT_SWAPFS_ROOT_PATH);
    EXPECT_EQ(inner.spaceLimitBytes, OHOS::FileManagement::Swapfs::DEFAULT_SPACE_LIMIT_BYTES);
    EXPECT_FALSE(inner.useDirectIo);
    EXPECT_FALSE(inner.managerId.empty());

    OH_SwapfsConfig config { "", 0, true };
    manager.ResolveConfig(&config, inner);
    EXPECT_EQ(inner.swapRootPath, OHOS::FileManagement::Swapfs::DEFAULT_SWAPFS_ROOT_PATH);
    EXPECT_EQ(inner.spaceLimitBytes, OHOS::FileManagement::Swapfs::DEFAULT_SPACE_LIMIT_BYTES);
    EXPECT_TRUE(inner.useDirectIo);

    config.swapRootPath = nullptr;
    manager.ResolveConfig(&config, inner);
    EXPECT_EQ(inner.swapRootPath, OHOS::FileManagement::Swapfs::DEFAULT_SWAPFS_ROOT_PATH);

    std::string baseWithSlash = std::string(TEST_SWAP_BASE) + "/";
    config.swapRootPath = baseWithSlash.c_str();
    manager.ResolveConfig(&config, inner);
    EXPECT_EQ(inner.swapRootPath, TEST_SWAP_ROOT);

    config.swapRootPath = "/";
    manager.ResolveConfig(&config, inner);
    EXPECT_EQ(inner.swapRootPath, "/swapfs");
}

HWTEST_F(SwapfsManagerTest, Swapfs_WaitForActiveOperationsHandlesTimeout_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    EXPECT_TRUE(manager.AllEntriesCleanLocked());
    manager.activeOps_ = 1;

    EXPECT_EQ(manager.activeOps_, 1u);
    EXPECT_FALSE(manager.AllEntriesCleanLocked());
    EXPECT_FALSE(manager.WaitForActiveOps(0));

    manager.activeOps_ = 0;
    EXPECT_TRUE(manager.AllEntriesCleanLocked());
}

HWTEST_F(SwapfsManagerTest, Swapfs_RemoveDataDefersDeletionWhileEntryIsBeingRead_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);
    const std::string payload = "deferred remove";
    OH_SwapfsSwapOutRequest request { payload.data(), payload.size() };
    uint64_t keyId = 0;
    ASSERT_EQ(manager.SwapOut(&request, &keyId), SWAPFS_E_OK);
    manager.entries_[keyId].readCount = 1;

    EXPECT_EQ(manager.RemoveData(keyId), SWAPFS_E_OK);
    EXPECT_EQ(manager.entries_[keyId].status, 1);
    EXPECT_EQ(manager.activeOps_, 0u);

    manager.FinishSwapIn(keyId);
    OH_SwapfsDataInfo info {};
    EXPECT_EQ(manager.QueryData(keyId, &info), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSwapRootCreatesOwnedFixedChild_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    std::string aliasedBase = "/data/swapfs_test/../swapfs_test/swapfs_manager_ut";
    config.swapRootPath = aliasedBase.c_str();
    auto manager = MakeManager();
    manager.ResolveConfig(&config, manager.config_);

    ASSERT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_OK);
    EXPECT_EQ(manager.config_.swapRootPath, TEST_SWAP_ROOT);
    EXPECT_TRUE(Exists(std::string(TEST_SWAP_ROOT) + "/.swapfs-root"));

    struct stat st {};
    ASSERT_EQ(lstat(TEST_SWAP_ROOT, &st), 0);
    EXPECT_EQ(st.st_mode & NON_OWNER_PERMS, 0u);
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSwapRootHandlesDefaultWorkDirectory_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.config_.swapRootPath = OHOS::FileManagement::Swapfs::DEFAULT_SWAPFS_ROOT_PATH;
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Mkdir(StrEq(OHOS::FileManagement::Swapfs::DEFAULT_SWAPFS_TEMP_PATH), _))
        .WillOnce(SetErrnoAndReturn(EIO, -1));

    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_IO_ERROR);
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSwapRootRejectsInvalidExistingRoots_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.config_.swapRootPath.clear();
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_INVAL);

    manager.config_.swapRootPath = "relative/swapfs";
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_INVAL);
    manager.config_.swapRootPath = "/" + std::string(PATH_MAX, 'a');
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_INVAL);

    manager.config_.swapRootPath = "/proc";
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    manager.config_.swapRootPath = std::string(TEST_LAYOUT_ROOT) + "/missing/swapfs";
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);
    manager.config_.swapRootPath = TEST_LAYOUT_ROOT;
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    CreateFile(std::string(TEST_LAYOUT_ROOT) + "/.swapfs-root");
    ASSERT_EQ(chmod(TEST_LAYOUT_ROOT, 0755), 0);
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_LAYOUT_ROOT, TEST_BASE_DIR);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Mkdir(StrEq(TEST_LAYOUT_ROOT), _))
        .WillOnce(Return(0)).RetiresOnSaturation();
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);
    testing::Mock::VerifyAndClearExpectations(mock.get());
    SwapfsSyscallMock::DisableMock();

    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);
    mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Mkdir(StrEq(TEST_LAYOUT_ROOT), _))
        .WillOnce(Return(0));
    EXPECT_CALL(*mock, OpenAt(_, StrEq(".swapfs-root"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1));
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_IO_ERROR);
    EXPECT_FALSE(Exists(TEST_LAYOUT_ROOT));
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSwapRootRejectsSymlink_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_EQ(mkdir(TEST_CREATE_ROOT, TEST_DIR_MODE), 0);
    CreateFile(std::string(TEST_CREATE_ROOT) + "/sentinel");
    ASSERT_EQ(symlink(TEST_CREATE_ROOT, TEST_LAYOUT_ROOT), 0);
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_LAYOUT_ROOT;

    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    EXPECT_TRUE(Exists(std::string(TEST_CREATE_ROOT) + "/sentinel"));
    EXPECT_TRUE(Exists(TEST_LAYOUT_ROOT));
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSwapRootRejectsInvalidRootMarkers_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);
    ASSERT_EQ(mkdir((std::string(TEST_LAYOUT_ROOT) + "/.swapfs-root").c_str(), TEST_DIR_MODE), 0);
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_LAYOUT_ROOT;

    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_LAYOUT_ROOT, TEST_BASE_DIR);
    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);
    std::string marker = std::string(TEST_LAYOUT_ROOT) + "/.swapfs-root";
    CreateFile(marker);
    ASSERT_EQ(chmod(marker.c_str(), 0644), 0);
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);

    (void)SwapfsSessionCleaner::RemoveSessionTree(TEST_LAYOUT_ROOT, TEST_BASE_DIR);
    ASSERT_EQ(mkdir(TEST_LAYOUT_ROOT, TEST_DIR_MODE), 0);
    CreateFile(std::string(TEST_LAYOUT_ROOT) + "/target");
    ASSERT_EQ(symlink("target", marker.c_str()), 0);
    EXPECT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_PATH_UNAVAILABLE);
}

HWTEST_F(SwapfsManagerTest, Swapfs_InitDoesNotScanParentLevelOldSessions_0000,
    testing::ext::TestSize.Level1)
{
    std::string oldSession = std::string(TEST_SWAP_BASE) + "/session-old-layout";
    ASSERT_EQ(mkdir(oldSession.c_str(), TEST_DIR_MODE), 0);
    CreateFile(oldSession + "/.swapfs-session");
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();

    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);
    EXPECT_TRUE(Exists(oldSession));
    EXPECT_EQ(CountSessionDirs(TEST_SWAP_ROOT), 1);

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
    EXPECT_TRUE(Exists(oldSession));
}

HWTEST_F(SwapfsManagerTest, Swapfs_CreateSessionLockedRollsBackFailures_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.config_.swapRootPath = std::string(TEST_CREATE_ROOT) + "/missing";
    manager.config_.managerId = "open-failure";
    EXPECT_NE(manager.CreateSessionLocked(-1), SWAPFS_E_OK);

    ASSERT_EQ(mkdir(TEST_CREATE_ROOT, TEST_DIR_MODE), 0);
    OHOS::UniqueFd rootFd(open(TEST_CREATE_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    manager.config_.swapRootPath = TEST_CREATE_ROOT;
    manager.config_.managerId = "missing/child";
    EXPECT_NE(manager.CreateSessionLocked(rootFd), SWAPFS_E_OK);
    EXPECT_TRUE(manager.sessionPath_.empty());

    manager.config_.managerId = "rename-failure";
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, RenameAt(_, _, _, _))
        .WillOnce(Invoke([](int, const char *, int, const char *) {
            errno = EIO;
            return -1;
        }));
    EXPECT_NE(manager.CreateSessionLocked(rootFd), SWAPFS_E_OK);
    EXPECT_EQ(manager.sessionLockFd_, -1);
    EXPECT_TRUE(manager.sessionPath_.empty());
}

HWTEST_F(SwapfsManagerTest, Swapfs_CreateSessionLockedHandlesSessionLockFailure_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_EQ(mkdir(TEST_CREATE_ROOT, TEST_DIR_MODE), 0);
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_CREATE_ROOT;
    manager.config_.managerId = "flock-failure";
    OHOS::UniqueFd rootFd(open(TEST_CREATE_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Flock(_, LOCK_UN))
        .WillRepeatedly(Return(0));
    EXPECT_CALL(*mock, Flock(_, LOCK_EX | LOCK_NB))
        .WillOnce(Invoke([](int, int) {
            errno = EIO;
            return -1;
        }));

    EXPECT_NE(manager.CreateSessionLocked(rootFd), SWAPFS_E_OK);

    EXPECT_EQ(manager.sessionLockFd_, -1);
    EXPECT_TRUE(manager.sessionPath_.empty());
    EXPECT_EQ(CountSessionDirs(TEST_CREATE_ROOT), 0);
}

HWTEST_F(SwapfsManagerTest, Swapfs_CreateSessionRollsBackInjectedAtFailures_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_EQ(mkdir(TEST_CREATE_ROOT, TEST_DIR_MODE), 0);
    OHOS::UniqueFd rootFd(open(TEST_CREATE_ROOT, O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    ASSERT_GE(rootFd, 0);
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_CREATE_ROOT;
    manager.config_.managerId = "injected-failure";
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, MkdirAt(_, StartsWith(".creating-"), _))
        .Times(testing::AnyNumber());
    EXPECT_CALL(*mock, OpenAt(_, StartsWith(".creating-"), _, _))
        .Times(testing::AnyNumber());
    EXPECT_CALL(*mock, OpenAt(_, StrEq(".swapfs-session"), _, _))
        .Times(testing::AnyNumber());
    EXPECT_CALL(*mock, MkdirAt(_, StrEq("data"), _))
        .Times(testing::AnyNumber());
    EXPECT_CALL(*mock, OpenAt(_, StrEq("data"), _, _))
        .Times(testing::AnyNumber());
    EXPECT_CALL(*mock, OpenAt(_, StrEq("session.lock"), _, _))
        .Times(testing::AnyNumber());

    EXPECT_CALL(*mock, OpenAt(_, StartsWith(".creating-"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_EQ(manager.CreateSessionLocked(rootFd), SWAPFS_E_IO_ERROR);
    EXPECT_CALL(*mock, OpenAt(_, StrEq(".swapfs-session"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_EQ(manager.CreateSessionLocked(rootFd), SWAPFS_E_IO_ERROR);
    EXPECT_CALL(*mock, MkdirAt(_, StrEq("data"), _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_EQ(manager.CreateSessionLocked(rootFd), SWAPFS_E_IO_ERROR);
    EXPECT_CALL(*mock, OpenAt(_, StrEq("session.lock"), _, _))
        .WillOnce(SetErrnoAndReturn(EIO, -1)).RetiresOnSaturation();
    EXPECT_EQ(manager.CreateSessionLocked(rootFd), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(CountSessionDirs(TEST_CREATE_ROOT), 0);
}

HWTEST_F(SwapfsManagerTest, Swapfs_PrepareSessionPropagatesCleanupFailure_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.config_.swapRootPath = TEST_SWAP_ROOT;
    ASSERT_EQ(manager.PrepareSwapRoot(), SWAPFS_E_OK);
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Dup(_)).WillOnce(SetErrnoAndReturn(EMFILE, -1));

    EXPECT_EQ(manager.PrepareSession(), SWAPFS_E_PATH_UNAVAILABLE);

    EXPECT_TRUE(manager.sessionPath_.empty());
}

HWTEST_F(SwapfsManagerTest, Swapfs_CleanupLockProtectsPartiallyCreatedSession_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto first = MakeManager();
    first.ResolveConfig(&config, first.config_);
    ASSERT_EQ(first.PrepareSwapRoot(), SWAPFS_E_OK);
    SwapfsSessionCleaner cleaner(TEST_SWAP_ROOT);
    OHOS::UniqueFd rootFd;
    OHOS::UniqueFd cleanupLock;
    ASSERT_EQ(cleaner.AcquireCleanupLock(rootFd, cleanupLock), SWAPFS_E_OK);
    std::string partial = std::string(TEST_SWAP_ROOT) + "/.creating-paused";
    ASSERT_EQ(mkdir(partial.c_str(), TEST_DIR_MODE), 0);

    auto second = MakeManager();
    std::atomic<bool> started = false;
    int initRet = SWAPFS_E_BUSY;
    std::thread createThread([&second, &config, &started, &initRet] {
        OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::EnableMock();
        auto tokenIdMock =
            OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::GetMock();
        ON_CALL(*tokenIdMock, IsSystemAppByFullTokenID(testing::_))
            .WillByDefault(testing::Return(true));
        started = true;
        initRet = second.Init(&config);
        OHOS::FileManagement::ModuleEnvironment::Test::TokenIdKitMock::DisableMock();
    });
    while (!started) {
        std::this_thread::yield();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(Exists(partial));
    cleanupLock = OHOS::UniqueFd();
    createThread.join();

    EXPECT_EQ(initRet, SWAPFS_E_OK);
    EXPECT_FALSE(Exists(partial));
    EXPECT_EQ(second.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_DestroyReturnsBusyWhileOperationRemainsActive_0000,
    testing::ext::TestSize.Level1)
{
    OH_SwapfsConfig config = MakeConfig();
    auto manager = MakeManager();
    ASSERT_EQ(manager.Init(&config), SWAPFS_E_OK);
    manager.activeOps_ = 1;

    EXPECT_EQ(manager.Destroy(), SWAPFS_E_BUSY);
    EXPECT_FALSE(manager.shuttingDown_);

    manager.activeOps_ = 0;
    EXPECT_EQ(manager.Destroy(), SWAPFS_E_OK);
}

HWTEST_F(SwapfsManagerTest, Swapfs_EmptyHelperPathsRemainIdempotent_0000,
    testing::ext::TestSize.Level1)
{
    auto manager = MakeManager();
    manager.CloseSessionLock();
    manager.RemoveSessionDir();

    EXPECT_EQ(manager.sessionLockFd_, -1);
    EXPECT_TRUE(manager.sessionPath_.empty());
    EXPECT_FALSE(manager.initialized_);

    SwapKeyEntry entry;
    entry.keyId = 123;
    entry.dataSize = 4;
    manager.FinishSwapIn(entry.keyId);
    manager.FinalizeRemoveEntry(entry, true);
    EXPECT_TRUE(manager.entries_.empty());
}

} // namespace
