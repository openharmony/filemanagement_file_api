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

#ifndef FILEMGMT_LIBHILOG_H
#define FILEMGMT_LIBHILOG_H

#include "hilog/log.h"

#include <string>

namespace OHOS {
namespace FileManagement {

#if defined __FILE_NAME__
#define FILEMGMT_FILE_NAME __FILE_NAME__
#else
#define FILEMGMT_FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef FILEMGMT_LOG_DOMAIN
#define FILEMGMT_LOG_DOMAIN 0xD004388
#endif

#undef FILEMGMT_LOG_TAG
#define FILEMGMT_LOG_TAG "file_api"


#ifdef HILOGD
#undef HILOGD
#endif

#ifdef HILOGF
#undef HILOGF
#endif

#ifdef HILOGE
#undef HILOGE
#endif

#ifdef HILOGW
#undef HILOGW
#endif

#ifdef HILOGI
#undef HILOGI
#endif

#define HILOGF(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, FILEMGMT_LOG_DOMAIN, FILEMGMT_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGE(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, FILEMGMT_LOG_DOMAIN, FILEMGMT_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGW(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, FILEMGMT_LOG_DOMAIN, FILEMGMT_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGI(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, FILEMGMT_LOG_DOMAIN, FILEMGMT_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGD(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, FILEMGMT_LOG_DOMAIN, FILEMGMT_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMGMT_LIBHILOG_H