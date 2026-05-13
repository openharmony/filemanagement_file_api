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

#ifndef FILEMGMT_CJ_FILE_UTILS_H
#define FILEMGMT_CJ_FILE_UTILS_H

#include <memory>

#include "filemgmt_libhilog.h"

namespace OHOS {
namespace CJSystemapi {
namespace FileFs {
template<typename T, typename... Args>
std::shared_ptr<T> CreateSharedPtr(Args&&... args)
{
    T* rawPtr = new (std::nothrow) T(std::forward<Args>(args)...);
    if (rawPtr == nullptr) {
        return nullptr;
    }
    return std::shared_ptr<T>(rawPtr);
}

template<typename T, typename... Args>
std::unique_ptr<T> CreateUniquePtr(Args&&... args)
{
    T* rawPtr = new (std::nothrow) T(std::forward<Args>(args)...);
    if (rawPtr == nullptr) {
        return nullptr;
    }
    return std::unique_ptr<T>(rawPtr);
}
} // namespace FileFs
} // namespace CJSystemapi
} // namespace OHOS
#endif // FILEMGMT_CJ_FILE_UTILS_H