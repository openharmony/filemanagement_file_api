/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hyperaio.h"

#include <chrono>
#include <thread>
#include "accesstoken_kit.h"
#include "hyperaio_trace.h"
#include "libhilog.h"
#include "ipc_skeleton.h"
#ifdef HYPERAIO_USE_LIBURING
#include "liburing.h"
#endif
namespace OHOS {
namespace HyperAio {
#ifdef HYPERAIO_USE_LIBURING
const uint32_t URING_QUEUE_SIZE = 512;
const uint32_t DELAY = 20;
const uint32_t BATCH_SIZE = 128;
const uint32_t RETRIES = 3;
class HyperAio::Impl {
public:
    io_uring uring_;
};

static bool HasAccessIouringPermission()
{
    Security::AccessToken::AccessTokenID tokenCaller = IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ALLOW_IOURING";
    int32_t res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenCaller, permissionName);
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOGE("have no ALLOW_IOURING permission");
        return false;
    }
    return true;
}

uint32_t HyperAio::SupportIouring()
{
    HyperaioTrace trace("SupportIouring");
    uint32_t flags = 0;
    if (HasAccessIouringPermission()) {
        flags |= IOURING_APP_PERMISSION;
    }
    return flags;
}

struct io_uring_sqe* GetSqeWithRetry(struct io_uring *ring)
{
    struct io_uring_sqe *sqe;
    for (uint32_t i = 0; i < RETRIES; i++) {
        sqe = io_uring_get_sqe(ring);
        if (sqe != nullptr) {
            return sqe;
        }
        io_uring_submit(ring);
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    }
    return nullptr;
}

int32_t HyperAio::CtxInit(ProcessIoResultCallBack *callBack)
{
    std::lock_guard<std::mutex> lock(initmtx);
    HyperaioTrace trace("CtxInit");
    if (initialized_.load()) {
        HILOGE("HyperAio has been initialized");
        return EOK;
    }
    if (callBack == nullptr) {
        HILOGE("callBack is null");
        return -EINVAL;
    }
    if (pImpl_ == nullptr) {
        pImpl_ = std::make_shared<Impl>();
    }
    int32_t ret = io_uring_queue_init(URING_QUEUE_SIZE, &pImpl_->uring_, 0);
    if (ret < 0) {
        HILOGE("init io_uring failed, ret = %{public}d", ret);
        return ret;
    }
    ioResultCallBack_ = *callBack;
    stopThread_.store(false);
    harvestThread_ = std::thread(&HyperAio::HarvestRes, this);
    initialized_.store(true);
    HILOGI("init hyperaio success");
    return EOK;
}

int32_t HyperAio::StartOpenReqs(OpenReqs *req)
{
    if (pImpl_ == nullptr) {
        return -EINVAL;
    }
    if (req == nullptr || req->reqs == nullptr) {
        return -EINVAL;
    }
    if (!initialized_.load()) {
        HILOGE("HyperAio is not initialized");
        return -EPERM;
    }
    HyperaioTrace trace("StartOpenReqs" + std::to_string(req->reqNum));
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("get sqe failed");
            return -ENOMEM;
        }
        struct OpenInfo *openInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(openInfo->userData));
        io_uring_prep_openat(sqe, openInfo->dfd, static_cast<const char *>(openInfo->path),
            openInfo->flags, openInfo->mode);
        count++;
        if (count >= BATCH_SIZE) {
            count = 0;
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("submit open reqs failed, ret = %{public}d", ret);
                return ret;
            }
        }
    }
    if (count > 0 && count < BATCH_SIZE) {
        int32_t ret = io_uring_submit(&pImpl_->uring_);
        if (ret < 0) {
            HILOGE("submit open reqs failed, ret = %{public}d", ret);
            return ret;
        }
    }
    return EOK;
}

