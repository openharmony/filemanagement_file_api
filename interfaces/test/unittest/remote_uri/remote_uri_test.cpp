/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "remote_uri.h"

namespace {
    using namespace std;
    using namespace OHOS::DistributedFS::ModuleRemoteUri;

    class RemoteUriTest : public testing::Test {
    public:
        static void SetUpTestCase(void) {};
        static void TearDownTestCase() {};
        void SetUp() {};
        void TearDown() {};
    };

    /**
     * @tc.name: Remote_uri_ConvertUri_0000
     * @tc.desc: Test function of ConvertUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_ConvertUri_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_ConvertUri_0000";
        int fd = 10;
        string remoteUri = "";
        string expectUri = "datashare:////#fdFromBinder=10";
        int ret = RemoteUri::ConvertUri(fd, remoteUri);
        EXPECT_TRUE(ret == 0);
        EXPECT_TRUE(remoteUri == expectUri);

        fd = -1;
        ret = RemoteUri::ConvertUri(fd, remoteUri);
        EXPECT_TRUE(ret == -EINVAL);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_ConvertUri_0000";
    }

    /**
     * @tc.name: Remote_uri_ConvertUri_0001
     * @tc.desc: Test function of ConvertUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_ConvertUri_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_ConvertUri_0001";
        const string fileStr = "/data/test/remote_uri_test.txt";
        int maxUriSize = 128;
        vector<string>remoteUriRecords;
        vector<int>fdRecords;
        for (int i = 0; i < maxUriSize + 1; i++) {
            int fd = open(fileStr.c_str(), O_RDWR);
            string remoteUri = "";
            int ret = RemoteUri::ConvertUri(fd, remoteUri);
            EXPECT_TRUE(ret == 0);
            remoteUriRecords.emplace_back(remoteUri);
            fdRecords.emplace_back(fd);
        }
        for (int i = 0; i < maxUriSize + 1; i++) {
            int fd = 0;
            bool ret = RemoteUri::IsRemoteUri(remoteUriRecords[i], fd);
            EXPECT_TRUE(ret == true);
            EXPECT_EQ(fd, fdRecords[i]);
        }
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_ConvertUri_0001";
    }

    /**
     * @tc.name: Remote_uri_OpenRemoteUri_0002
     * @tc.desc: Test function of OpenRemoteUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_OpenRemoteUri_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_OpenRemoteUri_0002";
        string remoteUri = "datashar:////#fdFromBinder=10";
        int ret = RemoteUri::OpenRemoteUri(remoteUri);
        EXPECT_TRUE(ret == -1);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_OpenRemoteUri_0002";
    }

    /**
     * @tc.name: Remote_uri_OpenRemoteUri_0003
     * @tc.desc: Test function of OpenRemoteUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_OpenRemoteUri_0003, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_OpenRemoteUri_0003";
        string remoteUri = "datashare:////fdFromBinder=10";
        int ret = RemoteUri::OpenRemoteUri(remoteUri);
        EXPECT_TRUE(ret == -1);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_OpenRemoteUri_0003";
    }

    /**
     * @tc.name: Remote_uri_OpenRemoteUri_0004
     * @tc.desc: Test function of OpenRemoteUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_OpenRemoteUri_0004, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_OpenRemoteUri_0004";
        string remoteUri = "datashare:////#fdFromBinde=10";
        int ret = RemoteUri::OpenRemoteUri(remoteUri);
        EXPECT_TRUE(ret == -1);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_OpenRemoteUri_0004";
    }

    /**
     * @tc.name: Remote_uri_OpenRemoteUri_0005
     * @tc.desc: Test function of OpenRemoteUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_OpenRemoteUri_0005, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_OpenRemoteUri0005";
        string remoteUri = "datashare:////#fdFromBinder=10abc";
        int ret = RemoteUri::OpenRemoteUri(remoteUri);
        EXPECT_TRUE(ret == -1);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_OpenRemoteUri_0005";
    }

    /**
     * @tc.name: Remote_uri_IsRemoteUri_006
     * @tc.desc: Test function of IsRemoteUri() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(RemoteUriTest, Remote_uri_IsRemoteUri_006, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "RemoteUriTest-begin Remote_uri_IsRemoteUri_0006";
        string remoteUri = "datashare:////#fdFromBinder=10";
        int fd = 0;
        (void)RemoteUri::IsRemoteUri(remoteUri, fd, O_RDWR);
        EXPECT_TRUE(fd == -1);
        GTEST_LOG_(INFO) << "RemoteUriTest-end Remote_uri_IsRemoteUri_0006";
    }
}
