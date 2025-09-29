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
#include "hyperaio_fuzzer.h"

#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

#include "fileapi_fuzzer_helper.h"
#include "filemgmt_libhilog.h"
#include "hyperaio.h"

using namespace OHOS::HyperAio;

namespace OHOS {
using namespace std;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;
constexpr uint32_t URING_QUEUE_SIZE = 512;
uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();
uint64_t HALF_MAX_UINT64 = MAX_UINT64 / 2;
std::function<void(unique_ptr<IoResponse>)> callBack = [](unique_ptr<IoResponse> response) {
    if (response == nullptr) {
        return;
    }
    if (response->userData < HALF_MAX_UINT64) {
        close(response->res);
    }
    return;
};

bool HyperaioStartOpenReqsFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<HyperAio::HyperAio> hyperAio_ = std::make_unique<HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);
    std::filesystem::path directory = "/data/local/tmp";
    if (!std::filesystem::exists(directory)) {
        HILOGE("hyperaio testsync dir not exists");
        return false;
    }
    uint64_t userData = fuzzData.GetData<uint64_t>();
    userData = userData % HALF_MAX_UINT64;
    OpenReqs reqs;
    std::vector<OpenInfo> openInfoArray;
    OpenInfo openInfo;
    openInfo.dfd = -1;
    openInfo.flags = O_RDONLY;
    openInfo.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    std::string pathStr = "/data/local/tmp/1.txt";
    openInfo.path = strdup(pathStr.c_str());
    openInfo.userData = userData;
    openInfoArray.push_back(openInfo);
    reqs.reqNum = openInfoArray.size();
    reqs.reqs = openInfoArray.data();
    hyperAio_->StartOpenReqs(&reqs);
    if (hyperAio_ != nullptr) {
        hyperAio_->DestroyCtx();
    }
    return true;
}

bool HyperaioStartOpenReqsBatchFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<HyperAio::HyperAio> hyperAio_ = std::make_unique<HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);
    std::filesystem::path directory = "/data/local/tmp";
    if (!std::filesystem::exists(directory)) {
        HILOGE("hyperaio testsync dir not exists");
        return false;
    }
    uint64_t userData = fuzzData.GetData<uint64_t>();
    userData = userData % HALF_MAX_UINT64;
    uint32_t batchSize = fuzzData.GetData<uint32_t>();
    batchSize = batchSize % URING_QUEUE_SIZE;
    if (batchSize == 0) {
        batchSize++;
    }
    OpenReqs reqs;
    std::vector<OpenInfo> openInfoArray;
    for (uint32_t i = 0; i < batchSize; ++i) {
        OpenInfo openInfo;
        openInfo.dfd = -1;
        openInfo.flags = O_RDONLY;
        openInfo.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
        std::string pathStr = "/data/local/tmp/" + std::to_string(i) + ".txt";
        openInfo.path = strdup(pathStr.c_str());
        openInfo.userData = userData;
        openInfoArray.push_back(openInfo);
    }
    reqs.reqNum = openInfoArray.size();
    reqs.reqs = openInfoArray.data();
    hyperAio_->StartOpenReqs(&reqs);
    if (hyperAio_ != nullptr) {
        hyperAio_->DestroyCtx();
    }
    return true;
}

bool HyperaioStartReadReqsFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<HyperAio::HyperAio> hyperAio_ = std::make_unique<HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);
    uint64_t userData = fuzzData.GetData<uint64_t>();
    userData = userData % HALF_MAX_UINT64 + HALF_MAX_UINT64 + 1;
    int32_t fd = fuzzData.GetData<int32_t>();
    uint32_t batchSize = fuzzData.GetData<uint32_t>();
    batchSize = batchSize % URING_QUEUE_SIZE;
    if (batchSize == 0) {
        batchSize++;
    }
    char *buf;
    size_t buffSize = 1024;
    buf = static_cast<char*>(malloc(buffSize));
    if (!buf) {
        return false;
    }
    std::vector<ReadInfo> infos;
    ReadInfo info;
    info.fd = fd;
    info.len = buffSize;
    info.offset = 0;
    info.buf = buf;
    info.userData = userData;
    infos.emplace_back(info);
    ReadReqs reqs;
    reqs.reqNum = infos.size();
    reqs.reqs = infos.data();
    hyperAio_->StartReadReqs(&reqs);
    if (hyperAio_ != nullptr) {
        hyperAio_->DestroyCtx();
    }
    free(buf);
    close(fd);
    return true;
}

bool HyperaioStartCancelReqsFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<HyperAio::HyperAio> hyperAio_ = std::make_unique<HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);
    uint64_t userData = fuzzData.GetData<uint64_t>();
    userData = userData % HALF_MAX_UINT64 + HALF_MAX_UINT64 + 1;
    CancelReqs reqs;
    std::vector<CancelInfo> cancelInfoArray;
    CancelInfo cancelInfo;
    cancelInfo.userData = userData;
    cancelInfo.targetUserData = userData + U32_AT_SIZE;
    cancelInfoArray.push_back(cancelInfo);
    reqs.reqNum = cancelInfoArray.size();
    reqs.reqs = cancelInfoArray.data();
    hyperAio_->StartCancelReqs(&reqs);
    if (hyperAio_ != nullptr) {
        hyperAio_->DestroyCtx();
    }
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
#ifdef HYPERAIO_USE_LIBURING
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U32_AT_SIZE + OHOS::U64_AT_SIZE)) {
        return 0;
    }

    OHOS::FuzzData fuzzData(data, size);
    OHOS::HyperaioStartOpenReqsFuzzTest(fuzzData, size);
    OHOS::HyperaioStartOpenReqsBatchFuzzTest(fuzzData, size);
    OHOS::HyperaioStartReadReqsFuzzTest(fuzzData, size);
    OHOS::HyperaioStartCancelReqsFuzzTest(fuzzData, size);
#endif
    return 0;
}