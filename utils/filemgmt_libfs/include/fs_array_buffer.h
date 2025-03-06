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

#ifndef FILEMANAGEMENT_FS_ARRAY_BUFFER_H
#define FILEMANAGEMENT_FS_ARRAY_BUFFER_H

#include <cstddef>

namespace OHOS::FileManagement::ModuleFileIO {

struct ArrayBuffer {
    void *buf;
    size_t length;

    ArrayBuffer(void *buffer, size_t len) : buf(buffer), length(len) {}
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // FILEMANAGEMENT_FS_ARRAY_BUFFER_H