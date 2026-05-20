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
#ifndef OH_ARCHIVE_WRITER_H
#define OH_ARCHIVE_WRITER_H
#include "oh_archive.h"


struct ArchiveWriteCtx;
typedef struct ArchiveWriteCtx HmArchiveWriteInfo;

typedef int (*FmtWriterOpenFunc)(HmArchiveWriteInfo *archive, const char *outfile);
typedef int (*FmtWriterAddFunc)(HmArchiveWriteInfo *archive, const char **infiles, uint64_t fileNum);
typedef int (*FmtWriterCloseFunc)(HmArchiveWriteInfo *archive);

typedef struct {
    FmtWriterOpenFunc open;
    FmtWriterAddFunc add;
    FmtWriterCloseFunc close;
} FmtWriterOps;


struct ArchiveWriteCtx {
    OH_Archive_ProgressHandlerWithData progressHandler;
    void *progressHandlerUserData;
    OH_Archive_CompressMethod method;
    int32_t level;
    const FmtWriterOps *fmtOps;
    void *fmtInfo;
    int append;
};

#endif
