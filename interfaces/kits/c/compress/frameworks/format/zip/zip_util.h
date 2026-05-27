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

#ifndef ZIP_UTIL_H
#define ZIP_UTIL_H

#include "oh_archive_reader.h"
#include "oh_archive_writer.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const FmtReaderOps g_ZipReaderFmtOps;
extern const FmtWriterOps g_ZipWriterFmtOps;

#ifdef __cplusplus
}
#endif

#endif