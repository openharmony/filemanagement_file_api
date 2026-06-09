/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
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

/**
 * @addtogroup Archive
 * @{
 *
 * @brief This module provides the archive operations.
 * @since 26.0.0
 */

/**
 * @file oh_archive.h
 *
 * @brief Provides archive APIs.
 * @kit CoreFileKit
 * @library liboharchive.so
 * @syscap SystemCapability.FileManagement.File.FileIO
 * @since 26.0.0
 */
#ifndef FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_H
#define FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_H

#include <stdint.h>
#include "oh_archive_errcode.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */
/**
 * @brief Archive Writer context structure.
 * @since 26.0.0
 */
typedef struct ArchiveWriteCtx *OH_Archive_Writer_Ctx;
/**
 * @brief Archive Reader context structure.
 * @since 26.0.0
 */
typedef struct ArchiveReadCtx *OH_Archive_Reader_Ctx;
/**
 * @brief Archive streamWrite context structure.
 * @since 26.0.0
 */
typedef struct ArchiveStreamWriteCtx *OH_Archive_StreamWrite_Ctx;
/**
 * @brief Archive streamRead context structure.
 * @since 26.0.0
 */
typedef struct ArchiveStreamReadCtx *OH_Archive_StreamRead_Ctx;

/**
 * @brief Archive format enumeration.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief ZIP format.
     * @since 26.0.0
     */
    OH_ARCHIVE_FMT_ZIP = 0
} OH_Archive_Format;

/**
 * @brief Archive compression method enumeration.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief No compression.
     * @since 26.0.0
     */
    OH_ARCHIVE_NO_COMPRESSION = 0,
    /**
     * @brief Deflate compression method.
     * @since 26.0.0
     */
    OH_ARCHIVE_COMPRESS_DEFLATE = 8
} OH_Archive_CompressMethod;

/**
 * @brief Archive open mode enumeration.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief Create mode.
     * @since 26.0.0
     */
    OH_ARCHIVE_OPEN_MODE_CREATE = 0
} OH_Archive_OpenMode;

/**
 * @brief Archive progress type enumeration.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief Continue compression/decompression operation.
     * @since 26.0.0
     */
    OH_ARCHIVE_PROGRESS_CONTINUE = 0,
    /**
     * @brief Cancel compression/decompression operation.
     * @since 26.0.0
     */
    OH_ARCHIVE_PROGRESS_CANCEL = 1
} OH_Archive_ProgressType;

/**
 * @brief Stream compression information structure.
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Total input bytes before compression.
     * @since 26.0.0
     */
    uint64_t totalInSize;
    /**
     * @brief Total output bytes after compression.
     * @since 26.0.0
     */
    uint64_t totalOutSize;
    /**
     * @brief Checksum, zero if set OH_ARCHIVE_NO_CHECKSUM.
     * @since 26.0.0
     */
    uint32_t checksum;
} OH_Archive_StreamInfo;

/**
 * @brief Hash algorithm used for checksum.
 * @since 26.0.0
 */
typedef enum {
    /**
     * @brief Do not calculate the hash value additionally.
     * @since 26.0.0
     */
    OH_ARCHIVE_NO_CHECKSUM = 0,
    /**
     * @brief Use CRC32 calculate the hash value.
     * @since 26.0.0
     */
    OH_ARCHIVE_CRC32 = 1
} OH_Archive_StreamChecksumAlg;

/**
 * @brief Stream compression configuration structure.
 * @since 26.0.0
 */
typedef struct {
    /**
     * @brief Size of each memory block in bytes (e.g., 32768 bytes, 65536 bytes),
     * for COMPRESS_DEFLATED, block_size>=32768 bytes
     * @since 26.0.0
     */
    uint32_t blockSize;
    /**
     * @brief Number of threads
     * @since 26.0.0
     */
    int32_t threadNum;
    /**
     * @brief Hash algorithm used for checksum.
     * @since 26.0.0
     */
    OH_Archive_StreamChecksumAlg checksum;
    /**
     * @brief Compression method.
     * @since 26.0.0
     */
    OH_Archive_CompressMethod method;
} OH_Archive_Stream_Config;

