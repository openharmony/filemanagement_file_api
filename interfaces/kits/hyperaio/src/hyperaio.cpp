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
        HILOGE("[HyperAio]have no ALLOW_IOURING permission, res = %{public}d", res);
        return false;
    }

    return true;
}

static bool ValidateReqNum(uint32_t reqNum)
{
    return reqNum > 0 && reqNum <= URING_QUEUE_SIZE;
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
    int ret = 0;
    struct io_uring_sqe *sqe;
    for (uint32_t i = 0; i < RETRIES; i++) {
        sqe = io_uring_get_sqe(ring);
        if (sqe != nullptr) {
            return sqe;
        }
        
        ret = io_uring_submit(ring);
        if (ret < 0) {
            HILOGE("[HyperAio] submit existing reqs failed , ret = %{public}d, times = %{public}d", ret, i);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
    }
    return nullptr;
}

int32_t HyperAio::CtxInit(ProcessIoResultCallBack *callBack)
{
    HyperaioTrace trace("CtxInit");
    if (initialized_.load()) {
        HILOGE("[HyperAio] HyperAio has been initialized");
        return EOK;
    }

    if (callBack == nullptr) {
        HILOGE("[HyperAio] callBack is null");
        return -EINVAL;
    }

    if (pImpl_ == nullptr) {
        pImpl_ = std::make_shared<Impl>();
    }

    int32_t ret = io_uring_queue_init(URING_QUEUE_SIZE, &pImpl_->uring_, 0);
    if (ret < 0) {
        HILOGE("[HyperAio] init io_uring failed, ret = %{public}d", ret);
        return ret;
    }

    ioResultCallBack_ = *callBack;
    stopThread_.store(false);
    harvestThread_ = std::thread(&HyperAio::HarvestRes, this);
    initialized_.store(true);
    HILOGI("[HyperAio] init hyperaio success");
    return EOK;
}

void HyperAio::HandleRequestError(std::vector<uint64_t> &errorVec, int32_t errorcode)
{
    if (errorVec.empty()) {
        HILOGE("[HyperAio] errorVec is empty");
        return;
    }
    for (auto &userdata : errorVec) {
        HILOGE("[HyperAio] HandleRequestError: userData = %{private}lu", userdata);
        auto response = std::make_unique<IoResponse>(userdata, errorcode, 0);
        ioResultCallBack_(std::move(response));
    }
    errorVec.clear();
}

void HyperAio::HandleSqeError(uint32_t count, std::vector<uint64_t> &infoVec)
{
    if (count > 0) {
        int32_t ret = io_uring_submit(&pImpl_->uring_);
        if (ret < 0) {
            HILOGE("[HyperAio] submit remaining reqs failed, ret = %{public}d", ret);
            HandleRequestError(infoVec, ret);
        }
        readReqCount_ += count;
    }
}

int32_t HyperAio::CheckParameter(uint32_t reqNum)
{
    if (pImpl_ == nullptr) {
        HILOGE("[HyperAio] pImpl is not initialized");
        return -EINVAL;
    }
    if (!initialized_.load()) {
        HILOGE("[HyperAio] HyperAio is not initialized");
        return -EINVAL;
    }
    if (destroyed_.load()) {
        HILOGE("[HyperAio] HyperAio is destroyed");
        return -EINVAL;
    }
    if (!ValidateReqNum(reqNum)) {
        HILOGE("[HyperAio] reqNum is out of range: %{public}u", reqNum);
        return -EINVAL;
    }
    return EOK;
}

int32_t HyperAio::StartOpenReqs(OpenReqs *req)
{
    if (req == nullptr || req->reqs == nullptr) {
        HILOGE("[HyperAio] the request is empty");
        return -EINVAL;
    }
    int32_t ret = CheckParameter(req->reqNum);
    if (ret < 0) {
        return ret;
    }
    HyperaioTrace trace("StartOpenReqs" + std::to_string(req->reqNum));
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    std::vector<uint64_t> errorVec;
    std::vector<uint64_t> openInfoVec;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("[HyperAio] get sqe failed");
            for (; i < totalReqs; ++i) {
                errorVec.push_back(req->reqs[i].userData);
            }
            HandleSqeError(count, openInfoVec);
            HandleRequestError(errorVec, -EBUSY);
            break;
        }
        struct OpenInfo *openInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(openInfo->userData));
        io_uring_prep_openat(sqe, openInfo->dfd, static_cast<const char *>(openInfo->path),
            openInfo->flags, openInfo->mode);
        HILOGD("[HyperAio] open flags = %{public}d, mode = %{public}u, userData = %{private}lu",
            openInfo->flags, openInfo->mode, openInfo->userData);
        HyperaioTrace trace("open flags:" + std::to_string(openInfo->flags) + "mode:" + std::to_string(openInfo->mode)
            + "userData:" + std::to_string(openInfo->userData));
        count++;
        openInfoVec.push_back(openInfo->userData);
        if (count >= BATCH_SIZE || i == totalReqs - 1) {
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("[HyperAio] submit open reqs failed, ret = %{public}d", ret);
                HandleRequestError(openInfoVec, -EBUSY);
            }
            HILOGI("[HyperAio] submit open reqs success, num = %{public}d", count);
            openReqCount_ += count;
            std::unique_lock<std::mutex> lock(cqeMutex_);
            pendingCqeCount_ += count;
            cqeCond_.notify_one();
            count = 0;
        }
    }
    return EOK;
}

