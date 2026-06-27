/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <fnmatch.h>

#include "file_fs_impl.h"
#include "n_error.h"
#include "securec.h"
#include "list_file.h"
#include "utils.h"

using namespace std;
using namespace OHOS::CJSystemapi;
using namespace OHOS::FileManagement::LibN;
using namespace OHOS::FileManagement::ModuleFileIO;

static thread_local OptionArgs g_optionArgs;

namespace {

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

static vector<string> CArrStringToVector(CArrString cArr)
{
    vector<string> ret;
    for (int64_t i = 0; i < cArr.size; i++) {
        ret.push_back(cArr.head[i]);
    }
    return ret;
}

static bool GetFileFilterParam(CFilter cFilter, FileFilter *filter)
{
    if (cFilter.suffix.size != 0) {
        auto suffix = CArrStringToVector(cFilter.suffix);
        if (!CheckSuffix(suffix) || suffix.size() == 0) {
            LOGE("Invalid suffix.");
            return false;
        }
        filter->SetSuffix(suffix);
    }

    if (cFilter.displayName.size != 0) {
        auto displayName = CArrStringToVector(cFilter.displayName);
        if (displayName.size() == 0) {
            LOGE("Invalid displayName.");
            return false;
        }
        filter->SetDisplayName(displayName);
    }

    if (cFilter.fileSizeOver != -1) {
        LOGI("GetFileFilterParam fileSizeOver");
        filter->SetFileSizeOver(cFilter.fileSizeOver);
    }
    
    double epsilon = 1e-6;
    if (fabs(cFilter.lastModifiedAfter + -1.0) > epsilon) {
        LOGI("GetFileFilterParam lastModifiedAfter");
        filter->SetLastModifiedAfter(cFilter.lastModifiedAfter);
    }
    return true;
}

static bool FilterSuffix(const vector<string> &suffixs, const struct dirent &filename)
{
    if (filename.d_type == DT_DIR) {
        return true;
    }
    size_t found = string(filename.d_name).rfind('.');
    if (found == string::npos) {
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
    string stPath = (g_optionArgs.path + '/' + string(filename.d_name));
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        LOGE("Failed to stat file.");
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
    string stPath = g_optionArgs.path + '/' + string(filename.d_name);
    int32_t res = stat(stPath.c_str(), &info);
    if (res != 0) {
        LOGE("Failed to stat file.");
        return false;
    }
    if (static_cast<double>(info.st_mtime) > lastModifiedAfter) {
        return true;
    }
    return false;
}

static bool FilterResult(const struct dirent &filename)
{
    vector<string> fSuffixs = g_optionArgs.filter.GetSuffix();
    if (!FilterSuffix(fSuffixs, filename) && fSuffixs.size() > 0) {
        return false;
    }
    vector<string> fDisplaynames = g_optionArgs.filter.GetDisplayName();
    if (!FilterDisplayname(fDisplaynames, filename) && fDisplaynames.size() > 0) {
        return false;
    }
    int64_t fFileSizeOver = g_optionArgs.filter.GetFileSizeOver();
    if (!FilterFilesizeOver(fFileSizeOver, filename)) {
        return false;
    }
    double fLastModifiedAfter = g_optionArgs.filter.GetLastModifiedAfter();
    if (!FilterLastModifyTime(fLastModifiedAfter, filename)) {
        return false;
    }
    g_optionArgs.countNum++;
    return true;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return OHOS::CJSystemapi::FILTER_DISMATCH;
    }

    if (g_optionArgs.countNum < g_optionArgs.listNum || g_optionArgs.listNum == 0) {
        if ((filename->d_type == DT_DIR && g_optionArgs.recursion) || FilterResult(*filename)) {
            return OHOS::CJSystemapi::FILTER_MATCH;
        }
    }
    return OHOS::CJSystemapi::FILTER_DISMATCH;
}

static int FilterFileRes(const string &path, vector<string> &dirents)
{
    unique_ptr<OHOS::CJSystemapi::FileFs::NameListArg, decltype(OHOS::CJSystemapi::FileFs::NameListArgDeleter)*>
        pNameList = {
            new (nothrow) OHOS::CJSystemapi::FileFs::NameListArg,
            OHOS::CJSystemapi::FileFs::NameListArgDeleter
        };
    if (!pNameList) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    if (num < 0) {
        LOGE("Failed to scan dir");
        return errno;
    } else {
        pNameList->direntNum = num;
        for (int i = 0; i < num; i++) {
            dirents.emplace_back(pNameList->namelist[i]->d_name);
        }
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

static int RecursiveFunc(const string &path, vector<string> &dirents)
{
    unique_ptr<OHOS::CJSystemapi::FileFs::NameListArg, decltype(OHOS::CJSystemapi::FileFs::NameListArgDeleter)*>
        pNameList = {
            new (nothrow) OHOS::CJSystemapi::FileFs::NameListArg,
            OHOS::CJSystemapi::FileFs::NameListArgDeleter
        };
    if (!pNameList) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    int num = scandir(path.c_str(), &(pNameList->namelist), FilterFunc, nullptr);
    if (num < 0) {
        LOGE("Failed to scan dir");
        return errno;
    }
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        if ((*(pNameList->namelist[i])).d_type == DT_REG) {
            dirents.emplace_back(path + '/' + pNameList->namelist[i]->d_name);
        } else if ((*(pNameList->namelist[i])).d_type == DT_DIR) {
            string pathTemp = g_optionArgs.path;
            g_optionArgs.path += '/' + string((*(pNameList->namelist[i])).d_name);
            int ret = RecursiveFunc(g_optionArgs.path, dirents);
            if (ret != OHOS::FileManagement::LibN::ERRNO_NOERR) {
                return ret;
            }
            g_optionArgs.path = pathTemp;
        }
    }
    return OHOS::FileManagement::LibN::ERRNO_NOERR;
}

}

namespace OHOS {
namespace CJSystemapi {

RetDataCArrStringN ListFileImpl::ListFile(const string& path, CListFileOptions options)
{
    RetDataCArrStringN ret = { .code = EINVAL, .data = { .head = nullptr, .size = 0 } };
    if (options.listNum < 0) {
        LOGE("Failed to get listNum prop");
        return ret;
    }
    g_optionArgs.path = path;
    g_optionArgs.listNum = options.listNum;
    g_optionArgs.recursion = options.recursion;
    auto gRet = GetFileFilterParam(options.filter, &(g_optionArgs.filter));
    if (!gRet) {
        LOGE("Failed to get filter prop.");
        g_optionArgs.Clear();
        return ret;
    }
    vector<string> direntsRes;
    int code = 0;
    code = options.recursion ? RecursiveFunc(path, direntsRes) : FilterFileRes(path, direntsRes);
    ret.code = code;
    if (code) {
        g_optionArgs.Clear();
        return ret;
    }
    if (options.recursion) {
        for (size_t i = 0; i < direntsRes.size(); i++) {
            direntsRes[i] = direntsRes[i].substr(path.length());
        }
    }
    ret.data.size = static_cast<int64_t>(direntsRes.size());
    ret.data.head = OHOS::CJSystemapi::FileFs::VectorToCArrString(direntsRes);
    g_optionArgs.Clear();
    return ret;
}

}
} // namespace OHOS::CJSystemapi