/**
 * @brief Defines a function pointer type OH_Archive_ProgressHandlerWithData for
 * specifying the progress display handler.
 * @param progress Processing progress percentage.
 * @param userData A pointer to user-defined data, passed when calling the callback.
 * @return Returns a compression/decompression Archive_ProgressType value.
 *         {@link OH_ARCHIVE_PROGRESS_CONTINUE} - continue current compression/decompression operation.
 *         {@link OH_ARCHIVE_PROGRESS_CANCEL} - cancel current compression/decompression operation.
 * @since 26.0.0
 */
typedef OH_Archive_ProgressType (*OH_Archive_ProgressHandlerWithData)(int32_t progress, void *userData);

/**
 * @brief Function pointer type for user-defined callback function to handle compressed data.
 * @param data Pointer to the compressed data.
 * @param size Length of the compressed data.
 * @param userData User-defined context that will be passed back in the callback.
 * @return Number of bytes successfully processed (typically equal to size for success).
 * @since 26.0.0
 */
typedef uint64_t (*OH_Archive_Stream_OutputHandler)(const void* data, uint64_t size, void* userData);

/**
 * @brief Opens an archive file for reading.
 * @note The returned context must be freed by calling OH_Archive_Reader_Close() to release allocated resources.
 * @param infile Path to the source archive file.
 * @return Returns a handle to the archive reader context, or NULL if the operation fails.
 * @since 26.0.0
 * @release archive/OH_Archive_Reader_Close {return}
 */
OH_Archive_Reader_Ctx OH_Archive_Reader_OpenFile(const char *infile);

/**
 * @brief Sets the progress callback function with user data for the archive reader.
 * @param arc Handle to the archive reader context.
 * @param progressHandler The callback function to handle progress updates.
 * @param userData User data, passed when calling the callback.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 *         {@link OH_ARCHIVE_OK} - Execution successful
 *         {@link OH_ARCHIVE_PARAM_ERROR} - Invalid input parameters.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Reader_SetProgressHandlerWithData(OH_Archive_Reader_Ctx arc,
                                                                OH_Archive_ProgressHandlerWithData progressHandler,
                                                                void *userData);
/**
 * @brief Extract all files from the archive.
 * @param arc Handle to the archive reader context.
 * @param outDir Output directory path.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Reader_ExtractAllFile(OH_Archive_Reader_Ctx arc, const char *outDir);

/**
 * @brief Closes an opened archive file and releases associated resources.
 * @param arc Handle to the archive reader context.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Reader_Close(OH_Archive_Reader_Ctx arc);

/**
 * @brief Creates and opens an archive file.
 * @note The returned context must be freed by calling OH_Archive_Writer_Close() to release allocated resources.
 * @param outfile outfile Path to the destination archive file.
 * @param openMode Append mode.
 * @param fmt Archive format.
 * @return Returns a handle to the archive writer context, or NULL if the operation fails.
 * @since 26.0.0
 * @release archive/OH_Archive_Writer_Close {return}
 */
OH_Archive_Writer_Ctx OH_Archive_Writer_OpenFile(const char *outfile,
                                                 OH_Archive_OpenMode openMode,
                                                 OH_Archive_Format fmt);

