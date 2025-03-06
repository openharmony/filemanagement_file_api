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

#include "listfile_core.h"

#include <fnmatch.h>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <sys/stat.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

thread_local OptionArgs g_optionArgsCore;

static bool CheckSuffix(const vector<string> &suffixs)
{
    for (string suffix : suffixs) {
        if (suffix.length() <= 1 || suffix.length() > MAX_SUFFIX_LENGTH) {
            return false;
        }
        if (suffix[0] != '.') {
            return false;
        }
        for (size_t i = 1; i < suffix.length(); i++) {
            if (!isalnum(suffix[i])) {
                return false;
            }
        }
    }
    return true;
}

static bool ValidFileFilterParam(FsFileFilter &fsFilter, FileFilter *filter)
{
    auto suffixs = fsFilter.GetSuffix();
    if (fsFilter.GetSuffix().has_value()) {
        vector<string> suffixs = fsFilter.GetSuffix().value();
        if (!CheckSuffix(suffixs) || suffixs.size() == 0) {
            HILOGE("Invalid suffix.");
            return false;
        }
        filter->SetSuffix(suffixs);
    }

    if (fsFilter.GetDisplayName().has_value()) {
        vector<string> displayNames = fsFilter.GetDisplayName().value();
        if (displayNames.size() == 0) {
            HILOGE("Invalid displayName.");
            return false;
        }
        filter->SetDisplayName(displayNames);
    }

    if (fsFilter.GetFileSizeOver().has_value()) {
        int64_t fileSizeOver = fsFilter.GetFileSizeOver().value();
        if (fileSizeOver < 0) {
            HILOGE("Failed to get fileSizeOver prop.");
            return false;
        }
        filter->SetFileSizeOver(fileSizeOver);
    }

    if (fsFilter.GetLastModifiedAfter().has_value()) {
        double lastModifiedAfter = fsFilter.GetLastModifiedAfter().value();
        if (lastModifiedAfter < 0) {
            HILOGE("Failed to get lastModifiedAfter prop.");
            return false;
        }
        filter->SetLastModifiedAfter(lastModifiedAfter);
    }

    return true;
}

static bool ValidOptionParam(const string &path, const optional<FsListFileOptions> &opt, OptionArgs &optionArgs)
{
    g_optionArgsCore.Clear();
    g_optionArgsCore.path = path;

    if (opt.has_value()) {
        auto op = opt.value();
        if (op.listNum < 0) {
            HILOGE("Failed to get listNum prop");
            return false;
        }

        optionArgs.recursion = op.recursion;
        optionArgs.listNum = op.listNum;

        if (op.filter.has_value()) {
            bool ret = ValidFileFilterParam(op.filter.value(), &(optionArgs.filter));
            if (!ret) {
                HILOGE("Failed to get filter prop.");
                return false;
            }
        }
    }

    return true;
}

static bool FilterSuffix(const vector<string> &suffixs, const struct dirent &filename)
{
    if (filename.d_type == DT_DIR) {
        return true;
    }
    size_t found = string(filename.d_name).rfind('.');
    if (found == std::string::npos) {
        return false;
    }
    string suffixStr = string(filename.d_name).substr(found);
    for (const auto &iter : suffixs) {
        if (iter == suffixStr) {
            return true;
        }
    }
    return false;
}

static bool FilterDisplayname(const vector<string> &displaynames, const struct dirent &filename)
{
    for (const auto &iter : displaynames) {
        int ret = fnmatch(iter.c_str(), filename.d_name, FNM_PATHNAME | FNM_PERIOD);
        if (ret == 0) {
            return true;
        }
    }
    return false;
}

static bool FilterFilesizeOver(const int64_t fFileSizeOver, const struct dirent &filename)
{
    if (fFileSizeOver < 0) {
        return true;
    }
    struct stat info;
    string stPath = (g_optionArgsCore.path + '/' + string(filename.d_name));
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        HILOGE("Failed to stat file.");
        return false;
    }
    if (info.st_size > fFileSizeOver) {
        return true;
    }
    return false;
}

