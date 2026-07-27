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

#include "swapfs_uring_read_engine.h"

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <utility>

#include "filemgmt_libhilog.h"
#include "swapfs_err_mapper.h"
#include "swapfs_errcode.h"
#include "swapfs_io_engine.h"

#ifndef O_DIRECT
#define O_DIRECT 0
#endif

#ifdef SWAPFS_USE_LIBURING
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#endif

namespace OHOS::FileManagement::Swapfs {
namespace {
#ifdef SWAPFS_USE_LIBURING
constexpr uint32_t URING_QUEUE_DEPTH = 64;

class LibUringAdapter final : public UringAdapter {
public:
    int QueueInit(unsigned entries, io_uring *ring, unsigned flags) override
    {
        return io_uring_queue_init(entries, ring, flags);
    }

    void QueueExit(io_uring *ring) override
    {
        io_uring_queue_exit(ring);
    }

    io_uring_sqe *GetSqe(io_uring *ring) override
    {
        return io_uring_get_sqe(ring);
    }

    int Submit(io_uring *ring) override
    {
        return io_uring_submit(ring);
    }

    int Cancel(io_uring *ring, uint64_t requestId, uint64_t cancelId) override
    {
        io_uring_sqe *sqe = io_uring_get_sqe(ring);
        if (sqe == nullptr) {
            return -EAGAIN;
        }
        io_uring_prep_cancel64(sqe, requestId, 0);
        io_uring_sqe_set_data64(sqe, cancelId);
        return io_uring_submit(ring);
    }

    int WaitCqe(io_uring *ring, io_uring_cqe **cqe) override
    {
        return io_uring_wait_cqe(ring, cqe);
    }

    void CqeSeen(io_uring *ring, io_uring_cqe *cqe) override
    {
        io_uring_cqe_seen(ring, cqe);
    }
};

bool HasAccessIouringPermission()
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ALLOW_IOURING";
    int32_t res =
        Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permissionName);
    return res == Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}
#endif
} // namespace

#ifdef SWAPFS_USE_LIBURING
UringReadEngine::UringReadEngine(std::shared_ptr<UringAdapter> adapter)
    : adapter_(adapter == nullptr ? std::make_shared<LibUringAdapter>() : std::move(adapter))
{
}
#else
UringReadEngine::UringReadEngine()
{
    HILOGI("[Swapfs] UringReadEngine not available, liburing not enabled");
}
#endif

UringReadEngine::~UringReadEngine()
{
#ifdef SWAPFS_USE_LIBURING
    {
        std::unique_lock<std::mutex> lock(poolMutex_);
        shuttingDown_ = true;
        poolCv_.notify_all();
        poolCv_.wait(lock, [this] {
            return std::none_of(slots_.begin(), slots_.end(), [](const RingSlot &slot) {
                return slot.inUse;
            });
        });
    }
    for (auto &slot : slots_) {
        if (slot.available) {
            adapter_->QueueExit(&slot.ring);
        }
    }
#endif
}

#ifdef SWAPFS_USE_LIBURING
void UringReadEngine::InitializePool()
{
    if (!HasAccessIouringPermission()) {
        HILOGW("[Swapfs] UringReadEngine no ALLOW_IOURING permission, falling back to sync");
        return;
    }
    for (auto &slot : slots_) {
        slot.available = adapter_->QueueInit(URING_QUEUE_DEPTH, &slot.ring, 0) == 0;
    }
    size_t availableCount = static_cast<size_t>(std::count_if(
        slots_.begin(), slots_.end(), [](const RingSlot &slot) { return slot.available; }));
    if (availableCount == 0) {
        HILOGE("[Swapfs] UringReadEngine io_uring_queue_init failed");
        return;
    }
    HILOGI("[Swapfs] UringReadEngine pool init success, rings: %{public}u, depth: %{public}u",
        static_cast<unsigned>(availableCount), URING_QUEUE_DEPTH);
}

bool UringReadEngine::HasAvailableRing() const
{
    return std::any_of(slots_.begin(), slots_.end(), [](const RingSlot &slot) {
        return slot.available;
    });
}

UringReadEngine::RingSlot *UringReadEngine::AcquireSlot()
{
    std::unique_lock<std::mutex> lock(poolMutex_);
    poolCv_.wait(lock, [this] {
        if (shuttingDown_ || !HasAvailableRing()) {
            return true;
        }
        return std::any_of(slots_.begin(), slots_.end(), [](const RingSlot &slot) {
            return slot.available && !slot.inUse;
        });
    });
    if (shuttingDown_ || !HasAvailableRing()) {
        return nullptr;
    }
    auto iter = std::find_if(slots_.begin(), slots_.end(), [](const RingSlot &slot) {
        return slot.available && !slot.inUse;
    });
    iter->inUse = true;
    return &(*iter);
}

void UringReadEngine::ReleaseSlot(RingSlot &slot)
{
    {
        std::lock_guard<std::mutex> lock(poolMutex_);
        slot.inUse = false;
    }
    poolCv_.notify_all();
}