/**
 * @brief Set the compression method for the archive file
 * @param arc Handle to the archive writer context.
 * @param method Compression method.
 * @param compressLevel Compression level. The value -1 indicates the default compression level.
 *     For OH_ARCHIVE_COMPRESS_DEFLATE, compression level is between 0 and 9. 1 gives best speed,
 *     9 gives best compression, 0 gives no compression, and 6 is default.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 *         {@link OH_ARCHIVE_OK} - Execution successful.
 *         {@link OH_ARCHIVE_PARAM_ERROR} - Invalid input parameters.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Writer_SetCompressMethod(OH_Archive_Writer_Ctx arc,
                                                       OH_Archive_CompressMethod method, int32_t compressLevel);

/**
 * @brief Set the compression progress function for the archive file.
 * @param arc Handle to the archive writer context.
 * @param progressHandler The callback function to handle progress updates.
 * @param userData User data, passed when calling the callback.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 *         {@link OH_ARCHIVE_OK} - Execution successful
 *         {@link OH_ARCHIVE_PARAM_ERROR} - Invalid input parameters.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Writer_SetProgressHandlerWithData(OH_Archive_Writer_Ctx arc,
                                                                OH_Archive_ProgressHandlerWithData progressHandler,
                                                                void *userData);

/**
 * @brief Adds a list of files to the archive.
 * @param arc Handle to the archive writer context.
 * @param infiles Files to be compressed.
 * @param fileNum Number of files.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Writer_Add(OH_Archive_Writer_Ctx arc, const char **infiles, uint64_t fileNum);

/**
 * @brief Closes the archive writer.
 * This function finalizes the archive writing process, flushes any buffered data to the output,
 * and releases the resources associated with the archive context.
 * @param arc Handle to the archive writer context.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_Writer_Close(OH_Archive_Writer_Ctx arc);

/**
 * @brief Calculates the maximum compressed data size for a given source length.
 * @param method Compression method type.
 * @param sourceLen Length of the source data to be compressed.
 * @return Returns the maximum possible compressed data size.
 * @since 26.0.0
 */
uint64_t OH_Archive_BufferWriteCompressBound(OH_Archive_CompressMethod method, uint64_t sourceLen);

/**
 * @brief Writes data to buffer and compresses it.
 * @param dstBuffer Pointer to the destination buffer for storing compressed data.
 * @param dstSize Pointer to the destination buffer size, input the buffer size, output the actual written size.
 * @param srcBuffer Pointer to the source buffer containing data to be compressed.
 * @param srcSize Size of the source buffer data.
 * @param method Compression method type.
 * @param compressLevel Compression level. The value -1 indicates the default compression level.
 *     For OH_ARCHIVE_COMPRESS_DEFLATE, compression level is between 0 and 9. 1 gives best speed,
 *     9 gives best compression, 0 gives no compression, and 6 is default.
 * @return Returns error code, returns OH_ARCHIVE_OK on success.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_BufferWrite(uint8_t *dstBuffer, uint64_t *dstSize,
                                          const uint8_t *srcBuffer, uint64_t srcSize,
                                          OH_Archive_CompressMethod method, int32_t compressLevel);


/**
 * @brief Reads data from buffer and decompresses it.
 * @param dstBuffer Pointer to the destination buffer for storing decompressed data.
 * @param dstSize Pointer to the destination buffer size, input the buffer size, output the actual decompressed size.
 * @param srcBuffer Pointer to the source buffer containing data to be decompressed.
 * @param srcSize Size of the source buffer data.
 * @param method Decompression method type.
 * @return Returns error code, returns OH_ARCHIVE_OK on success.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_BufferRead(uint8_t *dstBuffer, uint64_t *dstSize,
                                         const uint8_t *srcBuffer, uint64_t srcSize,
                                         OH_Archive_CompressMethod method);

/**
 * @brief Creates a compression instance.
 * @param config Compression configuration.
 * @return Returns a pointer to the context compression, or NULL if creation fails.
 * @since 26.0.0
 * @release archive/OH_Archive_StreamWrite_Destroy {return}
 */
OH_Archive_StreamWrite_Ctx OH_Archive_StreamWrite_Create(OH_Archive_Stream_Config config);

