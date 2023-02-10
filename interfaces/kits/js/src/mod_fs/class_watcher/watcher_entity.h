/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_ENTITY_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_ENTITY_H
#include <vector>
#include <string>
#include <memory>

#include "filemgmt_libn.h"
namespace OHOS::FileManagement::ModuleFileIO {

struct WatcherInfoArg {
    std::string filename;
    uint32_t events;
    int fd;
    int wd;
    napi_env env = nullptr;
    LibN::NRef nRef;

    explicit WatcherInfoArg(LibN::NVal jsVal) : nRef(jsVal) {}
    ~WatcherInfoArg() = default;
};

struct WatcherEntity {
    std::unique_ptr<WatcherInfoArg> data_;
};
} // namespace OHOS::FileManagement::ModuleFileIO namespace OHOS
#endif