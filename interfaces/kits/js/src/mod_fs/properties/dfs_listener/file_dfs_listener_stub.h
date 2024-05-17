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

#ifndef FILEMANAGEMENT_FILE_API_FILE_DFS_LISTENER_STUB_H
#define FILEMANAGEMENT_FILE_API_FILE_DFS_LISTENER_STUB_H

#include "i_file_dfs_listener.h"
#include "iremote_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"


namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
class FileDfsListenerStub : public IRemoteStub<Storage::DistributedFile::IFileDfsListener> {
public:
    FileDfsListenerStub();
    virtual ~FileDfsListenerStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using FileDfsListenerInterface = int32_t (FileDfsListenerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, FileDfsListenerInterface> opToInterfaceMap_;

    int32_t HandleOnStatus(MessageParcel &data, MessageParcel &reply);
};

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS

#endif // FILEMANAGEMENT_FILE_API_FILE_DFS_LISTENER_STUB_H