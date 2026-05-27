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

#include "dir_item_queue.h"
#include "errorcode.h"

void InitDirItemQueue(DirItemQueue *q)
{
    q->top = NULL;
    q->tail = NULL;
}

int PushDirItemQueueNode(DirItemQueue *q, const char *path)
{
    DirItemQueueNode *newNode = (DirItemQueueNode *)malloc(sizeof(DirItemQueueNode));
    if (!newNode) {
        return ARCHIVE_MEM_ERROR;
    }
    newNode->path = strdup(path);
    if (!newNode->path) {
        free(newNode);
        return ARCHIVE_MEM_ERROR;
    }
    newNode->next = NULL;

    if (q->top == NULL) {
        q->top = newNode;
        q->tail = newNode;
    } else {
        q->tail->next = newNode;
        q->tail = newNode;
    }
    return ARCHIVE_OK;
}

char *PopDirItemQueueNode(DirItemQueue *q)
{
    if (q->top == NULL) {
        return NULL;
    }
    DirItemQueueNode *temp = q->top;
    char *path = temp->path;
    q->top = q->top->next;
    if (q->top == NULL) {
        q->tail = NULL;
    }
    free(temp);
    return path;
}

int IsDirItemQueueEmpty(DirItemQueue *q)
{
    return q->top == NULL;
}

void FreeDirItemQueue(DirItemQueue *q)
{
    while (!IsDirItemQueueEmpty(q)) {
        char *path = PopDirItemQueueNode(q);
        free(path);
    }
}