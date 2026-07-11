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

#ifndef OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_IO_ENGINE_H
#define OHOS_FILEMANAGEMENT_FILE_API_NATIVE_SWAPFS_IO_ENGINE_H

#include <cstddef>
#include <string>

namespace OHOS::FileManagement::Swapfs {
class SyncReadEngine {
public:
    int Read(const std::string &path, void *buffer, size_t size, size_t offset, bool useDirectIo);
};

class SyncWriteEngine {
public:
    int Write(const std::string &path, const void *buffer, size_t size, bool useDirectIo);
};

bool IsDioAligned(const void *buffer, size_t size);
} // namespace OHOS::FileManagement::Swapfs

#endif
