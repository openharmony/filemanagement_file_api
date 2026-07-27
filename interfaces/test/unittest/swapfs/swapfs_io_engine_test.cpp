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

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <deque>
#include <fcntl.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "swapfs_errcode.h"
#include "swapfs_io_engine.h"
#include "swapfs_syscall_mock.h"
#include "swapfs_uring_read_engine.h"
#include "swapfs.h"

#ifdef SWAPFS_USE_LIBURING
#include "accesstoken_kit_mock.h"
#endif

namespace {
using OHOS::FileManagement::Swapfs::Test::SwapfsSyscallMock;
using OHOS::FileManagement::Swapfs::IsDioAligned;
using OHOS::FileManagement::Swapfs::SyncReadEngine;
using OHOS::FileManagement::Swapfs::SyncWriteEngine;
using OHOS::FileManagement::Swapfs::UringReadEngine;
#ifdef SWAPFS_USE_LIBURING
using OHOS::FileManagement::Swapfs::UringAdapter;
#endif
using testing::_;
using testing::AnyNumber;
using testing::Invoke;
using testing::Return;

constexpr const char *TEST_BASE_DIR = "/data/swapfs_test";
constexpr const char *TEST_FILE_PATH = "/data/swapfs_test/swapfs_io_engine_ut.swap";
constexpr mode_t TEST_DIR_MODE = S_IRWXU;

#ifdef SWAPFS_USE_LIBURING
constexpr mode_t TEST_FILE_MODE = S_IRUSR | S_IWUSR;
constexpr uint32_t CONCURRENT_WAIT_TIMEOUT_MS = 200;

class FakeUringAdapter final : public UringAdapter {
public:
    int QueueInit(unsigned entries, io_uring *ring, unsigned) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++queueInitCalls;
        queueDepths.push_back(entries);
        int result = PopResult(queueInitResults, 0);
        if (result != 0) {
            return result;
        }
        states_[ring] = {};
        return 0;
    }

    void QueueExit(io_uring *) override {}

    io_uring_sqe *GetSqe(io_uring *ring) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (nullSqe) {
            nullSqe = false;
            return nullptr;
        }
        auto &sqe = states_[ring].sqe;
        sqe = {};
        return &sqe;
    }

    int Submit(io_uring *) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++submitCalls;
        return PopResult(submitResults, 1);
    }

    int Cancel(io_uring *, uint64_t, uint64_t) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++cancelCalls;
        return PopResult(cancelResults, 1);
    }

    int WaitCqe(io_uring *ring, io_uring_cqe **cqe) override
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++waitCalls;
        if (!waitResults.empty()) {
            int result = waitResults.front();
            waitResults.pop_front();
            if (result != 0) {
                return result;
            }
        }
        if (nullCqe) {
            nullCqe = false;
            *cqe = nullptr;
            return 0;
        }
        ++activeWaits_;
        maxActiveWaits = std::max(maxActiveWaits, activeWaits_);
        concurrentCv_.notify_all();
        if (waitForConcurrent) {
            (void)concurrentCv_.wait_for(lock,
                std::chrono::milliseconds(CONCURRENT_WAIT_TIMEOUT_MS),
                [this] { return maxActiveWaits >= static_cast<int>(states_.size()); });
        }
        auto &state = states_[ring];
        state.cqe.res = PopResult(cqeResults, static_cast<int>(state.sqe.len));
        state.cqe.user_data = wrongUserData ? state.sqe.user_data + 1 : state.sqe.user_data;
        wrongUserData = false;
        *cqe = &state.cqe;
        --activeWaits_;
        return 0;
    }

    void CqeSeen(io_uring *, io_uring_cqe *) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++cqeSeenCalls;
    }

    std::deque<int> queueInitResults;
    std::deque<int> submitResults;
    std::deque<int> waitResults;
    std::deque<int> cancelResults;
    std::deque<int> cqeResults;
    std::vector<unsigned> queueDepths;
    int queueInitCalls = 0;
    int submitCalls = 0;
    int waitCalls = 0;
    int cancelCalls = 0;
    int cqeSeenCalls = 0;
    int maxActiveWaits = 0;
    bool waitForConcurrent = false;
    bool wrongUserData = false;
    bool nullSqe = false;
    bool nullCqe = false;