int32_t HyperAio::StartReadReqs(ReadReqs *req)
{
    if (pImpl_ == nullptr) {
        return -EINVAL;
    }
    HyperaioTrace trace("StartReadReqs" + std::to_string(req->reqNum));
    if (req == nullptr || req->reqs == nullptr) {
        return -EINVAL;
    }
    if (!initialized_.load()) {
        HILOGE("HyperAio is not initialized");
        return -EPERM;
    }
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("get sqe failed");
            return -ENOMEM;
        }
        struct ReadInfo *readInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(readInfo->userData));
        io_uring_prep_read(sqe, readInfo->fd, readInfo->buf, readInfo->len, readInfo->offset);
        count++;
        if (count >= BATCH_SIZE) {
            count = 0;
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("submit read reqs failed, ret = %{public}d", ret);
                return ret;
            }
        }
    }
    if (count > 0 && count < BATCH_SIZE) {
        int32_t ret = io_uring_submit(&pImpl_->uring_);
        if (ret < 0) {
            HILOGE("submit read reqs failed, ret = %{public}d", ret);
            return ret;
        }
    }
    return EOK;
}

int32_t HyperAio::StartCancelReqs(CancelReqs *req)
{
    if (pImpl_ == nullptr) {
        return -EINVAL;
    }
    HyperaioTrace trace("StartCancelReqs" + std::to_string(req->reqNum));
    if (req == nullptr || req->reqs == nullptr) {
        return -EINVAL;
    }
    if (!initialized_.load()) {
        HILOGE("HyperAio is not initialized");
        return -EPERM;
    }
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("get sqe failed");
            return -ENOMEM;
        }
        struct CancelInfo *cancelInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(cancelInfo->userData));
        io_uring_prep_cancel(sqe, reinterpret_cast<void *>(cancelInfo->targetUserData), 0);
        count++;
        if (count >= BATCH_SIZE) {
            count = 0;
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("submit cancel reqs failed, ret = %{public}d", ret);
                return ret;
            }
        }
    }
    if (count > 0 && count < BATCH_SIZE) {
        int32_t ret = io_uring_submit(&pImpl_->uring_);
        if (ret < 0) {
            HILOGE("submit cancel reqs failed, ret = %{public}d", ret);
            return ret;
        }
    }
    return EOK;
}

void HyperAio::HarvestRes()
{
    if (pImpl_ == nullptr) {
        return;
    }
    while (!stopThread_.load()) {
        struct io_uring_cqe *cqe;
        int32_t ret = io_uring_wait_cqe(&pImpl_->uring_, &cqe);
        if (ret < 0 || cqe == nullptr) {
            HILOGI("wait cqe failed, ret = %{public}d", ret);
            continue;
        }
        auto response = std::make_unique<IoResponse>(cqe->user_data, cqe->res, cqe->flags);
        io_uring_cqe_seen(&pImpl_->uring_, cqe);
        if (ioResultCallBack_) {
            ioResultCallBack_(std::move(response));
        }
    }
}

int32_t HyperAio::DestroyCtx()
{
    if (!initialized_.load()) {
        return EOK;
    }
    stopThread_.store(true);
    if (harvestThread_.joinable()) {
        harvestThread_.join();
    }
    if (pImpl_ != nullptr) {
        io_uring_queue_exit(&pImpl_->uring_);
    }
    initialized_.store(false);
    return EOK;
}
#else

uint32_t HyperAio::SupportIouring()
{
    return 0;
}
int32_t HyperAio::CtxInit(ProcessIoResultCallBack *callBack)
{
    return -ENOTSUP;
}
int32_t HyperAio::StartReadReqs(ReadReqs *req)
{
    return -ENOTSUP;
}
int32_t HyperAio::StartOpenReqs(OpenReqs *req)
{
    return -ENOTSUP;
}
int32_t HyperAio::StartCancelReqs(CancelReqs *req)
{
    return -ENOTSUP;
}
int32_t HyperAio::DestroyCtx()
{
    return -ENOTSUP;
}
#endif
}
}