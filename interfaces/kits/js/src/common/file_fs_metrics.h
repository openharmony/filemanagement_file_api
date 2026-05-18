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

#ifndef FILE_FS_METRICS_H
#define FILE_FS_METRICS_H

#ifdef FILE_API_METRICS
#include "histogram_plugin_macros.h"
#define METRICS_COUNT(name) HISTOGRAM_BOOLEAN(name, 1)
#define METRICS_ERROR(name, errCode) \
    HISTOGRAM_ENUMERATION(name, ToErrCodeIndex(errCode), FILEIO_ERR_INDEX_MAX)
#else
#define METRICS_COUNT(name)
#define METRICS_ERROR(name, errCode)
#endif

#endif
