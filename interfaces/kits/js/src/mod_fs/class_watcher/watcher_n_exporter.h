/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_WATCHER_WATCHER_N_EXPORTER_H
#include <memory>

#include "filemgmt_libn.h"
#include "watcher_entity.h"
namespace OHOS::FileManagement::ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
class WatcherNExporter final : public NExporter {
public:
    class JSCallbackContext {
    public:
        explicit JSCallbackContext(NRef &ref) : ref_(ref) {}
        ~JSCallbackContext() {}

    public:
        napi_env env_;
        NRef &ref_;
        std::string fileName_;
        uint32_t event_ = 0;
        uint32_t cookie_ = 0;
    };

    inline static const std::string className_ = "Watcher";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static void WatcherCallback(napi_env env, NRef &callback, const std::string &fileName, const uint32_t &event,
                                const uint32_t &cookie);

    WatcherNExporter(napi_env env, napi_value exports);
    ~WatcherNExporter() override;

private:
};
} // namespace OHOS::FileManagement::ModuleFileIO
#endif
