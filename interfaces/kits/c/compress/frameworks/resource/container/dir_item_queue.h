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

#ifndef HISPEED_DIR_ITEM_QUEUE_H
#define HISPEED_DIR_ITEM_QUEUE_H

#include <stdlib.h>
#include <string.h>

typedef struct DirItemQueueNode {
    char *path;
    struct DirItemQueueNode *next;
} DirItemQueueNode;

typedef struct {
    DirItemQueueNode *top;
    DirItemQueueNode *tail;
} DirItemQueue;

void InitDirItemQueue(DirItemQueue *q);

int PushDirItemQueueNode(DirItemQueue *q, const char *path);

char *PopDirItemQueueNode(DirItemQueue *q);

int IsDirItemQueueEmpty(DirItemQueue *q);

void FreeDirItemQueue(DirItemQueue *q);

#endif //HISPEED_DIR_ITEM_QUEUE_H