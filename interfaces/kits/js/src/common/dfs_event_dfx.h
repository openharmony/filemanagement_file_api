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

#ifndef DISTRIBUTEDFILE_DFS_EVENT_DFX_H
#define DISTRIBUTEDFILE_DFS_EVENT_DFX_H

#include "hisysevent.h"
namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
namespace RadarReporter {
using namespace OHOS::HiviewDFX;
static constexpr int DIST_DATA_MGR_SYS_ID = 0xd;
static constexpr int DFS_ID = 13;
enum BizScene : std::int32_t {
    DFX_SET_DFS = 1,
};

enum BizStageSetDfs : std::int32_t {
    DFX_SET_BIZ_SCENE = 1,
};

enum StageRes : std::int32_t {
    DFX_IDLE = 0,
    DFX_SUCCESS = 1,
    DFX_FAILED = 2,
    DFX_CANCELLED = 3,
};

enum BizState : std::int32_t {
    DFX_BEGIN = 1,
    DFX_END = 2,
};

enum ErrorCode : std::int32_t {
    DFS_ERROR = (DIST_DATA_MGR_SYS_ID << 21) | (DFS_ID << 16),
    PREPARE_COPY_SESSION_ERROR,
    COPY_TO_SANDBOX_ERROR,
    SEND_FILE_ERROR,
};

static constexpr char DOMAIN[] = "DISTDATAMGR";
constexpr const char* EVENT_NAME = "DISTRIBUTED_DFS_BEHAVIOR";
constexpr const char* ORG_PKG = "distributedfile";
constexpr const char* BIZ_STATE = "BIZ_STATE";
constexpr const char* ERROR_CODE = "ERROR_CODE";
constexpr const char* PACKAGE_NAME = "PACKAGE_NAME";
constexpr const char* CONCURRENT_ID = "CONCURRENT_ID";
static constexpr HiviewDFX::HiSysEvent::EventType TYPE = HiviewDFX::HiSysEvent::EventType::BEHAVIOR;

#define RADAR_REPORT(bizScene, bizStage, stageRes, ...)                                    \
({                                                                                         \
    HiSysEventWrite(RadarReporter::DOMAIN, RadarReporter::EVENT_NAME, RadarReporter::TYPE, \
        "ORG_PKG", RadarReporter::ORG_PKG, "FUNC", __FUNCTION__,                           \
        "BIZ_SCENE", bizScene, "BIZ_STAGE", bizStage, "STAGE_RES", stageRes,               \
        ##__VA_ARGS__);                                                                    \
})
} // namespace RadarReporter
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif //DISTRIBUTEDDATAMGR_DFS_EVENT_DFX_H