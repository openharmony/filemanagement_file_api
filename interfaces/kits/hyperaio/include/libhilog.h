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

#ifndef HYPERAIO_LIBHILOG_H
#define HYPERAIO_LIBHILOG_H

#include "hilog/log.h"

#include <string>

namespace OHOS {
namespace HyperAio {

#if defined __FILE_NAME__
#define FILEMGMT_FILE_NAME __FILE_NAME__
#else
#define FILEMGMT_FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef HYPERAIO_LOG_DOMAIN
#define HYPERAIO_LOG_DOMAIN 0xD004388
#endif

#undef HYPERAIO_LOG_TAG
#define HYPERAIO_LOG_TAG "hyperaio"

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
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, HYPERAIO_LOG_DOMAIN, HYPERAIO_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGE(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, HYPERAIO_LOG_DOMAIN, HYPERAIO_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGW(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, HYPERAIO_LOG_DOMAIN, HYPERAIO_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGI(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, HYPERAIO_LOG_DOMAIN, HYPERAIO_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#define HILOGD(fmt, ...)            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, HYPERAIO_LOG_DOMAIN, HYPERAIO_LOG_TAG, \
    "[%{public}s:%{public}d->%{public}s] " fmt, FILEMGMT_FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__))
} // namespace HyperAio
} // namespace OHOS

#endif // HYPERAIO_LIBHILOG_H