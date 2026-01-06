/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_TEST_UNITTEST_COMMON_UTILS_UT_FILE_UTILS_H
#define INTERFACES_TEST_UNITTEST_COMMON_UTILS_UT_FILE_UTILS_H

#include <filesystem>
#include <string>

namespace OHOS::FileManagement::ModuleFileIO::Test {
namespace fs = std::filesystem;

class FileUtils {
public:
    inline static const std::string testRootDir = "/data/unittest";
    inline static const std::string defaultTempDir = fs::temp_directory_path(); // /data/local/tmp

public:
    static bool IsDirectory(const fs::path &path);
    static bool IsFile(const fs::path &path);
    static off_t GetFileSize(const int fd);
    static off_t GetFileSize(const fs::path &path);
    static std::tuple<bool, std::string> ReadTextFileContent(const fs::path &path);
    static bool Exists(const fs::path &path);
    static bool CreateFile(const fs::path &path);
    static bool CreateFile(const fs::path &path, const std::string &content);
    static bool CreateFile(const fs::path &path, int32_t len);
    static bool CreateDirectories(const fs::path &dir, bool deleteIfExist = false);
    static bool RemoveAll(const fs::path &path);

private:
    static bool DoIsDirectory(const fs::path &path);
    static bool DoIsFile(const fs::path &path);
    static off_t DoGetFileSize(const int fd);
    static off_t DoGetFileSize(const fs::path &path);
    static std::tuple<bool, std::string> DoReadTextFileContent(const fs::path &path);
    static bool DoExists(const fs::path &path);
    static bool DoCreateTextFile(const fs::path &path, const std::string &content);
    static bool DoCreateBinaryFile(const fs::path &path, int32_t len);
    static bool DoCreateDirectories(const fs::path &dir, bool deleteIfExist = false);
    static bool DoRemoveAll(const fs::path &path);
    static std::tuple<bool, fs::path> CheckAndNormalizePath(const fs::path &path);
};

} // namespace OHOS::FileManagement::ModuleFileIO::Test
#endif // INTERFACES_TEST_UNITTEST_COMMON_UTILS_UT_FILE_UTILS_H