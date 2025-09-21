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

#include "file_fs_trace.h"
#ifdef FILE_API_TRACE
#include "hitrace_meter.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

FileFsTrace::FileFsTrace(const std::string& value)
{
#ifdef FILE_API_TRACE
    if (FileApiDebug::isTraceEnhanced) {
        StartTrace(HITRACE_TAG_FILEMANAGEMENT, "[FileFs]" + value);
    }
#endif
}

void FileFsTrace::End()
{
#ifdef FILE_API_TRACE
    if (!isEnded && FileApiDebug::isTraceEnhanced) {
        FinishTrace(HITRACE_TAG_FILEMANAGEMENT);
        isEnded = true;
    }
#endif
}

FileFsTrace::~FileFsTrace()
{
    End();
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS