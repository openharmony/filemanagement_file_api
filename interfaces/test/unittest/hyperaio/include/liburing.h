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

#ifndef UNITTEST_HYPERAIO_INCLUDE_LIBURING_H
#define UNITTEST_HYPERAIO_INCLUDE_LIBURING_H

namespace OHOS {
namespace HyperAio {
#define O_RDWR      02
struct io_uring_sqe {
    int32_t data;
};

struct io_uring_cqe {
    int32_t data;
    uint64_t user_data;
    int32_t res;
    uint32_t flags;
};

struct io_uring {
    int32_t data;
};

inline struct io_uring_sqe *io_uring_get_sqe(struct io_uring *ring)
{
    return new io_uring_sqe();
}

inline int io_uring_submit(struct io_uring *ring)
{
    return 1;
}

inline int io_uring_queue_init(unsigned entries, struct io_uring *ring, unsigned flags)
{
    return 1;
}

inline void io_uring_sqe_set_data(struct io_uring_sqe *sqe, void *data)
{
    return;
}

inline void io_uring_prep_openat(struct io_uring_sqe *sqe, int dfd,
    const char *path, int flags, mode_t mode)
{
    return;
}

inline void io_uring_prep_read(struct io_uring_sqe *sqe, int fd,
    void *buf, unsigned nbytes, uint64_t offset)
{
    return;
}

inline void io_uring_prep_cancel(struct io_uring_sqe *sqe,
    void *user_data, int flags)
{
    return;
}

inline int io_uring_wait_cqe(struct io_uring *ring, struct io_uring_cqe **cqe_ptr)
{
    *cqe_ptr = new io_uring_cqe;
    (*cqe_ptr)->data = 0;
    (*cqe_ptr)->user_data = 0;
    (*cqe_ptr)->res = 0;
    (*cqe_ptr)->flags = 0;
    return 1;
}

inline void io_uring_cqe_seen(struct io_uring *ring, struct io_uring_cqe *cqe)
{
    delete cqe;
    return;
}

inline void io_uring_queue_exit(struct io_uring *ring)
{
    return;
}

}
}
#endif