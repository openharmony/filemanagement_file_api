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

#include "read_lines_ani.h"

#include "ani_helper.h"
#include "error_handler.h"
#include "filemgmt_libhilog.h"
#include "read_lines_core.h"
#include "reader_iterator_ani.h"
#include "type_converter.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace ANI {
using namespace OHOS::FileManagement::ModuleFileIO;

static tuple<bool, optional<Options>> ToReadLinesOptions(ani_env *env, ani_object obj)
{
    Options options;

    ani_boolean isUndefined;
    env->Reference_IsUndefined(obj, &isUndefined);
    if (isUndefined) {
        return { true, nullopt };
    }

    auto [succEncoding, encoding] = AniHelper::ParseEncoding(env, obj);
    if (!succEncoding) {
        HILOGE("Illegal option.encoding parameter");
        return { false, nullopt };
    }
    options.encoding = encoding.value();

    return { true, make_optional<Options>(move(options)) };
}

ani_object ReadLinesAni::ReadLinesSync(
    ani_env *env, [[maybe_unused]] ani_class clazz, ani_string path, ani_object options)
{
    auto [succPath, filePath] = TypeConverter::ToUTF8String(env, path);
    if (!succPath) {
        HILOGE("Invalid path from ETS first argument");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto [succMode, opt] = ToReadLinesOptions(env, options);
    if (!succMode) {
        HILOGE("Invalid options");
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    FsResult<FsReaderIterator *> ret = ReadLinesCore::DoReadLines(filePath, opt);
    if (!ret.IsSuccess()) {
        HILOGE("Readlines failed");
        const auto &err = ret.GetError();
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    const FsReaderIterator *readerIterator = ret.GetData().value();
    auto result = ReaderIteratorAni::Wrap(env, move(readerIterator));
    if (result == nullptr) {
        delete readerIterator;
        readerIterator = nullptr;
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return nullptr;
    }
    return result;
}

} // namespace ANI
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS