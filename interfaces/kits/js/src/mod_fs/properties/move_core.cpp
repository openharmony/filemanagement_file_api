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

 #include "move_core.h"

 #ifdef __MUSL__
 #include <filesystem>
 #else
 #include <sys/stat.h>
 #endif
 
 #include <tuple>
 #include <unistd.h>
 
 #include "filemgmt_libhilog.h"
 #include "uv.h"
 
 namespace OHOS {
 namespace FileManagement {
 namespace ModuleFileIO {
 using namespace std;
 
 #ifdef __MUSL__
 static bool CheckDir(const string &path)
 {
     std::error_code errCode;
     if (!filesystem::is_directory(filesystem::status(path, errCode))) {
         return false;
     }
     return true;
 }
 #else
 static bool CheckDir(const string &path)
 {
     struct stat fileInformation;
     if (stat(path.c_str(), &fileInformation) == 0) {
         if (fileInformation.st_mode & S_IFDIR) {
             return true;
         }
     } else {
         HILOGE("Failed to stat file");
     }
     return false;
 }
 #endif
 
 static tuple<bool, string, string, int> ValidMoveArg(const string &src, const string &dest, const optional<int> &mode)
 {
     if (CheckDir(src)) {
         HILOGE("Invalid src");
         return { false, nullptr, nullptr, 0 };
     }
     if (CheckDir(dest)) {
         HILOGE("Invalid dest");
         return { false, nullptr, nullptr, 0 };
     }
     int modeType = 0;
     if (mode.has_value()) {
         modeType = mode.value();
         if ((modeType != MODE_FORCE_MOVE && modeType != MODE_THROW_ERR)) {
             HILOGE("Invalid mode");
             return { false, nullptr, nullptr, 0 };
         }
     }
     return { true, move(src), move(dest), modeType };
 }
 
 static int CopyAndDeleteFile(const string &src, const string &dest)
 {
     std::unique_ptr<uv_fs_t, decltype(FsUtils::FsReqCleanup)*> stat_req = {
         new (std::nothrow) uv_fs_t, FsUtils::FsReqCleanup };
     if (!stat_req) {
         HILOGE("Failed to request heap memory.");
         return ENOMEM;
     }
     int ret = uv_fs_stat(nullptr, stat_req.get(), src.c_str(), nullptr);
     if (ret < 0) {
         HILOGE("Failed to stat srcPath");
         return ret;
     }
 #if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
     filesystem::path dstPath(dest);
     std::error_code errCode;
     if (filesystem::exists(dstPath, errCode)) {
         if (!filesystem::remove(dstPath, errCode)) {
             HILOGE("Failed to remove dest file, error code: %{public}d", errCode.value());
             return errCode.value();
         }
     }
     filesystem::path srcPath(src);
     if (!filesystem::copy_file(srcPath, dstPath, filesystem::copy_options::overwrite_existing, errCode)) {
         HILOGE("Failed to copy file, error code: %{public}d", errCode.value());
         return errCode.value();
     }
 #else
     uv_fs_t copyfile_req;
     ret = uv_fs_copyfile(nullptr, &copyfile_req, src.c_str(), dest.c_str(), MODE_FORCE_MOVE, nullptr);
     uv_fs_req_cleanup(&copyfile_req);
     if (ret < 0) {
         HILOGE("Failed to move file using copyfile interface.");
         return ret;
     }
 #endif
     uv_fs_t unlink_req;
     ret = uv_fs_unlink(nullptr, &unlink_req, src.c_str(), nullptr);
     if (ret < 0) {
         HILOGE("Failed to unlink src file");
         int result = uv_fs_unlink(nullptr, &unlink_req, dest.c_str(), nullptr);
         if (result < 0) {
             HILOGE("Failed to unlink dest file");
             return result;
         }
         uv_fs_req_cleanup(&unlink_req);
         return ret;
     }
     uv_fs_req_cleanup(&unlink_req);
     return ERRNO_NOERR;
 }
 
 static int RenameFile(const string &src, const string &dest)
 {
     int ret = 0;
     uv_fs_t rename_req;
     ret = uv_fs_rename(nullptr, &rename_req, src.c_str(), dest.c_str(), nullptr);
     if (ret < 0 && (string_view(uv_err_name(ret)) == "EXDEV")) {
         return CopyAndDeleteFile(src, dest);
     }
     if (ret < 0) {
         HILOGE("Failed to move file using rename syscall.");
         return ret;
     }
     return ERRNO_NOERR;
 }
 
 static int MoveFile(const string &src, const string &dest, int mode)
 {
     uv_fs_t access_req;
     int ret = uv_fs_access(nullptr, &access_req, src.c_str(), W_OK, nullptr);
     if (ret < 0) {
         HILOGE("Failed to move src file due to doesn't exist or hasn't write permission");
         uv_fs_req_cleanup(&access_req);
         return ret;
     }
     if (mode == MODE_THROW_ERR) {
         ret = uv_fs_access(nullptr, &access_req, dest.c_str(), 0, nullptr);
         uv_fs_req_cleanup(&access_req);
         if (ret == 0) {
             HILOGE("Failed to move file due to existing destPath with MODE_THROW_ERR.");
             return EEXIST;
         }
         if (ret < 0 && (string_view(uv_err_name(ret)) != "ENOENT")) {
             HILOGE("Failed to access destPath with MODE_THROW_ERR.");
             return ret;
         }
     } else {
         uv_fs_req_cleanup(&access_req);
     }
     return RenameFile(src, dest);
 }
 
 FsResult<void> MoveCore::DoMove(const std::string &src, const std::string &dest, const std::optional<int> &mode)
 {
     auto [succ, srcPath, destPath, modeType] = ValidMoveArg(src, dest, mode);
     if (!succ) {
         return FsResult<void>::Error(EINVAL);
     }
     int ret = MoveFile(move(srcPath), move(destPath), modeType);
     if (ret) {
         return FsResult<void>::Error(ret);
     }
     return FsResult<void>::Success();
 }
 
 } // namespace ModuleFileIO
 } // namespace FileManagement
 } // namespace OHOS