private:
    static int PopResult(std::deque<int> &results, int fallback)
    {
        if (results.empty()) {
            return fallback;
        }
        int result = results.front();
        results.pop_front();
        return result;
    }

    struct RingState {
        io_uring_sqe sqe {};
        io_uring_cqe cqe {};
    };

    mutable std::mutex mutex_;
    std::condition_variable concurrentCv_;
    std::map<io_uring *, RingState> states_;
    int activeWaits_ = 0;
};

bool CreateEmptyTestFile()
{
    int fd = open(TEST_FILE_PATH, O_CREAT | O_CLOEXEC | O_WRONLY, TEST_FILE_MODE);
    if (fd < 0) {
        return false;
    }
    (void)close(fd);
    return true;
}

struct TestUring {
    TestUring()
    {
        ring.sq.khead = &head;
        ring.sq.ktail = &tail;
        ring.sq.kflags = &flags;
        ring.sq.sqes = &sqe;
        ring.sq.ring_entries = 1;
        ring.enter_ring_fd = -1;
    }

    io_uring ring {};
    io_uring_sqe sqe {};
    unsigned head = 0;
    unsigned tail = 0;
    unsigned flags = 0;
};
#endif

class SwapfsIoEngineTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
        mkdir(TEST_BASE_DIR, TEST_DIR_MODE);
#ifdef SWAPFS_USE_LIBURING
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::EnableMock();
        auto accessTokenMock =
            OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::GetMock();
        ON_CALL(*accessTokenMock, VerifyAccessToken(testing::_, testing::_))
            .WillByDefault(testing::Return(0));
#endif
    }

    static void TearDownTestSuite()
    {
#ifdef SWAPFS_USE_LIBURING
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::DisableMock();
#endif
    }

    void TearDown() override
    {
        SwapfsSyscallMock::DisableMock();
        (void)unlink(TEST_FILE_PATH);
    }
};

void WritePayload(const std::string &payload)
{
    SyncWriteEngine writer;
    ASSERT_EQ(writer.Write(TEST_FILE_PATH, payload.data(), payload.size(), false),
        SWAPFS_E_OK);
}

