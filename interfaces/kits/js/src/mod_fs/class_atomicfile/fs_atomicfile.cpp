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

#include "fs_atomicfile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <memory>
#include <sys/stat.h>

#include "fs_atomicfile_entity.h"
#include "filemgmt_libfs.h"
#include "file_instantiator.h"
#include "file_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace fs = std::filesystem;

const std::string READ_STREAM_CLASS = "ReadStream";
const std::string WRITE_STREAM_CLASS = "WriteStream";
const std::string TEMP_FILE_SUFFIX = "_XXXXXX";

FsAtomicFileEntity *FsAtomicFile::GetEntity()
{
    if (!entity) {
        return nullptr;
    }
    return entity.get();
}

void FsAtomicFile::FinalizeCallback(void *finalizeData, [[maybe_unused]] void *finalizeHint)
{
    BufferData *bufferData = static_cast<BufferData *>(finalizeData);
    delete bufferData;
}

string FsAtomicFile::GetPath()
{
    return entity->baseFileName;
}

FsResult<FsFile *> FsAtomicFile::GetBaseFile()
{
    if (entity == nullptr) {
        HILOGE("Failed to get atomicFileEntity");
        return FsResult<FsFile *>::Error(UNKNOWN_ERR);
    }

    if (entity->baseFileName.size() >= PATH_MAX) {
        HILOGE("Base file name is too long");
        return FsResult<FsFile *>::Error(UNKNOWN_ERR);
    }

    auto absolutePath = std::make_unique<char[]>(PATH_MAX);
    char *result = realpath(entity->baseFileName.c_str(), absolutePath.get());
    if (result == nullptr) {
        HILOGE("Failed to resolve real path, err:%{public}d", errno);
        return FsResult<FsFile *>::Error(errno);
    }

    int fd = open(result, O_RDONLY);
    if (fd < 0) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        return FsResult<FsFile *>::Error(errno);
    }

    return FileInstantiator::InstantiateFile(fd, entity->baseFileName, false);
}

static std::tuple<std::unique_ptr<BufferData>, int32_t> ReadFileToBuffer(FILE *fp)
{
    int fd = fileno(fp);
    if (fd < 0) {
        HILOGE("Failed to get file descriptor, err:%{public}d", errno);
        return { nullptr, UNKNOWN_ERR };
    }

    struct stat fileStat {};
    if (fstat(fd, &fileStat) < 0) {
        HILOGE("Failed to get file stats, err:%{public}d", errno);
        return { nullptr, errno };
    }

    long fileSize = fileStat.st_size;
    if (fileSize <= 0) {
        HILOGE("Invalid file size");
        return { nullptr, EIO };
    }

    auto bufferData = std::make_unique<BufferData>();
    bufferData->buffer = new (std::nothrow) uint8_t[fileSize];
    if (bufferData->buffer == nullptr) {
        HILOGE("Failed to allocate memory");
        return { nullptr, ENOMEM };
    }

    bufferData->length = fread(bufferData->buffer, sizeof(uint8_t), fileSize, fp);
    if ((bufferData->length != static_cast<size_t>(fileSize) && !feof(fp)) || ferror(fp)) {
        HILOGE("Failed to read file, actual length is:%zu, fileSize:%ld", bufferData->length, fileSize);
        delete[] bufferData->buffer;
        bufferData->buffer = nullptr;
        bufferData->length = 0;
        return { nullptr, EIO };
    }
    return { std::move(bufferData), ERRNO_NOERR };
}

