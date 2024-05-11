/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "readerIterator_impl.h"

using namespace OHOS::FileManagement;
using namespace OHOS::CJSystemapi::FileFs;

namespace OHOS::CJSystemapi {
ReadIteratorImpl::ReadIteratorImpl(std::shared_ptr<OHOS::FileManagement::ModuleFileIO::ReaderIteratorEntity> entity)
{
    entity_ = entity;
}
std::tuple<int32_t, bool, char*> ReadIteratorImpl::Next()
{
    Str *str = NextLine(entity_->iterator);
    if (str == nullptr) {
        if (entity_->offset != 0) {
            HILOGE("Failed to get next line, error:%{public}d", errno);
            return {GetErrorCode(errno), entity_->offset == 0, nullptr};
        }
        entity_ = nullptr;
        return {SUCCESS_CODE, true, nullptr};
    }
    bool done = entity_->offset == 0;
    char* value = static_cast<char*>(malloc((str->len + 1) * sizeof(char)));
    if (value == nullptr) {
        return {GetErrorCode(ENOMEM), done, nullptr};
    }
    memcpy_s(value, str->len + 1, str->str, str->len + 1);
    entity_->offset -= static_cast<int64_t>(str->len);
    StrFree(str);
    return { SUCCESS_CODE, done, value};
}
}