bool BytesEqual(const void *left, const void *right, size_t size)
{
    const auto *leftBytes = static_cast<const unsigned char *>(left);
    const auto *rightBytes = static_cast<const unsigned char *>(right);
    return std::equal(leftBytes, leftBytes + size, rightBytes);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_IsDioAlignedChecksAddressAndSize_0000,
    testing::ext::TestSize.Level1)
{
    void *alignedBuffer = nullptr;
    ASSERT_EQ(posix_memalign(&alignedBuffer, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT * 2), 0);
    std::unique_ptr<void, decltype(&free)> alignedBufferGuard(alignedBuffer, &free);

    EXPECT_TRUE(IsDioAligned(alignedBuffer, SWAPFS_DIO_ALIGNMENT));
    EXPECT_FALSE(IsDioAligned(alignedBuffer, SWAPFS_DIO_ALIGNMENT - 1));
    EXPECT_FALSE(IsDioAligned(static_cast<char *>(alignedBuffer) + 1, SWAPFS_DIO_ALIGNMENT));
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncBufferedReadWriteRoundTrip_0000,
    testing::ext::TestSize.Level1)
{
    const std::string payload = "swapfs io engine buffered payload";
    WritePayload(payload);

    std::vector<char> output(payload.size(), 0);
    SyncReadEngine reader;
    ASSERT_EQ(reader.Read(TEST_FILE_PATH, output.data(), output.size(), 0, false),
        SWAPFS_E_OK);
    EXPECT_TRUE(std::equal(output.begin(), output.end(), payload.begin()));
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncDioReadWriteRoundTrip_0000, testing::ext::TestSize.Level1)
{
    void *writeBuf = nullptr;
    ASSERT_EQ(posix_memalign(&writeBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> writeBufGuard(writeBuf, &free);
    std::fill_n(static_cast<unsigned char *>(writeBuf), SWAPFS_DIO_ALIGNMENT,
        static_cast<unsigned char>(0xAB));

    SyncWriteEngine writer;
    ASSERT_EQ(
        writer.Write(TEST_FILE_PATH, writeBuf, SWAPFS_DIO_ALIGNMENT, true),
        SWAPFS_E_OK);

    void *readBuf = nullptr;
    ASSERT_EQ(posix_memalign(&readBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> readBufGuard(readBuf, &free);
    std::fill_n(static_cast<unsigned char *>(readBuf), SWAPFS_DIO_ALIGNMENT, 0);

    SyncReadEngine reader;
    ASSERT_EQ(reader.Read(TEST_FILE_PATH, readBuf, SWAPFS_DIO_ALIGNMENT, 0, true),
        SWAPFS_E_OK);
    EXPECT_TRUE(BytesEqual(readBuf, writeBuf, SWAPFS_DIO_ALIGNMENT));
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncEnginesRejectInvalidParams_0000,
    testing::ext::TestSize.Level1)
{
    SyncWriteEngine writer;
    EXPECT_EQ(writer.Write(TEST_FILE_PATH, nullptr, 10, false), SWAPFS_E_INVAL);

    char buf[16] = {};
    EXPECT_EQ(writer.Write(TEST_FILE_PATH, buf, 0, false), SWAPFS_E_INVAL);

    SyncReadEngine reader;
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, nullptr, 10, 0, false), SWAPFS_E_INVAL);
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, buf, 0, 0, false), SWAPFS_E_INVAL);
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, buf, sizeof(buf), 0, true),
        SWAPFS_E_DIO_ALIGN);

    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT * 2] = {};
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, buffer + 1, SWAPFS_DIO_ALIGNMENT,
        0, true),
        SWAPFS_E_DIO_ALIGN);
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, buffer, SWAPFS_DIO_ALIGNMENT - 1,
        0, true),
        SWAPFS_E_DIO_ALIGN);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncReadHandlesFileBoundaryCases_0000,
    testing::ext::TestSize.Level1)
{
    const std::string payload = "prefix-payload";
    WritePayload(payload);

    SyncReadEngine reader;
    char largeOutput[32] = {};
    EXPECT_EQ(
        reader.Read(TEST_FILE_PATH, largeOutput, sizeof(largeOutput), 0, false),
        SWAPFS_E_IO_ERROR);

    char output[7] = {};
    ASSERT_EQ(reader.Read(TEST_FILE_PATH, output, sizeof(output), 7, false),
        SWAPFS_E_OK);
    EXPECT_EQ(std::string(output, sizeof(output)), "payload");

    const char *badPath = "/data/swapfs_test/nonexistent_file.swap";
    (void)unlink(badPath);
    EXPECT_NE(reader.Read(badPath, output, sizeof(output), 0, false), SWAPFS_E_OK);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncWriteOpenFailure_0000, testing::ext::TestSize.Level1)
{
    constexpr const char *missingDir = "/data/swapfs_test/swapfs_io_missing_ut";
    constexpr const char *missingPath = "/data/swapfs_test/swapfs_io_missing_ut/swapfs.swap";
    (void)rmdir(missingDir);
    const std::string payload = "data";
    SyncWriteEngine writer;

    int ret = writer.Write(missingPath, payload.data(), payload.size(), false);

    EXPECT_EQ(ret, SWAPFS_E_KEY_NOT_FOUND);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncEnginesRetryAfterEintr_0000, testing::ext::TestSize.Level1)
{
    char buffer[16] = {};
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Write(_, _, sizeof(buffer)))
        .WillOnce(Invoke([](int, const void *, size_t) {
            errno = EINTR;
            return -1;
        }))
        .WillOnce(Return(static_cast<ssize_t>(sizeof(buffer))));
    EXPECT_CALL(*mock, Fsync(_)).WillOnce(Return(0));

    SyncWriteEngine writer;
    EXPECT_EQ(
        writer.Write(TEST_FILE_PATH, buffer, sizeof(buffer), false),
        SWAPFS_E_OK);
    char output[16] = {};
    EXPECT_CALL(*mock, Pread(_, _, sizeof(output), 0))
        .WillOnce(Invoke([](int, void *, size_t, off_t) {
            errno = EINTR;
            return -1;
        }))
        .WillOnce(Return(static_cast<ssize_t>(sizeof(output))));

    SyncReadEngine reader;
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, output, sizeof(output), 0, false),
        SWAPFS_E_OK);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncWriteRejectsZeroProgress_0000,
    testing::ext::TestSize.Level1)
{
    char buffer[16] = {};
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Write(_, _, sizeof(buffer))).WillOnce(Return(0));

    SyncWriteEngine writer;
    EXPECT_EQ(
        writer.Write(TEST_FILE_PATH, buffer, sizeof(buffer), false),
        SWAPFS_E_IO_ERROR);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncWriteReportsFsyncFailure_0000,
    testing::ext::TestSize.Level1)
{
    char buffer[16] = {};
    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Write(_, _, _)).Times(AnyNumber()).WillRepeatedly(Return(0));
    EXPECT_CALL(*mock, Write(_, _, sizeof(buffer)))
        .WillOnce(Return(static_cast<ssize_t>(sizeof(buffer))));
    EXPECT_CALL(*mock, Fsync(_)).WillOnce(Invoke([](int) {
        errno = EIO;
        return -1;
    }));

    SyncWriteEngine writer;
    EXPECT_EQ(
        writer.Write(TEST_FILE_PATH, buffer, sizeof(buffer), false),
        SWAPFS_E_IO_ERROR);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_SyncReadReportsPreadFailure_0000,
    testing::ext::TestSize.Level1)
{
    char output[16] = {};
    const std::string payload(sizeof(output), 'x');
    WritePayload(payload);

    auto mock = SwapfsSyscallMock::GetMock();
    SwapfsSyscallMock::EnableMock();
    EXPECT_CALL(*mock, Pread(_, _, sizeof(output), 0))
        .WillOnce(Invoke([](int, void *, size_t, off_t) {
            errno = EIO;
            return -1;
        }));

    SyncReadEngine reader;
    EXPECT_EQ(reader.Read(TEST_FILE_PATH, output, sizeof(output), 0, false),
        SWAPFS_E_IO_ERROR);
}

