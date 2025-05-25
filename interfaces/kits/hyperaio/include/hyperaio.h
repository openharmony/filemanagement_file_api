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

#ifndef FILEMANAGEMENT_FILE_API_INTERFACES_KITS_IOURING_HYPER_AIO_H
#define FILEMANAGEMENT_FILE_API_INTERFACES_KITS_IOURING_HYPER_AIO_H

#include <functional>
#include <thread>
#ifdef HYPERAIO_USE_LIBURING
#include "liburing.h"
#endif
namespace OHOS {
namespace HyperAio {
#define IOURING_APP_PERMISSION      (1U << 0)
#ifndef EOK
#define EOK (0)
#endif

struct ReadInfo {
    int32_t fd;
    uint32_t len;
    uint64_t offset;
    void *buf;
    uint64_t userData;
};

struct ReadReqs {
    uint32_t reqNum;
    struct ReadInfo *reqs;
};

struct OpenInfo {
    int32_t dfd;
    int32_t flags;
    uint32_t mode;
    void *path;
    uint64_t userData;
};

struct OpenReqs {
    uint32_t reqNum;
    struct OpenInfo *reqs;
};

struct CancelInfo {
    uint64_t userData;
    uint64_t targetUserData;
};

struct CancelReqs {
    uint32_t reqNum;
    struct CancelInfo *reqs;
};

struct IoResponse {
    uint64_t userData;
    int32_t res;
    uint32_t flags;
    IoResponse(uint64_t userData, int32_t res, uint32_t flags)
        : userData(userData),
          res(res),
          flags(flags) {
    }
};

class HyperAio {
public:
    using ProcessIoResultCallBack = std::function<void(std::unique_ptr<IoResponse>)>;
    uint32_t SupportIouring();
    int32_t CtxInit(ProcessIoResultCallBack *callBack);
    int32_t StartReadReqs(ReadReqs *req);
    int32_t StartOpenReqs(OpenReqs *req);
    int32_t StartCancelReqs(CancelReqs *req);
    int32_t DestroyCtx();
private:
    ProcessIoResultCallBack ioResultCallBack_ = nullptr;
#ifdef HYPERAIO_USE_LIBURING
    io_uring uring_;
    std::thread harvestThread_;
    std::atomic<bool> stopThread_;
    std::atomic<bool> initialized_;
    void HarvestRes();
    struct io_uring_sqe* GetSqeWithRetry(struct io_uring *ring);
#endif
};
}
}
#endif