int32_t HyperAio::StartReadReqs(ReadReqs *req)
{
    if (req == nullptr || req->reqs == nullptr) {
        HILOGE("[HyperAio] the request is empty");
        return -EINVAL;
    }
    int32_t ret = CheckParameter(req->reqNum);
    if (ret < 0) {
        return ret;
    }
    HyperaioTrace trace("StartReadReqs" + std::to_string(req->reqNum));
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    std::vector<uint64_t> errorVec;
    std::vector<uint64_t> readInfoVec;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("[HyperAio] get sqe failed");
            for (; i < totalReqs; ++i) {
                errorVec.push_back(req->reqs[i].userData);
            }
            HandleSqeError(count, readInfoVec);
            HandleRequestError(errorVec, -EBUSY);
            break;
        }
        struct ReadInfo *readInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(readInfo->userData));
        io_uring_prep_read(sqe, readInfo->fd, readInfo->buf, readInfo->len, readInfo->offset);
        HILOGD("[HyperAio] read len = %{public}u, offset = %{public}lu, userData = %{private}lu",
            readInfo->len, readInfo->offset, readInfo->userData);
        HyperaioTrace trace("read len:" + std::to_string(readInfo->len) + "offset:" + std::to_string(readInfo->offset)
            + "userData:" + std::to_string(readInfo->userData));
        count++;
        readInfoVec.push_back(readInfo->userData);
        if (count >= BATCH_SIZE || i == totalReqs - 1) {
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("[HyperAio] submit read reqs failed, ret = %{public}d", ret);
                HandleRequestError(readInfoVec, -EBUSY);
            }
            HILOGI("[HyperAio] submit read reqs success, num = %{public}d", count);
            readReqCount_ += count;
            std::unique_lock<std::mutex> lock(cqeMutex_);
            pendingCqeCount_ += count;
            cqeCond_.notify_one();
            count = 0;
        }
    }
    return EOK;
}

