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
#include "hyperaio.h"
#include "fileapi_fuzzer_helper.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;
constexpr uint32_t URING_QUEUE_SIZE = 512;
std::function<void(std::unique_ptr<OHOS::HyperAio::IoResponse>)> callBack = [](std::unique_ptr<OHOS::HyperAio::IoResponse> response) {};

bool HyperaioStartOpenReqsFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<OHOS::HyperAio::HyperAio> hyperAio_ = std::make_unique<OHOS::HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);

    // userData
    uint64_t userData = fuzzData.GetData<uint64_t>();

    OHOS::HyperAio::OpenInfo openInfo = {0, O_RDWR, 0, nullptr, userData};
    OHOS::HyperAio::OpenReqs openReqs = {1, &openInfo};

    hyperAio_->StartOpenReqs(&openReqs);
    hyperAio_->DestroyCtx();

    return true;
}

bool HyperaioStartOpenReqsBatchFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::unique_ptr<OHOS::HyperAio::HyperAio> hyperAio_ = std::make_unique<OHOS::HyperAio::HyperAio>();
    hyperAio_->CtxInit(&callBack);

    // userData
    uint64_t userData = fuzzData.GetData<uint64_t>();
    uint32_t batchSize = fuzzData.GetData<uint32_t>();
    batchSize = batchSize % URING_QUEUE_SIZE;

    auto openInfos = std::make_unique<OHOS::HyperAio::OpenInfo[]>(batchSize);
    for (uint32_t i = 0; i < batchSize; ++i) {
        openInfos[i].dfd = 0;
        openInfos[i].flags = O_RDWR;
        openInfos[i].mode = 0;
        openInfos[i].path = nullptr;
        openInfos[i].userData = userData + i;
    }
    OHOS::HyperAio::OpenReqs openReqs = {batchSize, openInfos.get()};

    hyperAio_->StartOpenReqs(&openReqs);
    hyperAio_->DestroyCtx();

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
#endif
    return 0;
}