/**
 * @brief Starts a compression task, initializing user callback function and user data.
 * @param ctx Compression context.
 * @param outputHandler Callback function for compressed data, user-defined.
 * @param userData User-defined context that will be passed back in the callback. The userData is owned
 *     by the caller and must remain valid until OH_Archive_StreamWrite_End is complete.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamWrite_Start(OH_Archive_StreamWrite_Ctx ctx,
                                                OH_Archive_Stream_OutputHandler outputHandler, void* userData);

/**
 * @brief Sets the compression level for StreamCompress.
 * @param ctx Compression context.
 * @param compressLevel Compression level. For OH_ARCHIVE_COMPRESS_DEFLATE, compression level is between 0 and 9.
 *     1 gives best speed, 9 gives best compression, 0 gives no compression, and 6 is default.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamWrite_SetCompressLevel(OH_Archive_StreamWrite_Ctx ctx, int32_t compressLevel);

/**
 * @brief Forces cancellation of the current blocking operation and wakes up all waiting threads.
 * @param ctx Compression context.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if cancellation is successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamWrite_Cancel(OH_Archive_StreamWrite_Ctx ctx);

/**
 * @brief Submits compression data. This interface will block when the memory pool is full.
 * @param ctx Compression context.
 * @param data Original data to be compressed.
 * @param size Size of the data to be compressed.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if compression is successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamWrite_Update(OH_Archive_StreamWrite_Ctx ctx, const uint8_t* data, uint64_t size);


/**
 * @brief Ends the compression, flushes all remaining data, and cleans up memory.
 * @param ctx Compression context.
 * @param streamInfo Compression information including original data size, compressed data size, and CRC32 value.
 * @return Returns the error code. Returns OH_ARCHIVE_OK if compression is successful.
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamWrite_End(OH_Archive_StreamWrite_Ctx ctx, OH_Archive_StreamInfo *streamInfo);

/**
 * @brief Destroys the compression instance and releases associated resources.
 * @param ctx Compression context to be destroyed.
 * @since 26.0.0
 */
void OH_Archive_StreamWrite_Destroy(OH_Archive_StreamWrite_Ctx ctx);

/**
 * @brief Create a decompression instance
 * @param config Decompression configuration information
 * @return Returns a pointer to the decompression context, or NULL if creation fails
 * @since 26.0.0
 * @release archive/OH_Archive_StreamRead_Destroy {return}
 */
OH_Archive_StreamRead_Ctx OH_Archive_StreamRead_Create(OH_Archive_Stream_Config config);

/**
 * @brief Start a decompression task, initialize user callback function and user data
 * @param ctx Decompression context
 * @param outputHandler User-defined callback function for handling decompressed data
 * @param userData User-defined context data that will be passed back in the callback. The userData is owned by
 *     the caller and must remain valid until OH_Archive_StreamRead_End is complete.
 * @return Returns error code, returns OH_ARCHIVE_OK if successful
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamRead_Start(OH_Archive_StreamRead_Ctx ctx,
                                               OH_Archive_Stream_OutputHandler outputHandler, void* userData);

/**
 * @brief Force cancellation of the current blocking operation and wake up all waiting threads
 * @param ctx Decompression context
 * @return Returns error code, returns OH_ARCHIVE_OK if cancellation is successful
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamRead_Cancel(OH_Archive_StreamRead_Ctx ctx);

/**
 * @brief Submit decompression data. This interface will block when the memory pool is full
 * @param ctx Decompression context
 * @param data Data to be decompressed
 * @param size Size of the data
 * @return Returns error code, returns OH_ARCHIVE_OK if successful
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamRead_Update(OH_Archive_StreamRead_Ctx ctx, const uint8_t* data, uint64_t size);

/**
 * @brief End the decompression, flush all remaining data, and clean up memory
 * @param ctx Decompression context
 * @param streamInfo Decompression information including original data size, compressed data size, and CRC32 value.
 * @return Returns error code, returns OH_ARCHIVE_OK if successful
 * @since 26.0.0
 */
OH_Archive_ErrCode OH_Archive_StreamRead_End(OH_Archive_StreamRead_Ctx ctx, OH_Archive_StreamInfo *streamInfo);

/**
 * @brief Destroy the decompression instance and release associated resources
 * @param ctx Decompression context to be destroyed
 * @since 26.0.0
 */
void OH_Archive_StreamRead_Destroy(OH_Archive_StreamRead_Ctx ctx);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
/** @} */
#endif /* FILE_MANAGEMENT_ARCHIVE_OH_ARCHIVE_H */