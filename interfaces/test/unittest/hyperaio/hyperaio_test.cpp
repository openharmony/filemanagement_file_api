/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <cerrno>
#include <string>
#include <vector>

#include "hyperaio.h"

namespace {
    using namespace std;
    using namespace OHOS::HyperAio;
    class HyperAioTest : public testing::Test {
    public:
        static void SetUpTestCase(void) {};
        static void TearDownTestCase() {};
        void SetUp() {};
        void TearDown() {};
    };
#ifdef HYPERAIO_USE_LIBURING
    const uint64_t userData = 12345;
    const uint32_t len = 1024;
    std::function<void(std::unique_ptr<IoResponse>)> callBack = [](std::unique_ptr<IoResponse> response) {
        GTEST_LOG_(INFO) << "HyperAioTest callBack";
    };

    /**
     * @tc.name: HyperAio_StartOpenReqs_0000
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(nullptr);
        EXPECT_EQ(result, -EINVAL);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartOpenReqs_0001
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        OpenInfo openInfo = {0, O_RDWR, 0, nullptr, userData};
        OpenReqs openReqs = {1, &openInfo};
        result = hyperAio_->StartOpenReqs(&openReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartOpenReqs_0002
     * @tc.desc: Test function of StartOpenReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartOpenReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartOpenReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        OpenInfo openInfo = {0, O_RDWR, 0, nullptr, userData};
        OpenReqs openReqs = {1, &openInfo};
        result = hyperAio_->StartOpenReqs(&openReqs);
        EXPECT_EQ(result, -EPERM);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartOpenReqs_0002";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0000
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(nullptr);
        EXPECT_EQ(result, -EINVAL);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0001
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        ReadInfo readInfo = {0, len, 0, nullptr, userData};
        ReadReqs readReqs = {1, &readInfo};
        result = hyperAio_->StartReadReqs(&readReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartReadReqs_0002
     * @tc.desc: Test function of StartReadReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartReadReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartReadReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        ReadInfo readInfo = {0, len, 0, nullptr, userData};
        ReadReqs readReqs = {1, &readInfo};
        result = hyperAio_->StartReadReqs(&readReqs);
        EXPECT_EQ(result, -EPERM);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartReadReqs_0002";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0000
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0000";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(nullptr);
        EXPECT_EQ(result, -EINVAL);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0000";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0001
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0001";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        result = hyperAio_->CtxInit(&callBack);
        EXPECT_EQ(result, 0);
        CancelInfo cancelInfo = {userData, 0};
        CancelReqs cancelReqs = {1, &cancelInfo};
        result = hyperAio_->StartCancelReqs(&cancelReqs);
        EXPECT_EQ(result, 0);
        result = hyperAio_->DestroyCtx();
        EXPECT_EQ(result, 0);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0001";
    }

    /**
     * @tc.name: HyperAio_StartCancelReqs_0002
     * @tc.desc: Test function of StartCancelReqs() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(HyperAioTest, HyperAio_StartCancelReqs_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "HyperAioTest-begin HyperAio_StartCancelReqs_0002";
        std::unique_ptr<HyperAio> hyperAio_ = std::make_unique<HyperAio>();
        int32_t result = hyperAio_->SupportIouring();
        EXPECT_EQ((result & HYPERAIO_APP_PERMISSION) == 0, true);
        if ((result & HYPERAIO_APP_PERMISSION) == 0) {
            return;
        }
        CancelInfo cancelInfo = {userData, 0};
        CancelReqs cancelReqs = {1, &cancelInfo};
        result = hyperAio_->StartCancelReqs(&cancelReqs);
        EXPECT_EQ(result, -EPERM);
        GTEST_LOG_(INFO) << "HyperAioTest-end HyperAio_StartCancelReqs_0002";
    }
#endif
}