// ============================ io_uring (UringReadEngine) ============================

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringReadEngineRejectsUnaligned_0000,
    testing::ext::TestSize.Level1)
{
    UringReadEngine uring;
#ifdef SWAPFS_USE_LIBURING
    if (!uring.IsAvailable()) {
        GTEST_SKIP() << "io_uring not available in this environment";
    }
    char buffer[SWAPFS_DIO_ALIGNMENT];
    EXPECT_EQ(
        uring.Read(TEST_FILE_PATH, buffer + 1, SWAPFS_DIO_ALIGNMENT - 1, 0),
        SWAPFS_E_DIO_ALIGN);
#else
    EXPECT_FALSE(uring.IsAvailable());
    char buffer[SWAPFS_DIO_ALIGNMENT];
    EXPECT_EQ(
        uring.Read(TEST_FILE_PATH, buffer, SWAPFS_DIO_ALIGNMENT, 0),
        SWAPFS_E_FEATURE_DISABLED);
#endif
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringReadEngineDioRoundTrip_0000,
    testing::ext::TestSize.Level1)
{
    UringReadEngine uring;
#ifdef SWAPFS_USE_LIBURING
    if (!uring.IsAvailable()) {
        GTEST_SKIP() << "io_uring not available in this environment";
    }

    void *writeBuf = nullptr;
    ASSERT_EQ(posix_memalign(&writeBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> writeBufGuard(writeBuf, &free);
    std::fill_n(static_cast<unsigned char *>(writeBuf), SWAPFS_DIO_ALIGNMENT,
        static_cast<unsigned char>(0x5A));
    SyncWriteEngine writer;
    ASSERT_EQ(
        writer.Write(TEST_FILE_PATH, writeBuf, SWAPFS_DIO_ALIGNMENT, true),
        SWAPFS_E_OK);

    void *readBuf = nullptr;
    ASSERT_EQ(posix_memalign(&readBuf, SWAPFS_DIO_ALIGNMENT, SWAPFS_DIO_ALIGNMENT), 0);
    std::unique_ptr<void, decltype(&free)> readBufGuard(readBuf, &free);
    std::fill_n(static_cast<unsigned char *>(readBuf), SWAPFS_DIO_ALIGNMENT, 0);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, readBuf, SWAPFS_DIO_ALIGNMENT, 0),
        SWAPFS_E_OK);
    EXPECT_TRUE(BytesEqual(readBuf, writeBuf, SWAPFS_DIO_ALIGNMENT));
#else
    GTEST_SKIP() << "SWAPFS_USE_LIBURING not defined, io_uring disabled";
#endif
}

