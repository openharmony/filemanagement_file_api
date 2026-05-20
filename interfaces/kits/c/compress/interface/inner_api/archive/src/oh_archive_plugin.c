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

#include "oh_archive_plugin.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <securec.h>

static const char *HISPEED_ARCHIVE_SO_PATH = "libhispeed_compress.so";

static void *g_HSDCompressHandle = NULL;
static HispeedArchivePlugin g_hispeedArchivePlugin = {0};
static pthread_once_t g_onceControl = PTHREAD_ONCE_INIT;

static pthread_mutex_t g_pluginMutex = PTHREAD_MUTEX_INITIALIZER;

// archive reader
FmtReaderOps g_HSDZipReaderFmtOps = {0};

typedef void (*GenericFuncPtr)(void);

static void CloseHispeedArchivePluginIfSymNotFound(GenericFuncPtr func)
{
    if (func == NULL && g_HSDCompressHandle != NULL) {
        dlclose(g_HSDCompressHandle);
        g_HSDCompressHandle = NULL;
    }
}

static void ArchiveReaderLoadHispeedPlugin()
{
    g_HSDZipReaderFmtOps.open = (FmtReaderOpenFunc)dlsym(g_HSDCompressHandle,
        "HSD_Archive_ZipReaderOpenFile");
    g_HSDZipReaderFmtOps.extract = (FmtReaderExtractFunc)dlsym(g_HSDCompressHandle,
        "HSD_Archive_ZipReaderExtractAllFiles");
    g_HSDZipReaderFmtOps.close = (FmtReaderCloseFunc)dlsym(g_HSDCompressHandle,
        "HSD_Archive_ZipReaderClose");

    if (g_HSDZipReaderFmtOps.open == NULL || g_HSDZipReaderFmtOps.extract == NULL ||
        g_HSDZipReaderFmtOps.close == NULL) {
        if (g_HSDCompressHandle != NULL) {
            dlclose(g_HSDCompressHandle);
            g_HSDCompressHandle = NULL;
        }
        g_HSDZipReaderFmtOps.open = NULL;
        g_HSDZipReaderFmtOps.extract = NULL;
        g_HSDZipReaderFmtOps.close = NULL;
    }
    g_hispeedArchivePlugin.HSDZipReaderFmtOps = &g_HSDZipReaderFmtOps;
}

__attribute__((destructor)) static void ArchiveUnloadHispeedPlugin()
{
    pthread_mutex_lock(&g_pluginMutex);
    if (g_HSDCompressHandle != NULL) {
        dlclose(g_HSDCompressHandle);
        g_HSDCompressHandle = NULL;
    }
    pthread_mutex_unlock(&g_pluginMutex);
}

static void ArchiveLoadHispeedPlugin()
{
    g_HSDCompressHandle = dlopen(HISPEED_ARCHIVE_SO_PATH, RTLD_LAZY);
    if (g_HSDCompressHandle == NULL) {
        return;
    }

    g_hispeedArchivePlugin.streamWriteCreate = (HSDPlugin_StreamWrite_Create)dlsym(g_HSDCompressHandle,
        "HSD_Archive_StreamWrite_Create");
    g_hispeedArchivePlugin.streamWriteStart = (HSDPlugin_StreamWrite_Start)dlsym(g_HSDCompressHandle,
        "HSD_Archive_StreamWrite_Start");
    g_hispeedArchivePlugin.streamWriteSetCompressLevel = (HSDPlugin_StreamWrite_SetCompressLevel)dlsym(
        g_HSDCompressHandle, "HSD_Archive_StreamWrite_SetCompressLevel");
    g_hispeedArchivePlugin.streamWriteAbort = (HSDPlugin_StreamWrite_Abort)dlsym(g_HSDCompressHandle,
        "HSD_Archive_StreamWrite_Abort");
    g_hispeedArchivePlugin.streamWriteUpdate = (HSDPlugin_StreamWrite_Update)dlsym(g_HSDCompressHandle,
        "HSD_Archive_StreamWrite_Update");
    g_hispeedArchivePlugin.streamWriteEnd = (HSDPlugin_StreamWrite_End)dlsym(g_HSDCompressHandle,
                                                                             "HSD_Archive_StreamWrite_End");
    g_hispeedArchivePlugin.streamWriteDestroy = (HSDPlugin_StreamWrite_Destroy)dlsym(g_HSDCompressHandle,
        "HSD_Archive_StreamWrite_Destroy");
    g_hispeedArchivePlugin.bufferWrite = (HSDPlugin_BufferWrite)dlsym(g_HSDCompressHandle,
                                                                      "HSD_Archive_BufferWrite");
    g_hispeedArchivePlugin.bufferWriteCompressBound = (HSDPlugin_BufferWriteCompressBound)dlsym(
        g_HSDCompressHandle,
        "HSD_Archive_BufferWriteCompressBound");
    g_hispeedArchivePlugin.zipWriterOps.open = dlsym(g_HSDCompressHandle, "HSD_Archive_ZipWriterOpenFile");
    g_hispeedArchivePlugin.zipWriterOps.add = dlsym(g_HSDCompressHandle, "HSD_Archive_ZipWriterAddFunc");
    g_hispeedArchivePlugin.zipWriterOps.close = dlsym(g_HSDCompressHandle, "HSD_Archive_ZipWriterClose");

    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteCreate);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteStart);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteSetCompressLevel);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteAbort);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteUpdate);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteEnd);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.streamWriteDestroy);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.bufferWrite);
    CloseHispeedArchivePluginIfSymNotFound((GenericFuncPtr)g_hispeedArchivePlugin.bufferWriteCompressBound);

    ArchiveReaderLoadHispeedPlugin();
}

ARCHIVE_API const HispeedArchivePlugin *GetHispeedArchivePluginHandle()
{
    pthread_once(&g_onceControl, ArchiveLoadHispeedPlugin);
    if (g_HSDCompressHandle == NULL) {
        return NULL;
    }
    return &g_hispeedArchivePlugin;
}

ARCHIVE_API void ReleaseHispeedArchivePluginHandle()
{
    pthread_mutex_lock(&g_pluginMutex);
    if (g_HSDCompressHandle != NULL) {
        dlclose(g_HSDCompressHandle);
        g_HSDCompressHandle = NULL;
        memset_s(&g_hispeedArchivePlugin, sizeof(HispeedArchivePlugin), 0, sizeof(HispeedArchivePlugin));
        memset_s(&g_HSDZipReaderFmtOps, sizeof(FmtReaderOps), 0, sizeof(FmtReaderOps));
    }
    pthread_mutex_unlock(&g_pluginMutex);
}