void UringReadEngine::RebuildSlot(RingSlot &slot)
{
    // Rebuild is only entered before submit or after the expected request CQE has been consumed.
    adapter_->QueueExit(&slot.ring);
    slot.ring = {};
    bool rebuilt = adapter_->QueueInit(URING_QUEUE_DEPTH, &slot.ring, 0) == 0;
    {
        std::lock_guard<std::mutex> lock(poolMutex_);
        slot.available = rebuilt;
    }
    if (!rebuilt) {
        HILOGE("[Swapfs] UringReadEngine failed to rebuild ring");
    }
}

int UringReadEngine::SubmitRead(RingSlot &slot, int fd, void *buffer, size_t size, size_t offset)
{
    uint64_t requestId = nextRequestId_.fetch_add(1, std::memory_order_relaxed);
    io_uring_sqe *sqe = adapter_->GetSqe(&slot.ring);
    if (sqe == nullptr) {
        RebuildSlot(slot);
        return SWAPFS_E_IO_ERROR;
    }
    io_uring_prep_read(sqe, fd, buffer, size, static_cast<off_t>(offset));
    io_uring_sqe_set_data64(sqe, requestId);
    int submitRet = 0;
    do {
        submitRet = adapter_->Submit(&slot.ring);
    } while (submitRet == -EINTR);
    if (submitRet <= 0) {
        int error = submitRet < 0
            ? MapErrno(-submitRet, SwapfsErrContext::IO_OPERATION)
            : SWAPFS_E_IO_ERROR;
        RebuildSlot(slot);
        return error;
    }
    WaitState state;
    state.error = submitRet == 1 ? SWAPFS_E_OK : SWAPFS_E_IO_ERROR;
    state.rebuild = submitRet != 1;
    return WaitForCompletion(slot, requestId, size, state);
}

void UringReadEngine::HandleWaitFailure(
    RingSlot &slot, uint64_t requestId, int waitRet, WaitState &state)
{
    if (state.error == SWAPFS_E_OK) {
        state.error = waitRet < 0
            ? MapErrno(-waitRet, SwapfsErrContext::IO_OPERATION)
            : SWAPFS_E_IO_ERROR;
    }
    state.rebuild = true;
    if (state.cancelAttempted) {
        return;
    }
    uint64_t cancelId = nextRequestId_.fetch_add(1, std::memory_order_relaxed);
    int cancelRet = 0;
    do {
        cancelRet = adapter_->Cancel(&slot.ring, requestId, cancelId);
    } while (cancelRet == -EINTR);
    state.cancelAttempted = true;
    if (cancelRet != 1) {
        HILOGE("[Swapfs] UringReadEngine failed to cancel request");
    }
}

int UringReadEngine::WaitForCompletion(
    RingSlot &slot, uint64_t requestId, size_t size, WaitState &state)
{
    for (;;) {
        io_uring_cqe *cqe = nullptr;
        int waitRet = adapter_->WaitCqe(&slot.ring, &cqe);
        if (waitRet == -EINTR) {
            continue;
        }
        if (waitRet < 0 || cqe == nullptr) {
            HandleWaitFailure(slot, requestId, waitRet, state);
            continue;
        }
        int result = cqe->res;
        uint64_t completionId = io_uring_cqe_get_data64(cqe);
        adapter_->CqeSeen(&slot.ring, cqe);
        if (completionId != requestId) {
            state.error = SWAPFS_E_IO_ERROR;
            state.rebuild = true;
            continue;
        }
        if (state.rebuild) {
            RebuildSlot(slot);
        }
        if (state.error != SWAPFS_E_OK) {
            return state.error;
        }
        if (result < 0) {
            return MapErrno(-result, SwapfsErrContext::IO_OPERATION);
        }
        return static_cast<size_t>(result) == size ? SWAPFS_E_OK : SWAPFS_E_IO_ERROR;
    }
}

#endif

bool UringReadEngine::IsAvailable()
{
#ifdef SWAPFS_USE_LIBURING
    std::call_once(initializeFlag_, [this] { InitializePool(); });
    std::lock_guard<std::mutex> lock(poolMutex_);
    return !shuttingDown_ && HasAvailableRing();
#else
    return false;
#endif
}

int UringReadEngine::Read(const std::string &path, void *buffer, size_t size, size_t offset)
{
#ifdef SWAPFS_USE_LIBURING
    if (!IsAvailable()) {
        return SWAPFS_E_FEATURE_DISABLED;
    }
    if (!IsDioAligned(buffer, size)) {
        return SWAPFS_E_DIO_ALIGN;
    }
    RingSlot *slot = AcquireSlot();
    if (slot == nullptr) {
        return SWAPFS_E_FEATURE_DISABLED;
    }
    int fd = OpenSwapFileForRead(path, static_cast<uint32_t>(O_DIRECT));
    if (fd < 0) {
        ReleaseSlot(*slot);
        return MapErrno(errno, SwapfsErrContext::IO_OPERATION);
    }
    int ret = SubmitRead(*slot, fd, buffer, size, offset);
    (void)close(fd);
    ReleaseSlot(*slot);
    return ret;
#else
    (void)path;
    (void)buffer;
    (void)size;
    (void)offset;
    return SWAPFS_E_FEATURE_DISABLED;
#endif
}
} // namespace OHOS::FileManagement::Swapfs
