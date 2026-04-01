/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_FDTAG_FUNC_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_FDTAG_FUNC_H
 
#include "fd_guard.h"
 
#include <cstdint>
#include <cstddef>
 
namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
 
constexpr uint64_t PREFIX_ADDR = 0xabc0000000000000;
 
#define FD_SAN_TABLE_SIZE 128
#define FD_SAN_OVERFLOW_END 2048
 
struct FdSanEntry {
    _Atomic(uint64_t) close_tag;
};
 
struct FdSanTableOverflow {
    size_t len;
    struct FdSanEntry entries[];
};
 
struct FdSanTable {
    struct FdSanEntry entries[FD_SAN_TABLE_SIZE];
    _Atomic(struct FdSanTableOverflow*) overflow;
};
 
struct FdTagFunc {
#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM) && !defined(CROSS_PLATFORM)
    static uint64_t GetFdTag(int fd);
    static void SetFdTag(int fd, uint64_t tag);
#endif
};
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_FDTAG_FUNC_H