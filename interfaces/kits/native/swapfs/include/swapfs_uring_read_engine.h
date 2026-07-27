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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_URING_READ_ENGINE_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_URING_READ_ENGINE_H

#include <array>
#include <atomic>
#include <cstddef>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#ifdef SWAPFS_USE_LIBURING
#include "liburing.h"
#endif

namespace OHOS::FileManagement::Swapfs {
#ifdef SWAPFS_USE_LIBURING
class UringAdapter {
public:
    virtual ~UringAdapter() = default;
    virtual int QueueInit(unsigned entries, io_uring *ring, unsigned flags) = 0;
    virtual void QueueExit(io_uring *ring) = 0;
    virtual io_uring_sqe *GetSqe(io_uring *ring) = 0;
    virtual int Submit(io_uring *ring) = 0;
    virtual int Cancel(io_uring *ring, uint64_t requestId, uint64_t cancelId) = 0;
    virtual int WaitCqe(io_uring *ring, io_uring_cqe **cqe) = 0;
    virtual void CqeSeen(io_uring *ring, io_uring_cqe *cqe) = 0;
};
#endif

class UringReadEngine {
public:
#ifdef SWAPFS_USE_LIBURING
    explicit UringReadEngine(std::shared_ptr<UringAdapter> adapter = nullptr);
#else
    UringReadEngine();
#endif
    ~UringReadEngine();
    bool IsAvailable();
    int Read(const std::string &path, void *buffer, size_t size, size_t offset);

private:
#ifdef SWAPFS_USE_LIBURING
    struct RingSlot {
        io_uring ring {};
        bool available = false;
        bool inUse = false;
    };

    struct WaitState {
        int error = 0;
        bool rebuild = false;
        bool cancelAttempted = false;
    };

    void InitializePool();
    bool HasAvailableRing() const;
    RingSlot *AcquireSlot();
    void ReleaseSlot(RingSlot &slot);
    void RebuildSlot(RingSlot &slot);
    void HandleWaitFailure(RingSlot &slot, uint64_t requestId, int waitRet, WaitState &state);
    int WaitForCompletion(RingSlot &slot, uint64_t requestId, size_t size, WaitState &state);
    int SubmitRead(RingSlot &slot, int fd, void *buffer, size_t size, size_t offset);

    static constexpr size_t URING_POOL_SIZE = 4;
    std::shared_ptr<UringAdapter> adapter_;
    std::array<RingSlot, URING_POOL_SIZE> slots_;
    std::once_flag initializeFlag_;
    std::mutex poolMutex_;
    std::condition_variable poolCv_;
    std::atomic<uint64_t> nextRequestId_ { 1 };
    bool shuttingDown_ = false;
#endif
};
} // namespace OHOS::FileManagement::Swapfs

#endif
