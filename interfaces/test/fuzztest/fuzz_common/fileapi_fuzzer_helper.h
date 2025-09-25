/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FILEAPI_FUZZER_HELPER_H
#define FILEAPI_FUZZER_HELPER_H

#include "securec.h"
#include <string>

namespace OHOS {
class FuzzData {
public:
    FuzzData(const uint8_t *data, size_t size)
    {
        baseData = data;
        baseSize = size;
    }

    void ResetData(size_t newSize)
    {
        if (newSize <= baseSize) {
            baseSize = newSize;
        }
        basePos = 0;
    }

    template<class T>
    T GetData()
    {
        T object{};
        size_t objectSize = sizeof(object);
        if (baseData == nullptr || objectSize > baseSize - basePos) {
            return object;
        }
        if (memcpy_s(&object, objectSize, baseData + basePos, objectSize) != EOK) {
            return {};
        }
        basePos = basePos + objectSize;
        return object;
    }

    size_t GetRemainSize() const
    {
        return baseSize - basePos;
    }

    std::string GetStringFromData(int len)
    {
        if (len <= 0 || baseData == nullptr || basePos >= baseSize) {
            return "";
        }

        std::string cstr;
        for (int i = 0; i < len; ++i) {
            if (basePos >= baseSize) {
                basePos = 0;
            }
            char tmp = GetData<char>();
            if (tmp == '\0') {
                cstr.push_back('1');
            } else {
                cstr.push_back(tmp);
            }
        }
        return cstr;
    }

private:
    const uint8_t *baseData{nullptr};
    size_t baseSize{0};
    size_t basePos{0};
};
} // namespace OHOS
#endif // FILEAPI_FUZZER_HELPER_H