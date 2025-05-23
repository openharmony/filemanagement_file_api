/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FILEMGMT_LIBN_N_EXPORTER_H
#define FILEMGMT_LIBN_N_EXPORTER_H

#include "n_napi.h"

#include <functional>
#include <string>

#include "n_val.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
class NExporter {
public:
    NExporter(napi_env env, napi_value exports) : exports_(env, exports) {};
    virtual ~NExporter() = default;
    virtual bool Export() = 0;
#ifdef WIN_PLATFORM
    virtual std::string GetNExporterName() = 0;
#else
    virtual std::string GetClassName() = 0;
#endif

protected:
    NVal exports_;
};
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBN_N_EXPORTER_H