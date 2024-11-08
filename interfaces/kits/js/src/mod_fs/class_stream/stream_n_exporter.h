/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_STREAM_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_STREAM_N_EXPORTER_H

#include "filemgmt_libn.h"

#include <mutex>
#include "stream_entity.h"
namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
class StreamNExporter final : public NExporter {
public:
    static std::mutex mutex;

    inline static const std::string className_ = "FsStream";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info cbInfo);

    static napi_value WriteSync(napi_env env, napi_callback_info cbInfo);
    static napi_value ReadSync(napi_env env, napi_callback_info cbInfo);
    static napi_value CloseSync(napi_env env, napi_callback_info cbInfo);
    static napi_value FlushSync(napi_env env, napi_callback_info cbInfo);

    static napi_value Write(napi_env env, napi_callback_info cbInfo);
    static napi_value Read(napi_env env, napi_callback_info cbInfo);
    static napi_value Close(napi_env env, napi_callback_info cbInfo);
    static napi_value Seek(napi_env env, napi_callback_info cbInfo);
    static napi_value Flush(napi_env env, napi_callback_info cbInfo);

    static std::shared_ptr<FILE> GetFilePtr(StreamEntity *streamEntity);
    static StreamEntity *GetEntityOf(napi_env env, NFuncArg &funcArg);

    StreamNExporter(napi_env env, napi_value exports);
    ~StreamNExporter() override;
};

struct AsyncReadArg {
    size_t lenRead = 0;
    NRef refReadBuf;

    explicit AsyncReadArg(NVal jsReadBuf) : refReadBuf(jsReadBuf) {}
    ~AsyncReadArg() = default;
};

struct AsyncWriteArg {
    NRef refWriteArrayBuf;
    std::unique_ptr<char[]> guardWriteStr = nullptr;
    size_t actLen = 0;

    explicit AsyncWriteArg(NVal refWriteArrayBuf) : refWriteArrayBuf(refWriteArrayBuf) {}
    explicit AsyncWriteArg(std::unique_ptr<char[]> &&guardWriteStr) : guardWriteStr(std::move(guardWriteStr)) {}
    ~AsyncWriteArg() = default;
};

const std::string PROCEDURE_STREAM_WRITE_NAME = "FileIOStreamWrite";
const std::string PROCEDURE_STREAM_READ_NAME = "FileIOStreamRead";
const std::string PROCEDURE_STREAM_CLOSE_NAME = "FileIOStreamClose";
const std::string PROCEDURE_STREAM_FLUSH_NAME = "FileIOStreamFlush";

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_CLASS_STREAM_STREAM_N_EXPORTER_H