static bool FilterLastModifyTime(const double lastModifiedAfter, const struct dirent &filename)
{
    if (lastModifiedAfter < 0) {
        return true;
    }
    struct stat info;
    string stPath = g_optionArgsCore.path + '/' + string(filename.d_name);
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        HILOGE("Failed to stat file.");
        return false;
    }
    if (static_cast<double>(info.st_mtime) > lastModifiedAfter) {
        return true;
    }
    return false;
}

static bool FilterResult(const struct dirent &filename)
{
    vector<string> fSuffixs = g_optionArgsCore.filter.GetSuffix();
    if (!FilterSuffix(fSuffixs, filename) && fSuffixs.size() > 0) {
        return false;
    }
    vector<string> fDisplaynames = g_optionArgsCore.filter.GetDisplayName();
    if (!FilterDisplayname(fDisplaynames, filename) && fDisplaynames.size() > 0) {
        return false;
    }
    int64_t fFileSizeOver = g_optionArgsCore.filter.GetFileSizeOver();
    if (!FilterFilesizeOver(fFileSizeOver, filename)) {
        return false;
    }
    double fLastModifiedAfter = g_optionArgsCore.filter.GetLastModifiedAfter();
    if (!FilterLastModifyTime(fLastModifiedAfter, filename)) {
        return false;
    }
    g_optionArgsCore.countNum++;
    return true;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILTER_DISMATCH;
    }

    if (g_optionArgsCore.countNum < g_optionArgsCore.listNum || g_optionArgsCore.listNum == 0) {
        if ((filename->d_type == DT_DIR && g_optionArgsCore.recursion) || FilterResult(*filename)) {
            return FILTER_MATCH;
        }
    }
    return FILTER_DISMATCH;
}

static void Deleter(struct NameListArg *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
    arg->namelist = nullptr;
    delete arg;
    arg = nullptr;
}

static int FilterFileRes(const string &path, vector<string> &dirents)
{
    unique_ptr<struct NameListArg, decltype(Deleter) *> pNameList = { new (nothrow) struct NameListArg, Deleter };
    if (!pNameList) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    if (num < 0) {
        HILOGE("Failed to scan dir");
        return errno;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        dirents.emplace_back(pNameList->namelist[i]->d_name);
    }
    return ERRNO_NOERR;
}

static int RecursiveFunc(const string &path, vector<string> &dirents)
{
    unique_ptr<struct NameListArg, decltype(Deleter) *> pNameList = { new (nothrow) struct NameListArg, Deleter };
    if (!pNameList) {
        HILOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    if (num < 0) {
        HILOGE("Failed to scan dir");
        return errno;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        if ((*(pNameList->namelist[i])).d_type == DT_REG) {
            dirents.emplace_back(path + '/' + pNameList->namelist[i]->d_name);
        } else if ((*(pNameList->namelist[i])).d_type == DT_DIR) {
            string pathTemp = g_optionArgsCore.path;
            g_optionArgsCore.path += '/' + string((*(pNameList->namelist[i])).d_name);
            int ret = RecursiveFunc(g_optionArgsCore.path, dirents);
            if (ret != ERRNO_NOERR) {
                return ret;
            }
            g_optionArgsCore.path = pathTemp;
        }
    }
    return ERRNO_NOERR;
}

static void DoListFileVector(const string &path, vector<string> &dirents, bool recursion)
{
    if (recursion) {
        for (size_t i = 0; i < dirents.size(); i++) {
            dirents[i] = dirents[i].substr(path.length());
        }
    }
}

FsResult<std::vector<std::string>> ListFileCore::DoListFile(const string &path, const optional<FsListFileOptions> &opt)
{
    if (!ValidOptionParam(path, opt, g_optionArgsCore)) {
        HILOGE("Invalid options");
        return FsResult<std::vector<std::string>>::Error(EINVAL);
    }

    vector<string> direntsRes;
    int ret = 0;
    ret = g_optionArgsCore.recursion ? RecursiveFunc(path, direntsRes) : FilterFileRes(path, direntsRes);
    if (ret) {
        return FsResult<std::vector<std::string>>::Error(ret);
    }
    DoListFileVector(path, direntsRes, g_optionArgsCore.recursion);
    g_optionArgsCore.Clear();

    return FsResult<std::vector<std::string>>::Success(direntsRes);
}

} // namespace OHOS::FileManagement::ModuleFileIO