#ifdef SWAPFS_USE_LIBURING
HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolUsesDepth64AndRunsReadsConcurrently_0000,
    testing::ext::TestSize.Level1)
{
    constexpr size_t readCount = 5;
    ASSERT_TRUE(CreateEmptyTestFile());
    auto adapter = std::make_shared<FakeUringAdapter>();
    adapter->waitForConcurrent = true;
    UringReadEngine uring(adapter);
    ASSERT_TRUE(uring.IsAvailable());

    alignas(SWAPFS_DIO_ALIGNMENT)
        std::array<std::array<char, SWAPFS_DIO_ALIGNMENT>, readCount> buffers {};
    std::array<int, readCount> results {};
    std::vector<std::thread> threads;
    for (size_t i = 0; i < readCount; ++i) {
        threads.emplace_back([&, i] {
            results[i] = uring.Read(TEST_FILE_PATH, buffers[i].data(),
                buffers[i].size(), 0);
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(std::all_of(results.begin(), results.end(), [](int ret) {
        return ret == SWAPFS_E_OK;
    }));
    EXPECT_EQ(adapter->maxActiveWaits, 4);
    ASSERT_EQ(adapter->queueDepths.size(), 4);
    EXPECT_TRUE(std::all_of(adapter->queueDepths.begin(), adapter->queueDepths.end(),
        [](unsigned depth) { return depth == 64; }));
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolHandlesSubmitFailures_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(CreateEmptyTestFile());
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};

    adapter->submitResults.push_back(-EINTR);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0), SWAPFS_E_OK);
    EXPECT_EQ(adapter->submitCalls, 2);

    adapter->nullSqe = true;
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    adapter->submitResults.push_back(-EIO);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    adapter->submitResults.push_back(0);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);

    adapter->submitResults.push_back(2);
    adapter->waitResults.push_back(-EIO);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    EXPECT_EQ(adapter->cancelCalls, 1);
    EXPECT_EQ(adapter->queueInitCalls, 8);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolHandlesWaitAndCompletionRouting_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(CreateEmptyTestFile());
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};

    adapter->waitResults.push_back(-EINTR);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0), SWAPFS_E_OK);
    EXPECT_EQ(adapter->waitCalls, 2);

    adapter->wrongUserData = true;
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    EXPECT_EQ(adapter->cqeSeenCalls, 3);
    adapter->waitResults = { -EIO, -EIO };
    adapter->cancelResults = { -EINTR, 0 };
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    EXPECT_EQ(adapter->cancelCalls, 2);

    adapter->nullCqe = true;
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    EXPECT_EQ(adapter->cancelCalls, 3);
    EXPECT_EQ(adapter->queueInitCalls, 7);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0), SWAPFS_E_OK);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolRejectsPermissionAndInitFailures_0000,
    testing::ext::TestSize.Level1)
{
    auto accessTokenMock =
        OHOS::FileManagement::ModuleEnvironment::Test::AccessTokenKitMock::GetMock();
    ON_CALL(*accessTokenMock, VerifyAccessToken(testing::_, testing::_))
        .WillByDefault(Return(-1));
    auto deniedAdapter = std::make_shared<FakeUringAdapter>();
    {
        UringReadEngine denied(deniedAdapter);
        EXPECT_FALSE(denied.IsAvailable());
    }
    ON_CALL(*accessTokenMock, VerifyAccessToken(testing::_, testing::_))
        .WillByDefault(Return(0));
    EXPECT_EQ(deniedAdapter->queueInitCalls, 0);

    auto failedAdapter = std::make_shared<FakeUringAdapter>();
    for (size_t i = 0; i < 4; ++i) {
        failedAdapter->queueInitResults.push_back(-EIO);
    }
    UringReadEngine failed(failedAdapter);
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};
    EXPECT_FALSE(failed.IsAvailable());
    EXPECT_EQ(failed.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_FEATURE_DISABLED);
    EXPECT_EQ(failedAdapter->queueInitCalls, 4);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolMapsCompletionFailures_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(CreateEmptyTestFile());
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};

    adapter->cqeResults.push_back(-EIO);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    adapter->cqeResults.push_back(SWAPFS_DIO_ALIGNMENT - 1);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0), SWAPFS_E_OK);
    EXPECT_EQ(adapter->cqeSeenCalls, 3);
    EXPECT_EQ(adapter->queueInitCalls, 4);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolDisablesFailedRingSlots_0000,
    testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(CreateEmptyTestFile());
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);
    ASSERT_TRUE(uring.IsAvailable());
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};

    for (size_t i = 0; i < 4; ++i) {
        adapter->waitResults.push_back(-EIO);
        adapter->queueInitResults.push_back(-EIO);
        EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
            SWAPFS_E_IO_ERROR);
    }
    EXPECT_EQ(adapter->cancelCalls, 4);
    EXPECT_EQ(adapter->queueInitCalls, 8);
    EXPECT_FALSE(uring.IsAvailable());
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0),
        SWAPFS_E_FEATURE_DISABLED);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringPoolReleasesSlotAfterOpenFailure_0000,
    testing::ext::TestSize.Level1)
{
    constexpr const char *missingPath = "/data/swapfs_test/missing/uring.swap";
    (void)unlink(missingPath);
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);
    alignas(SWAPFS_DIO_ALIGNMENT) char buffer[SWAPFS_DIO_ALIGNMENT] = {};

    EXPECT_EQ(uring.Read(missingPath, buffer, sizeof(buffer), 0),
        SWAPFS_E_IO_ERROR);
    ASSERT_TRUE(CreateEmptyTestFile());
    EXPECT_EQ(uring.Read(TEST_FILE_PATH, buffer, sizeof(buffer), 0), SWAPFS_E_OK);
    EXPECT_EQ(adapter->submitCalls, 1);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringAdapterCancelBuildsCancellationSqe_0000,
    testing::ext::TestSize.Level1)
{
    constexpr uint64_t requestId = 21;
    constexpr uint64_t cancelId = 22;
    UringReadEngine uring;
    TestUring testRing;

    testRing.ring.sq.ring_entries = 0;
    EXPECT_EQ(uring.adapter_->Cancel(&testRing.ring, requestId, cancelId), -EAGAIN);

    testRing.ring.sq.ring_entries = 1;
    EXPECT_EQ(uring.adapter_->Cancel(&testRing.ring, requestId, cancelId), -EBADF);
    EXPECT_EQ(testRing.sqe.opcode, IORING_OP_ASYNC_CANCEL);
    EXPECT_EQ(testRing.sqe.addr, requestId);
    EXPECT_EQ(testRing.sqe.user_data, cancelId);
}

HWTEST_F(SwapfsIoEngineTest, Swapfs_UringAcquireSlotRejectsShutdownAndEmptyPool_0000,
    testing::ext::TestSize.Level1)
{
    auto adapter = std::make_shared<FakeUringAdapter>();
    UringReadEngine uring(adapter);

    uring.shuttingDown_ = true;
    EXPECT_EQ(uring.AcquireSlot(), nullptr);

    uring.shuttingDown_ = false;
    EXPECT_FALSE(uring.HasAvailableRing());
    EXPECT_EQ(uring.AcquireSlot(), nullptr);
}
#endif
} // namespace
