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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_FS_STREAM_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_FS_STREAM_H

#include "stream_entity.h"

#include <memory>
#include <mutex>
#include <optional>

#include "filemgmt_libfs.h"
#include "fs_utils.h"
#include "write_options.h"
#include "read_options.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;

class FsStream final {
public:
    StreamEntity *GetStreamEntity() const
    {
        return streamEntity.get();
    }

    FsStream(const FsStream &) = delete;
    FsStream &operator=(const FsStream &) = delete;

    std::shared_ptr<FILE> GetFilePtr();
    FsResult<size_t> Write(const ArrayBuffer &buf, const optional<WriteOptions> &options = nullopt);
    FsResult<size_t> Write(const string &buf, const optional<WriteOptions> &options = nullopt);
    FsResult<size_t> Read(ArrayBuffer &buf, const optional<ReadOptions> &options = nullopt);
    FsResult<void> Close();
    FsResult<void> Flush();
    FsResult<int64_t> Seek(const int64_t &offset, const optional<int32_t> &typeOpt = nullopt);

    ~FsStream() {
        HILOGE("~FsStream success.");
    }
    static FsResult<FsStream *> Constructor();

private:
    std::mutex mtx;
    unique_ptr<StreamEntity> streamEntity;
    explicit FsStream(unique_ptr<StreamEntity> entity) : streamEntity(move(entity)) {}
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_FS_STREAM_H