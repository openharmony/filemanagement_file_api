/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "randomAccessFile_impl.h"

using namespace OHOS::CJSystemapi::FileFs;

namespace OHOS::CJSystemapi {
static int64_t CalculateOffset(int64_t offset, int64_t fPointer)
{
    if (offset < 0) {
        LOGE("No specified offset provided");
        offset = fPointer;
    } else {
        offset += fPointer;
    }
    return offset;
}
static int DoWriteRAF(char* buf, size_t len, int fd, int64_t offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> write_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (write_req == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    LOGI("write buffer is %{public}s", buf);
    uv_buf_t iov = uv_buf_init(buf, len);
    int ret = uv_fs_write(nullptr, write_req.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}
static int DoReadRAF(char* buf, size_t len, int fd, int64_t offset)
{
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> read_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (read_req == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    uv_buf_t iov = uv_buf_init(buf, len);
    int ret = uv_fs_read(nullptr, read_req.get(), fd, &iov, 1, offset, nullptr);
    return ret;
}
RandomAccessFileImpl::RandomAccessFileImpl(std::shared_ptr<
        OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity> entity)
{
    entity_ = entity;
}
int32_t RandomAccessFileImpl::GetFd()
{
    LOGI("start get fs in cpp");
    if (entity_ == nullptr) {
        LOGE("Failed to creat entity.");
        return -1;
    }
    if (entity_->fd.get() == nullptr) {
        LOGE("Failed to get fd.");
        return -1;
    }
    return entity_->fd.get()->GetFD();
}
int64_t RandomAccessFileImpl::GetFPointer()
{
    if (entity_ == nullptr) {
        LOGE("Failed to get entity.");
        return 0;
    }
    return entity_->filePointer;
}
void RandomAccessFileImpl::SetFilePointerSync(int64_t fp)
{
    if (entity_ == nullptr) {
        LOGE("Failed to get entity.");
        return;
    }
    entity_->filePointer = fp;
    return;
}
void RandomAccessFileImpl::CloseSync()
{
    if (entity_ == nullptr || entity_->fd.get() == nullptr) {
        LOGE("Failed to get entity.");
        entity_ = nullptr;
        return;
    }
    std::unique_ptr<uv_fs_t, decltype(CommonFunc::FsReqCleanup)*> close_req = {
        new (std::nothrow) uv_fs_t, CommonFunc::FsReqCleanup };
    if (!close_req) {
        LOGE("Failed to close file with ret: %{public}d", ENOMEM);
        return;
    }
    int ret = uv_fs_close(nullptr, close_req.get(), entity_->fd.get()->GetFD(), nullptr);
    if (ret < 0) {
        LOGE("Failed to close file with ret: %{public}d", ret);
    }
    entity_ = nullptr;
    return;
}
std::tuple<int32_t, int64_t> RandomAccessFileImpl::WriteSync(char* buf, size_t len, int64_t offset)
{
    if (entity_ == nullptr || entity_->fd.get() == nullptr) {
        LOGE("Failed to get entity.");
        return {EIO, 0};
    }
    int64_t newOffset = CalculateOffset(offset, entity_->filePointer);
    int writeCode = DoWriteRAF(buf, len, entity_->fd.get()->GetFD(), newOffset);
    if (writeCode < 0) {
        LOGE("Failed to read file for %{public}d", writeCode);
        return {GetErrorCode(-writeCode), 0};
    }
    entity_->filePointer = newOffset + writeCode;
    return {SUCCESS_CODE, writeCode};
}
std::tuple<int32_t, int64_t> RandomAccessFileImpl::ReadSync(char* buf, size_t len, int64_t offset)
{
    if (entity_ == nullptr || entity_->fd.get() == nullptr) {
        LOGE("Failed to get entity.");
        return {EIO, 0};
    }
    int64_t newOffset = CalculateOffset(offset, entity_->filePointer);
    int readCode = DoReadRAF(buf, len, entity_->fd.get()->GetFD(), newOffset);
    if (readCode < 0) {
        LOGE("Failed to read file for %{public}d", readCode);
        return {GetErrorCode(-readCode), 0};
    }
    entity_->filePointer = newOffset + readCode;
    return {SUCCESS_CODE, readCode};
}
std::shared_ptr<OHOS::FileManagement::ModuleFileIO::RandomAccessFileEntity> RandomAccessFileImpl::GetEntity()
{
    return entity_;
}
}