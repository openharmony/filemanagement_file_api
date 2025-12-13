/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "ut_file_utils.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/stat.h>

#ifdef INTERFACES_TEST_UNITTEST_COMMON_MOCK_UNISTD_MOCK_H
#include "unistd_mock.h"
#endif

namespace OHOS::FileManagement::ModuleFileIO::Test {

template <typename Func, typename... Args>
auto InvokeWithMockDisabled(Func func, Args &&...args)
{
#ifdef INTERFACES_TEST_UNITTEST_COMMON_MOCK_UNISTD_MOCK_H
    bool unistdMocked = UnistdMock::IsMockable();
    // If UnistdMock has been enabled, then disable it to prevent filesystem (unistd) function from failing.
    if (unistdMocked) {
        UnistdMock::DisableMock();
    }
#endif

    auto result = std::invoke(func, std::forward<Args>(args)...);

#ifdef INTERFACES_TEST_UNITTEST_COMMON_MOCK_UNISTD_MOCK_H
    // Restore UnistdMock to its previous state.
    if (unistdMocked) {
        UnistdMock::EnableMock();
    }
#endif
    return result;
}

off_t FileUtils::GetFileSize(const int fd)
{
    return InvokeWithMockDisabled(
        [](int f) {
            return FileUtils::DoGetFileSize(f);
        },
        fd);
}

off_t FileUtils::DoGetFileSize(const int fd)
{
    if (fd < 0) {
        GTEST_LOG_(ERROR) << "Invalid fd: " << fd;
        return -1;
    }

    struct stat st;
    int ret = fstat(fd, &st);
    if (ret != 0) {
        GTEST_LOG_(ERROR) << "Get file stat by fd failed! ret: " << ret << ", errno: " << errno;
        return -1;
    }

    if (!S_ISREG(st.st_mode)) {
        GTEST_LOG_(ERROR) << "Is not a regular file!";
        return -1;
    }

    return st.st_size;
}

off_t FileUtils::GetFileSize(const fs::path &path)
{
    return InvokeWithMockDisabled(
        [](const fs::path &p) {
            return FileUtils::DoGetFileSize(p);
        },
        path);
}

off_t FileUtils::DoGetFileSize(const fs::path &path)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return -1;
    }

    std::error_code err;
    if (!fs::exists(normalizedPath, err) || !fs::is_regular_file(normalizedPath, err) || err) {
        GTEST_LOG_(ERROR) << "Non-existent or invalid file! Error: " << err.message() << ". Code: " << err.value();
        return -1;
    }

    struct stat st;
    int ret = stat(normalizedPath.c_str(), &st);
    if (ret != 0) {
        GTEST_LOG_(ERROR) << "Get file stat by path failed! ret: " << ret << ", errno: " << errno;
        return -1;
    }

    return st.st_size;
}

std::tuple<bool, std::string> FileUtils::ReadTextFileContent(const fs::path &path)
{
    return InvokeWithMockDisabled(&FileUtils::DoReadTextFileContent, path);
}

std::tuple<bool, std::string> FileUtils::DoReadTextFileContent(const fs::path &path)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return { false, "" };
    }
    std::error_code err;
    if (!fs::exists(normalizedPath, err) || !fs::is_regular_file(normalizedPath, err) || err) {
        GTEST_LOG_(ERROR) << "Non-existent or invalid file! Error: " << err.message() << ". Code: " << err.value();
        return { false, "" };
    }
    std::ifstream file(normalizedPath, std::ios::in);
    if (!file.is_open()) {
        GTEST_LOG_(ERROR) << "Failed to open the text file! errno: " << errno;
        return { false, "" };
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return { true, ss.str() };
}

bool FileUtils::Exists(const fs::path &path)
{
    return InvokeWithMockDisabled(&FileUtils::DoExists, path);
}

bool FileUtils::DoExists(const fs::path &path)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return false;
    }
    std::error_code err;
    bool ret = fs::exists(normalizedPath, err);
    if (err) {
        GTEST_LOG_(ERROR) << "Failed to check if the path exists! Error: " << err.message()
                          << ". Code: " << err.value();
        return false;
    }
    return ret;
}

bool FileUtils::CreateFile(const fs::path &path)
{
    return CreateFile(path, "");
}

bool FileUtils::CreateFile(const fs::path &path, const std::string &content)
{
    return InvokeWithMockDisabled(&FileUtils::DoCreateTextFile, path, content);
}

bool FileUtils::CreateFile(const fs::path &path, int32_t len)
{
    return InvokeWithMockDisabled(&FileUtils::DoCreateBinaryFile, path, len);
}