FsResult<unique_ptr<BufferData>> FsAtomicFile::ReadFully()
{
    if (entity == nullptr) {
        HILOGE("Failed to get atomicFileEntity");
        return FsResult<unique_ptr<BufferData>>::Error(UNKNOWN_ERR);
    }

    auto absolutePath = std::make_unique<char[]>(PATH_MAX);
    char *result = realpath(entity->baseFileName.c_str(), absolutePath.get());
    if (result == nullptr) {
        HILOGE("Failed to resolve file real path, err:%{public}d", errno);
        return FsResult<unique_ptr<BufferData>>::Error(errno);
    }

    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(result, "rb"), &std::fclose);
    if (!file) {
        HILOGE("Failed to open file, err:%{public}d", errno);
        return FsResult<unique_ptr<BufferData>>::Error(errno);
    }

    auto [bufferData, errCode] = ReadFileToBuffer(file.get());
    if (errCode != ERRNO_NOERR) {
        return FsResult<unique_ptr<BufferData>>::Error(errCode);
    }
    return FsResult<unique_ptr<BufferData>>::Success(move(bufferData));
}

FsResult<string> FsAtomicFile::StartWrite()
{
    fs::path filePath = entity->newFileName;
    fs::path parentPath = filePath.parent_path();
    if (access(parentPath.c_str(), F_OK) != 0) {
        HILOGE("Parent directory does not exist, err:%{public}d", errno);
        return FsResult<string>::Error(ENOENT);
    }

    char *tmpfile = const_cast<char *>(entity->newFileName.c_str());
    int fd = mkstemp(tmpfile);
    if (fd == -1) {
        HILOGE("Fail to create tmp file err:%{public}d!", errno);
        return FsResult<string>::Error(ENOENT);
    }
    close(fd);

    return FsResult<string>::Success(entity->newFileName);
}

FsResult<void> FsAtomicFile::FinishWrite()
{
    if (std::rename(entity->newFileName.c_str(), entity->baseFileName.c_str()) != 0) {
        HILOGE("Rename failed");
        return FsResult<void>::Error(errno);
    }
    std::string tmpNewFileName = entity->baseFileName;
    entity->newFileName = tmpNewFileName.append(TEMP_FILE_SUFFIX);

    return FsResult<void>::Success();
}

FsResult<void> FsAtomicFile::FailWrite()
{
    if (!fs::remove(entity->newFileName)) {
        HILOGW("Failed to remove file");
        return FsResult<void>::Error(errno);
    }
    std::string tmpNewFileName = entity->baseFileName;
    entity->newFileName = tmpNewFileName.append(TEMP_FILE_SUFFIX);

    return FsResult<void>::Success();
}

FsResult<void> FsAtomicFile::Delete()
{
    auto rafentity = GetEntity();
    if (rafentity == nullptr) {
        HILOGE("Failed to get atomicFileEntity");
        return FsResult<void>::Error(UNKNOWN_ERR);
    }

    bool errFlag = false;
    std::error_code fsErrcode;
    if (fs::exists(rafentity->newFileName, fsErrcode) && !fs::remove(rafentity->newFileName, fsErrcode)) {
        errFlag = true;
    }
    if (fs::exists(rafentity->baseFileName, fsErrcode) && !fs::remove(rafentity->baseFileName, fsErrcode)) {
        errFlag = true;
    }
    if (errFlag) {
        HILOGE("Failed to remove file, err:%{public}s", fsErrcode.message().c_str());
        return FsResult<void>::Error(fsErrcode.value());
    }

    rafentity->newFileName.clear();
    rafentity->baseFileName.clear();
    return FsResult<void>::Success();
}

FsResult<FsAtomicFile *> FsAtomicFile::Constructor(string path)
{
    auto atomicFileEntity = CreateUniquePtr<FsAtomicFileEntity>();
    if (atomicFileEntity == nullptr) {
        HILOGE("Failed to request heap memory");
        return FsResult<FsAtomicFile *>::Error(ENOMEM);
    }
    atomicFileEntity->baseFileName = path;
    atomicFileEntity->newFileName = path.append(TEMP_FILE_SUFFIX);

    auto file = new (std::nothrow) FsAtomicFile(move(atomicFileEntity));
    if (file == nullptr) {
        HILOGE("Failed to create FsAtomicFile");
        return FsResult<FsAtomicFile *>::Error(ENOMEM);
    }

    return FsResult<FsAtomicFile *>::Success(file);
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS