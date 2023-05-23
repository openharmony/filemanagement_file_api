/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FILEMGMT_COMMON_FILE_UTILS_H
#define FILEMGMT_COMMON_FILE_UTILS_H

#include <memory>

#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement {
template<typename T, typename... Args>
std::shared_ptr<T> CreateSharedPtr(Args&&... args)
{
    std::shared_ptr<T> sPtr = nullptr;
    try {
        sPtr = std::make_shared<T>(std::forward<Args>(args)...);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
    return sPtr;
};

template<typename T, typename... Args>
std::unique_ptr<T> CreateUniquePtr(Args&&... args)
{
    std::unique_ptr<T> uPtr = nullptr;
    try {
        uPtr = std::make_unique<T>(std::forward<Args>(args)...);
    } catch (const std::bad_alloc&) {
        return nullptr;
    }
    return uPtr;
}

} // namespace OHOS::FileManagement
#endif // FILEMGMT_COMMON_FILE_UTILS_H
