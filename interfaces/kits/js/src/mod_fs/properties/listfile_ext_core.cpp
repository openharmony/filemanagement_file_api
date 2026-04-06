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

#include "listfile_ext_core.h"

#include <memory>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <thread>
#include <tuple>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "i_file_filter.h"

namespace OHOS::FileManagement::ModuleFileIO {

static thread_local OptionArgs g_optionArgs;

static bool ValidOptionParam(const string &path, const optional<ListFileExtOptions> &options, OptionArgs &optionArgs)
{
    g_optionArgs.Clear();
    g_optionArgs.path = path;
    g_optionArgs.originalPath = path;

    if (options.has_value()) {
        auto op = options.value();
        if (op.listNum.has_value()) {
            auto listNum = op.listNum.value();
            if (listNum < 0) {
                HILOGE("Failed to get listNum prop");
                return false;
            }
            optionArgs.listNum = listNum;
        }

        optionArgs.recursion = op.recursion;

        if (op.fileFilter != nullptr) {
            optionArgs.fileFilter = op.fileFilter;
        }
    }

    return true;
}

static bool FilterResult(const struct dirent &filename)
{
    if (g_optionArgs.fileFilter) {
        std::string filterName;
        if (g_optionArgs.recursion) {
            if (g_optionArgs.path == g_optionArgs.originalPath) {
                filterName = "/" + std::string(filename.d_name);
            } else {
                filterName = g_optionArgs.path.substr(g_optionArgs.originalPath.length()) + '/' + filename.d_name;
            }
        } else {
            filterName = filename.d_name;
        }
        auto matched = g_optionArgs.fileFilter->Filter(filterName);
        if (!matched) {
            return false;
        }
    }
    g_optionArgs.countNum++;
    return true;
}

static int32_t FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return FILTER_DISMATCH;
    }

    if (g_optionArgs.countNum < g_optionArgs.listNum || g_optionArgs.listNum == 0) {
        if ((filename->d_type == DT_DIR && g_optionArgs.recursion) || FilterResult(*filename)) {
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
            string pathTemp = g_optionArgs.path;
            g_optionArgs.path += '/' + string((*(pNameList->namelist[i])).d_name);
            int ret = RecursiveFunc(g_optionArgs.path, dirents);
            if (ret != ERRNO_NOERR) {
                return ret;
            }
            g_optionArgs.path = pathTemp;
        }
    }
    return ERRNO_NOERR;
}

static void DoListFileExtVector(const string &path, vector<string> &dirents, bool recursion)
{
    if (recursion) {
        for (size_t i = 0; i < dirents.size(); i++) {
            dirents[i] = dirents[i].substr(path.length());
        }
    }
}

FsResult<std::vector<std::string>> ListFileExtCore::DoListFileExt(
    const string &path, const std::optional<ListFileExtOptions> &options)
{
    if (!ValidOptionParam(path, options, g_optionArgs)) {
        HILOGE("Invalid options");
        return FsResult<std::vector<std::string>>::Error(EINVAL);
    }

    vector<string> direntsRes;
    int ret = 0;
    ret = g_optionArgs.recursion ? RecursiveFunc(path, direntsRes) : FilterFileRes(path, direntsRes);
    if (ret) {
        return FsResult<std::vector<std::string>>::Error(ret);
    }
    DoListFileExtVector(path, direntsRes, g_optionArgs.recursion);
    g_optionArgs.Clear();

    return FsResult<std::vector<std::string>>::Success(direntsRes);
}

} // namespace OHOS::FileManagement::ModuleFileIO
