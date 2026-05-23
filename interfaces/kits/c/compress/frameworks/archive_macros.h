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
#ifndef ARCHIVE_MACROS_H
#define ARCHIVE_MACROS_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#define ZIP_FILE_NAME_LEN_MAX PATH_MAX
#define ZIP_BUF_INTERNAL 32768

#define RETURN_IF_MAGIC_ERR(obj, val) ((void)0)
#define ASSERT_IF_MAGIC_ERR(obj, val) ASSERT(((obj)->magic) == (val))
#define SET_OBJ_MAGIC(obj, val) (obj)->magic = (val)

#ifndef NDEBUG
#define ARCHIVE_ERR(format, arg...)
#define ARCHIVE_INFO(format, arg...)
#define ARCHIVE_DEBUG(format, arg...)
#else
#define ARCHIVE_ERR(format, arg...)
#define ARCHIVE_INFO(format, arg...)
#define ARCHIVE_DEBUG(format, arg...)
#endif

#define ARCHIVE_IMPL __attribute__((visibility("hidden")))
#define ARCHIVE_API __attribute__((visibility("default")))

#define LOCAL static
#define ARCHIVE_UNUSED(x) ((void)0)
#define RETURN_IF_FAIL(ret)                                     \
    do {                                                        \
        if ((ret) != 0) {                                       \
            ARCHIVE_ERR("function failed ret is %d !\n", ret);  \
            return (ret);                                       \
        }                                                       \
    } while (0)

#define ASSERT(f) ((void)0)

#endif