int32_t HyperAio::StartCancelReqs(CancelReqs *req)
{
    if (req == nullptr || req->reqs == nullptr) {
        HILOGE("the request is empty");
        return -EINVAL;
    }
    int32_t ret = CheckParameter(req->reqNum);
    if (ret < 0) {
        return ret;
    }
    HyperaioTrace trace("StartCancelReqs" + std::to_string(req->reqNum));
    uint32_t totalReqs = req->reqNum;
    uint32_t count = 0;
    std::vector<uint64_t> errorVec;
    std::vector<uint64_t> cancelInfoVec;
    for (uint32_t i = 0; i < totalReqs; i++) {
        struct io_uring_sqe *sqe = GetSqeWithRetry(&pImpl_->uring_);
        if (sqe == nullptr) {
            HILOGE("[HyperAio] get sqe failed");
            for (; i < totalReqs; ++i) {
                errorVec.push_back(req->reqs[i].userData);
            }
            HandleSqeError(count, cancelInfoVec);
            HandleRequestError(errorVec, -EBUSY);
            break;
        }
        struct CancelInfo *cancelInfo = &req->reqs[i];
        io_uring_sqe_set_data(sqe, reinterpret_cast<void *>(cancelInfo->userData));
        io_uring_prep_cancel(sqe, reinterpret_cast<void *>(cancelInfo->targetUserData), 0);
        HILOGD("[HyperAio] cancel userData = %{private}lu,  targetUserData = %{private}lu",
            cancelInfo->userData, cancelInfo->targetUserData);
        HyperaioTrace trace("cancel userData:" + std::to_string(cancelInfo->userData)
            + "targetUserData:" + std::to_string(cancelInfo->targetUserData));
        count++;
        cancelInfoVec.push_back(cancelInfo->userData);
        if (count >= BATCH_SIZE || i == totalReqs - 1) {
            int32_t ret = io_uring_submit(&pImpl_->uring_);
            if (ret < 0) {
                HILOGE("[HyperAio] submit cancel reqs failed, ret = %{public}d", ret);
                HandleRequestError(cancelInfoVec, -EBUSY);
            }
            HILOGI("[HyperAio] submit cancel reqs success, num = %{public}d", count);
            cancelReqCount_ += count;
            std::unique_lock<std::mutex> lock(cqeMutex_);
            pendingCqeCount_ += count;
            cqeCond_.notify_one();
            count = 0;
        }
    }
    return EOK;
}

void HyperAio::GetIoResult()
{
    struct io_uring_cqe *cqe;
    int32_t ret = io_uring_wait_cqe(&pImpl_->uring_, &cqe);
    pendingCqeCount_--;
    if (ret < 0 || cqe == nullptr) {
        HILOGE("[HyperAio] wait cqe failed, ret = %{public}d", ret);
        return;
    }
    cqeCount_++;
    if (cqe->res < 0) {
        HILOGE("[HyperAio] cqe failed, cqe->res = %{public}d", cqe->res);
    }
    auto response = std::make_unique<IoResponse>(cqe->user_data, cqe->res, cqe->flags);
    HyperaioTrace trace("harvest: userdata " + std::to_string(cqe->user_data)
        + " res " + std::to_string(cqe->res) + " flags " + std::to_string(cqe->flags));
    io_uring_cqe_seen(&pImpl_->uring_, cqe);
    if (ioResultCallBack_) {
        ioResultCallBack_(std::move(response));
    }
}

void HyperAio::HarvestRes()
{
    if (pImpl_ == nullptr) {
        HILOGE("[HyperAio] pImpl is null");
        return;
    }
    HILOGI("[HyperAio] harvest thread started");

    while (true) {
        std::unique_lock<std::mutex> lock(cqeMutex_);
        cqeCond_.wait(lock, [this] { return pendingCqeCount_.load() > 0 || stopThread_.load(); });
        while (pendingCqeCount_.load() > 0) {
            GetIoResult();
        }
        if (stopThread_.load()) {
            break;
        }
    }
    HILOGI("[HyperAio] exit harvest thread");
}

int32_t HyperAio::DestroyCtx()
{
    HILOGI("[HyperAio] openReqCount = %{public}u, readReqCount = %{public}u, "
        "cancelReqCount = %{public}u, cqeCount = %{public}u",
        openReqCount_.load(), readReqCount_.load(), cancelReqCount_.load(), cqeCount_.load());
    if (!initialized_.load()) {
        HILOGE("[HyperAio] not initialized");
        return EOK;
    }
    destroyed_.store(true);
    if (harvestThread_.joinable()) {
        HILOGI("[HyperAio] start harvest thread join");
        {
            std::unique_lock<std::mutex> lock(cqeMutex_);
            stopThread_.store(true);
            cqeCond_.notify_all();
        }
        harvestThread_.join();
        // This log is only printed after join() completes successfully
        HILOGI("[HyperAio] join success");
    }

    if (pImpl_ != nullptr) {
        io_uring_queue_exit(&pImpl_->uring_);
    }

    initialized_.store(false);
    HILOGI("[HyperAio] destroy hyperaio success");
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