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

#include <cerrno>

#include <gtest/gtest.h>

#include "swapfs_err_mapper.h"
#include "swapfs_errcode.h"

namespace OHOS::FileManagement::Swapfs {
namespace {

TEST(SwapfsErrMapperTest, MapKnownErrno)
{
    EXPECT_EQ(MapErrno(EINVAL, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_INVAL);
    EXPECT_EQ(MapErrno(EACCES, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_ACCES);
    EXPECT_EQ(MapErrno(EPERM, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_ACCES);
    EXPECT_EQ(MapErrno(ENOSPC, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_NOSPC);
    EXPECT_EQ(MapErrno(EDQUOT, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_QUOTA_EXCEEDED);
    EXPECT_EQ(MapErrno(EBUSY, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_BUSY);
    EXPECT_EQ(MapErrno(EIO, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(ENOMEM, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_NOMEM);
}

TEST(SwapfsErrMapperTest, MapMissingPathByContext)
{
    EXPECT_EQ(MapErrno(ENOENT, SwapfsErrContext::PATH_OPERATION), SWAPFS_E_PATH_UNAVAILABLE);
    EXPECT_EQ(MapErrno(ENOENT, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(MapErrno(ENOTDIR, SwapfsErrContext::PATH_OPERATION), SWAPFS_E_PATH_UNAVAILABLE);
    EXPECT_EQ(MapErrno(ENOTDIR, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_KEY_NOT_FOUND);
    EXPECT_EQ(MapErrno(ENOENT, SwapfsErrContext::IO_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(ENOTDIR, SwapfsErrContext::IO_OPERATION), SWAPFS_E_IO_ERROR);
}

TEST(SwapfsErrMapperTest, MapUnknownErrnoToIoError)
{
    EXPECT_EQ(MapErrno(123456, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(-1, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(ESTALE, SwapfsErrContext::PATH_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(123456, SwapfsErrContext::PATH_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(0, SwapfsErrContext::KEY_OPERATION), SWAPFS_E_IO_ERROR);
    EXPECT_EQ(MapErrno(ECHILD, SwapfsErrContext::IO_OPERATION), SWAPFS_E_IO_ERROR);
}

} // namespace
} // namespace OHOS::FileManagement::Swapfs
