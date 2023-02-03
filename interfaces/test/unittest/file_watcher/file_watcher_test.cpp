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

#include "file_watcher.h"
#include <cerrno>
#include <fcntl.h>
#include <thread>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
namespace {
    using namespace std;
    using namespace OHOS::DistributedFS::ModuleFileIO;

    class FileWatcherTest : public testing::Test {
    public:
        static void SetUpTestCase(void){};
        static void TearDownTestCase(){};
        void SetUp(){};
        void TearDown(){};
    };


    void WatcherCallback(napi_env env, napi_ref callback, const std::string &filename, const uint32_t &event)
    {
        string path = "/data/data/test2";
        EXPECT_STREQ(path.c_str(), filename.c_str());
        if (event == IN_CLOSE || event == IN_DELETE) {
            EXPECT_TRUE(true);
        } else {
            EXPECT_TRUE(false);
        }
    }

    void GetEvent(shared_ptr<FileWatcher> &fileWatcherPtr, std::shared_ptr<WatcherInfoArg> &arg)
    {
        fileWatcherPtr->GetNotifyEvent(arg, WatcherCallback);
    }

    /**
     * @tc.name: Watcher_InitNotify_0000
     * @tc.desc: Test function of InitNotify() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_InitNotify_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_InitNotify_0000";
        int fd = 0;
        int error = 0;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        bool ret = fileWatcherPtr->InitNotify(fd, error);

        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(error == 0);
        EXPECT_TRUE(ret == true);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_InitNotify_0000";
    }

    /**
     * @tc.name: Watcher_StartNotify_0000
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0000";
        int error = 0;
        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        bool ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0000";
    }

    /**
     * @tc.name: Watcher_StartNotify_0001
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0001, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0001";
        int error = 0;
        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = 10;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        bool ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0001";
    }

    /**
     * @tc.name: Watcher_StartNotify_0002
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0002, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0002";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = "";

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0002";
    }

    /**
     * @tc.name: Watcher_StartNotify_0003
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0003, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0003";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = "/data/data";

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0003";
    }

    /**
     * @tc.name: Watcher_StartNotify_0004
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0004, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0004";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::vector<uint32_t> events;
        events.push_back(0);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = "/data/data";
        arg->events = events;

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0004";
    }

    /**
     * @tc.name: Watcher_StartNotify_0005
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0005, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0005";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::vector<uint32_t> events;
        events.push_back(IN_ACCESS);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = "/data/test";
        arg->events = events;

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0005";
    }

    /**
     * @tc.name: Watcher_StartNotify_0006
     * @tc.desc: Test function of StartNotify() interface for FAIL.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0006, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0006";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::vector<uint32_t> events;
        events.push_back(IN_ACCESS);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = "/data/test";
        arg->events = events;

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == false);
        EXPECT_TRUE(error > 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0006";
    }

    /**
     * @tc.name: Watcher_StartNotify_0007
     * @tc.desc: Test function of StartNotify() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_StartNotify_0007, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_StartNotify_0007";
        int fd = 0;
        int error = 0;
        bool ret = false;
        string filename = "/data/data/test";
        int fileFd = open(filename.c_str(), O_CREAT);
        EXPECT_TRUE(fileFd > 0);

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        std::vector<uint32_t> events;
        events.push_back(IN_ACCESS);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = filename;
        arg->events = events;

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(ret == true);
        EXPECT_TRUE(error == 0);

        ret = fileWatcherPtr->StopNotify(arg, error);
        EXPECT_TRUE(ret == true);
        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_StartNotify_0007";
    }

    /**
     * @tc.name: Watcher_GetNotifyEvent_0000
     * @tc.desc: Test function of GetNotifyEvent() interface for SUCCESS.
     * @tc.size: MEDIUM
     * @tc.type: FUNC
     * @tc.level Level 1
     * @tc.require: AR000HG8M4
     */
    HWTEST_F(FileWatcherTest, Watcher_GetNotifyEvent_0000, testing::ext::TestSize.Level1)
    {
        GTEST_LOG_(INFO) << "FileWatcherTest-begin Watcher_GetNotifyEvent_0000";
        int fd = 0;
        int error = 0;
        bool ret = false;

        shared_ptr<FileWatcher> fileWatcherPtr = make_shared<FileWatcher>();
        ret = fileWatcherPtr->InitNotify(fd, error);
        EXPECT_TRUE(fd > 0);
        EXPECT_TRUE(ret == true);

        string filename = "/data/data/test2";
        int fileFd = open(filename.c_str(), O_CREAT);
        EXPECT_TRUE(fileFd > 0);

        std::vector<uint32_t> events;
        events.push_back(IN_CLOSE);
        events.push_back(IN_DELETE);

        std::shared_ptr<WatcherInfoArg> arg = make_shared<WatcherInfoArg>();
        arg->fd = fd;
        arg->filename = filename;
        arg->events = events;

        ret = fileWatcherPtr->StartNotify(arg, error);
        EXPECT_TRUE(error == 0);
        EXPECT_TRUE(ret == true);
        
        thread t(GetEvent, ref(fileWatcherPtr), ref(arg));
        t.detach();

        this_thread::sleep_for(chrono::seconds(2));
        int clsoeRet = close(fileFd);
        EXPECT_TRUE(clsoeRet == 0);

        this_thread::sleep_for(chrono::seconds(2));
        int removeRet = remove(filename.c_str());
        EXPECT_TRUE(removeRet == 0);

        GTEST_LOG_(INFO) << "FileWatcherTest-end Watcher_GetNotifyEvent_0000";
    }

} // namespace