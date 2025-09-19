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

#include "prop_n_exporter_mock.h"
#include "n_class.h"

#include <dlfcn.h>

namespace OHOS::FileManagement::ModuleFileIO::Test {
using namespace OHOS::FileManagement::ModuleFileIO;

thread_local std::shared_ptr<PropNExporterMock>PropNExporterMock::pPropNExporterMock = nullptr;
thread_local bool PropNExporterMock::mockable = false;

std::shared_ptr<PropNExporterMock> PropNExporterMock::GetMock()
{
    if (pPropNExporterMock == nullptr) {
        pPropNExporterMock = std::make_shared<PropNExporterMock>();
    }
    return pPropNExporterMock;
}

void PropNExporterMock::EnableMock()
{
    mockable = true;
}

void PropNExporterMock::DisableMock()
{
    pPropNExporterMock = nullptr;
    mockable = false;
}

bool PropNExporterMock::IsMockable()
{
    return mockable;
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test

using namespace OHOS::FileManagement::ModuleFileIO::Test;