bool FileUtils::DoCreateTextFile(const fs::path &path, const std::string &content)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return false;
    }
    std::error_code err;
    bool ret = fs::create_directories(normalizedPath.parent_path(), err);
    if (!ret && err) {
        GTEST_LOG_(ERROR) << "Failed to create parent directory! Error: " << err.message() << ". Code: " << err.value();
        return false;
    }
    err.clear();
    // If the file already exists, its contents will be overwritten; if it does not exist, a new file will be created.
    std::ofstream file(normalizedPath);
    if (!file.is_open()) {
        GTEST_LOG_(ERROR) << "Failed to create or open test file! errno: " << errno;
        return false;
    }
    file << content;
    return true;
}

bool FileUtils::DoCreateBinaryFile(const fs::path &path, int32_t len)
{
    constexpr int32_t maxFileSize = 1024 * 1024 * 1024; // 1GB
    if (len <= 0 || len > maxFileSize) {
        GTEST_LOG_(ERROR) << "Invalid len! Must be 0 < len <= " << maxFileSize;
        return false;
    }
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return false;
    }
    std::error_code err;
    bool ret = fs::create_directories(normalizedPath.parent_path(), err);
    if (!ret && err) {
        GTEST_LOG_(ERROR) << "Failed to create parent directory! Error: " << err.message() << ". Code: " << err.value();
        return false;
    }
    err.clear();
    // If the file already exists, its contents will be overwritten; if it does not exist, a new file will be created.
    std::ofstream file(normalizedPath, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        GTEST_LOG_(ERROR) << "Failed to create or open test file! errno: " << errno;
        return false;
    }
    // Move the pointer to the byte before the specified 'len', which will expand the file.
    file.seekp(len - 1);
    // And then write one byte, which ensures that the file size is 'len'.
    file.write("", 1);
    return true;
}

bool FileUtils::CreateDirectories(const fs::path &dir, bool deleteIfExist)
{
    return InvokeWithMockDisabled(&FileUtils::DoCreateDirectories, dir, deleteIfExist);
}

bool FileUtils::DoCreateDirectories(const fs::path &dir, bool deleteIfExist)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(dir);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before creating!";
        return false;
    }
    std::error_code err;
    if (deleteIfExist && fs::exists(normalizedPath, err)) {
        fs::remove_all(normalizedPath, err);
        if (err) {
            GTEST_LOG_(ERROR) << "Failed to delete the existing path! Error: " << err.message()
                              << ". Code: " << err.value();
            return false;
        }
    }
    if (err) {
        GTEST_LOG_(ERROR) << "Failed to check if the path exists! Error: " << err.message()
                          << ". Code: " << err.value();
        return false;
    }
    err.clear();
    bool ret = fs::create_directories(normalizedPath, err);
    if (!ret && err) {
        GTEST_LOG_(ERROR) << "Failed to create test directory! Error: " << err.message() << ". Code: " << err.value();
        return false;
    }
    return true;
}

bool FileUtils::RemoveAll(const fs::path &path)
{
    return InvokeWithMockDisabled(&FileUtils::DoRemoveAll, path);
}

bool FileUtils::DoRemoveAll(const fs::path &path)
{
    auto [succ, normalizedPath] = CheckAndNormalizePath(path);
    if (!succ) {
        GTEST_LOG_(ERROR) << "Failed to check and normalize path before removing!";
        return false;
    }
    std::error_code err;
    if (fs::exists(normalizedPath, err)) {
        fs::remove_all(normalizedPath, err);
        if (err) {
            GTEST_LOG_(ERROR) << "Failed to delete the existing path! Error: " << err.message()
                              << ". Code: " << err.value();
            return false;
        }
    }
    if (err) {
        GTEST_LOG_(ERROR) << "Failed to check if the path exists! Error: " << err.message()
                          << ". Code: " << err.value();
        return false;
    }
    return true;
}

std::tuple<bool, fs::path> FileUtils::CheckAndNormalizePath(const fs::path &path)
{
    if (path.empty()) {
        GTEST_LOG_(ERROR) << "Path is empty!";
        return { false, {} };
    }
    std::error_code err;
    const fs::path normalizedPath = fs::weakly_canonical(path, err);
    if (err) {
        GTEST_LOG_(ERROR) << "Failed to normalize path! Error: " << err.message() << ". Code: " << err.value();
        return { false, {} };
    }
    return { true, normalizedPath };
}

} // namespace OHOS::FileManagement::ModuleFileIO::Test
