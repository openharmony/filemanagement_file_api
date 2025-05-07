/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef UV_FS_MOCK_H
#define UV_FS_MOCK_H

#include "read_core.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::ModuleFileIO {

class Uvfs {
public:
    static inline std::shared_ptr<Uvfs> ins = nullptr;
public:
    virtual ~Uvfs() = default;
    virtual int uv_fs_read(uv_loop_t* loop, uv_fs_t* req,
               uv_file file,
               const uv_buf_t bufs[],
               unsigned int nbufs,
               int64_t off,
               uv_fs_cb cb) = 0;
};

class UvfsMock : public Uvfs {
public:
    MOCK_METHOD7(uv_fs_read, int(uv_loop_t* loop, uv_fs_t* req,
               uv_file file,
               const uv_buf_t bufs[],
               unsigned int nbufs,
               int64_t off,
               uv_fs_cb cb));
};


} // OHOS::FileManagement::ModuleFileIO
#endif