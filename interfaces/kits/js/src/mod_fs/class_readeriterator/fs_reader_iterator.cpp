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

#include "fs_reader_iterator.h"

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "rust_file.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

FsResult<FsReaderIterator *> FsReaderIterator::Constructor()
{
    auto entity = CreateUniquePtr<ReaderIteratorEntity>();
    if (entity == nullptr) {
        HILOGE("Failed to request heap memory.");
        return FsResult<FsReaderIterator *>::Error(ENOMEM);
    }

    FsReaderIterator *it = new (nothrow) FsReaderIterator(move(entity));

    if (it == nullptr) {
        HILOGE("Failed to create FsReaderIterator object on heap.");
        return FsResult<FsReaderIterator *>::Error(ENOMEM);
    }
    return FsResult<FsReaderIterator *>::Success(move(it));
}

FsResult<ReaderIteratorResult> FsReaderIterator::Next()
{
    if (entity == nullptr) {
        HILOGE("Failed to get reader iterator entity");
        return FsResult<ReaderIteratorResult>::Error(UNKNOWN_ERR);
    }

    Str *str = NextLine(entity->iterator);
    if (str == nullptr && entity->offset != 0) {
        HILOGE("Failed to get next line, error:%{public}d", errno);
        return FsResult<ReaderIteratorResult>::Error(errno);
    }

    ReaderIteratorResult result;
    bool done = entity->offset == 0;
    result.done = done;
    if (str != nullptr) {
        std::string value(str->str, str->len);
        result.value = value;
        entity->offset -= static_cast<int64_t>(str->len);
    } else {
        result.value = "";
    }
    StrFree(str);

    return FsResult<ReaderIteratorResult>::Success(move(result));
}

} // namespace OHOS::FileManagement::ModuleFileIO