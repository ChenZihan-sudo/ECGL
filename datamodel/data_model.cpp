//#ifdef __cplusplus
// extern "C"
//{
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../display/display_config.h"
#include "../canvas/canvas.h"
#include "../canvas/canvas_ext.h"
#include "data_model.h"

//#ifdef __cplusplus
//}
//#endif

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr NULL
#define bool short
#endif

// * Link List
LinkList_ptr newLinkListNode(LinkList_ptr linklist, void *data)
{
    LinkList_ptr ptr = (LinkList_ptr)malloc(sizeof(LinkList_t));
    ptr->data = data;
    ptr->next = linklist;
    linklist = ptr;
    return linklist;
}

// 大于为从大到小排序，小于为从小到大排
LinkList_ptr sortNewLinkListNode(LinkList_ptr linklist, void *data,
                                 bool (*compare)(void *data1, void *data2))
{
    if (linklist != NULL)
    {
        bool first = true;
        LinkList_ptr oPtr = linklist;
        LinkList_ptr cPtr = linklist;

        for (;;)
        {
            LinkList_ptr ptr = (LinkList_ptr)malloc(sizeof(LinkList_t));
            ptr->data = data;

            bool comp = compare(cPtr->data, data);

            if (first)
            {
                first = false;
                if (comp)
                {
                    ptr->next = oPtr;
                    oPtr = ptr;
                    break;
                }
            }

            if (!comp)
            {
                if (cPtr->next != NULL)
                {
                    bool compAfter = compare(cPtr->next->data, data);
                    if (!compAfter)
                    {
                        cPtr = cPtr->next;
                        continue;
                    }
                }

                LinkList_ptr cPtrNext = cPtr->next;
                ptr->next = cPtrNext;
                cPtr->next = ptr;
                break;
            }
            else if (cPtr->next != NULL)
            {
                cPtr = cPtr->next;
            }
            else
            {
                printf("Warning:End of LinkList\n");
            }
        }
        return oPtr;
    }

    return newLinkListNode(linklist, data);
}

// HINT 改进sortLinkList的时间复杂度
LinkList_ptr sortLinkList(LinkList_ptr linklist,
                          bool (*compare)(void *data1, void *data2))
{
    // Use Bubble Sort
    if (linklist != NULL)
    {
        LinkList_ptr last2Node = NULL;
        LinkList_ptr lastNode = linklist;
        LinkList_ptr currentNode = readLinkListNode(linklist);
        bool sortOver = true;

        while (1)
        {
            while (currentNode != NULL)
            {
                bool compRes = compare(lastNode->data, currentNode->data);
                // printf("RUN2 %d\n", compRes);
                if (!compRes)
                {
                    sortOver = false;

                    // Switch two nodes
                    lastNode->next = currentNode->next;
                    currentNode->next = lastNode;
                    if (lastNode == linklist)
                    {
                        // printf("TriggerThis\n");
                        linklist = currentNode;
                    }
                    else if (last2Node != NULL)
                    {
                        last2Node->next = currentNode;
                    }

                    lastNode = currentNode;
                    currentNode = currentNode->next;
                }

                last2Node = lastNode;
                lastNode = currentNode;
                currentNode = readLinkListNode(currentNode);
            }

            // printf("RUN4 %d\n", sortOver);
            if (sortOver)
                return linklist;
            last2Node = NULL;
            lastNode = linklist;
            currentNode = readLinkListNode(linklist);
            sortOver = true;
        }
    }
    return linklist;
}

LinkList_ptr deleteLinkListNode(LinkList_ptr linklist, LinkList_ptr nodePtr)
{
    LinkList_ptr ptr = linklist;

    if (linklist != NULL)
    {
        if (ptr == nodePtr)
        {
            linklist = ptr->next;
            free(nodePtr->data);
            free(nodePtr);
            return linklist;
        }

        for (;;)
        {
            if (ptr == NULL)
                break;

            if (ptr->next != NULL)
            {
                if (nodePtr == ptr->next)
                {
                    ptr->next = nodePtr->next;
                    free(nodePtr->data);
                    free(nodePtr);
                }
                ptr = ptr->next;
            }
            else
                break;
        }
    }

    return linklist;
};

LinkList_ptr readLinkListNode(const LinkList_ptr currentNode)
{
    if (currentNode != NULL)
    {
        if (currentNode->next != NULL)
            return currentNode->next;
    }

    return NULL;
}

void releaseLinkListNode(LinkList_ptr linklist)
{
    if (linklist != NULL)
    {
        LinkList_ptr ptr = NULL;
        for (;;)
        {
            ptr = linklist;

            if (linklist->next != NULL)
            {
                linklist = linklist->next;
            }
            else
                break;

            if (ptr->data != nullptr)
                free(ptr->data);
            free(ptr);
        }
    }
}

// * Stack
Stack_ptr newStack(Stack_ptr st, size_t capacity, bool allowExCapacity)
{
    st = (Stack_ptr)malloc(sizeof(Stack_t));
    st->stArr = (void **)malloc(capacity * sizeof(void *));
    st->capacity = capacity;
    st->allowExCapacity = allowExCapacity;
    st->size = 0;
    st->stFrame = 0;
    return st;
}

bool stackPush(Stack_ptr st, void *data)
{
    if (st != nullptr)
    {
        if (st->size < st->capacity)
        {
            if (st->size != 0)
                st->stFrame++;
            st->size++;
            st->stArr[st->stFrame] = data;
            return true;
        }
        else if (st->allowExCapacity)
        {
            void **reaStArr = (void **)realloc(st->stArr, (st->capacity + 1) * sizeof(void *));
            if (reaStArr != nullptr)
            {
                st->stArr = reaStArr;
                st->capacity++;
                return stackPush(st, data);
            }
        }
    }

    return false;
};

bool stackPop(Stack_ptr st)
{
    if (st != nullptr)
    {
        if (st->size > 0)
        {
            st->size--;
            if (st->stFrame > 0)
                st->stFrame--;
            return true;
        }
    }
    return false;
};

size_t stackSize(Stack_ptr st)
{
    if (st != nullptr)
        return st->size;
    return 0;
};

void *stackTop(Stack_ptr st)
{
    if (st != nullptr)
        if (st->size > 0)
            return st->stArr[st->stFrame];
    return nullptr;
};

bool stackEmpty(Stack_ptr st)
{
    if (st != nullptr)
        return st->size == 0;
    return false;
};

Stack_ptr releaseStack(Stack_ptr st)
{
    if (st != nullptr)
    {
        free(st->stArr);
        free(st);
    }

    return (st = nullptr);
};

// * Queue
// HINT: Need to specify the amount of data
Queue_ptr newQueue(Queue_ptr que, int capacity)
{
    if (que == NULL)
    {
        que = (Queue_ptr)malloc(sizeof(Queue_t));
        que->capacity = capacity + 1;
        que->maxPosi = que->capacity - 1;
        que->headPosi = 0;
        que->tailPosi = 0;
        que->size = 0;
        que->queArr = (void **)malloc(que->capacity * sizeof(void *));
    }
    return que;
}

int queueTranPosi(Queue_ptr que, int posi)
{
    if (posi < 0)
        return (que->maxPosi + 1) + (posi % -(que->maxPosi + 1));
    return posi % (que->maxPosi + 1);
}

bool queuePush(Queue_ptr que, void *data)
{
    if (!queueFull(que))
    {
        que->queArr[que->tailPosi] = data;
        que->tailPosi = queueTranPosi(que, que->tailPosi + 1);
        que->size++;
        return true;
    }
    return false;
}

bool queuePop(Queue_ptr que)
{
    if (!queueEmpty(que))
    {
        que->headPosi = queueTranPosi(que, que->headPosi + 1);
        que->size--;
        return true;
    }
    return false;
}

void *queueFront(Queue_ptr que)
{
    if (!queueEmpty(que))
        return que->queArr[que->headPosi];
    return NULL;
}

void *queueBack(Queue_ptr que)
{
    if (!queueEmpty(que))
        return que->queArr[queueTranPosi(que, que->tailPosi - 1)];
    return NULL;
}

bool queueEmpty(Queue_ptr que)
{
    if (queueTranPosi(que, que->tailPosi) == queueTranPosi(que, que->headPosi))
        return true;
    return false;
}

bool queueFull(Queue_ptr que)
{
    if (queueTranPosi(que, que->tailPosi + 1) == queueTranPosi(que, que->headPosi))
        return true;
    return false;
}

int queueSize(Queue_ptr que)
{
    return que->size;
};

Queue_ptr releaseQueue(Queue_ptr que)
{
    free(que->queArr);
    free(que);
    return (que = NULL);
}

// * QueueX
/// @brief Create queues with unlimited capacity. Usage example: que = newQueueX(que,0);
/// @param que Make sure it's an nullptr
/// @param maxCapacity Set to 0 to treat as unlimited
QueueX_ptr newQueueX(QueueX_ptr que, size_t maxCapacity)
{
    if (que == nullptr)
    {
        que = (QueueX_ptr)malloc(sizeof(QueueX));
        que->headNode = newLinkListNode(que->headNode, NULL);
        que->tailPtr = que->headNode;
        que->size = 0;
        que->maxCapacity = maxCapacity;
    }
    return que;
};

/// @brief Push data into queue
/// @param que Make sure you have called newQueueX() and it's NOT an nullptr
/// @param data Store the data you want
/// @return Return false if failed.
bool queueXPush(QueueX_ptr que, void *data)
{
    if (que != nullptr)
    {
        if (que->maxCapacity == 0 || que->maxCapacity > que->size)
        {
            LinkList_ptr newQueNode = (LinkList_ptr)malloc(sizeof(LinkList));
            newQueNode->next = nullptr;
            newQueNode->data = data;
            que->tailPtr->next = newQueNode;
            que->tailPtr = newQueNode;
            que->size++;
            return true;
        }
    }
    return false;
};

/// @brief Pop data from the queue
/// @param que Make sure you have called newQueueX() and it's NOT an nullptr
/// @return Return false if failed.
bool queueXPop(QueueX_ptr que)
{
    if (que != nullptr)
    {
        if (que->size != 0)
        {
            LinkList_ptr nextNode = que->headNode->next->next;
            free(que->headNode->next);
            que->headNode->next = nextNode;
            if (que->size == 1)
                que->tailPtr = que->headNode;
            que->size--;
            return true;
        }
    }
    return false;
};

void *queueXFront(QueueX_ptr que)
{
    if (que != nullptr)
    {
        if (que->size != 0)
        {
            return que->headNode->next->data;
        }
    }
    return nullptr;
}

void *queueXBack(QueueX_ptr que)
{
    if (que != nullptr)
    {
        if (que->size != 0)
        {
            return que->tailPtr->data;
        }
    }
    return nullptr;
}

bool queueXEmpty(QueueX_ptr que)
{
    if (que != nullptr)
    {
        return que->size == 0;
    }
    return false;
};

bool queueXFull(QueueX_ptr que)
{
    if (que != nullptr)
    {
        if (que->maxCapacity != 0)
        {
            return !(que->maxCapacity > que->size);
        }
    }
    return false;
};

size_t queueXSize(QueueX_ptr que)
{
    if (que != nullptr)
    {
        return que->size;
    }
    return 0;
};

QueueX_ptr releaseQueueX(QueueX_ptr que)
{
    if (que != nullptr)
    {
        while (queueXPop(que))
        {
        };
        free(que->headNode);
        free(que);
    }
    return (que = nullptr);
};

//* Array
Array_ptr newArray(size_t size)
{
    Array_ptr arr = nullptr;
    arr = (Array_ptr)malloc(sizeof(Array_t));
    arr->size = size;
    arr->arr = (void **)malloc(size * sizeof(void *));
    return arr;
}

void *arrData(Array_ptr arr, size_t posi)
{
    if (posi < arr->size)
        return arr->arr[posi];
    return NULL;
};

bool arrWrite(Array_ptr arr, size_t posi, void *data)
{
    size_t size = posi + 1;
    if (size > arr->size)
    {
        printf("Arr DataExtend\n");
        void **res = (void **)realloc(arr->arr, size * sizeof(void *));
        if (res != nullptr)
        {
            arr->arr = res;
            arr->size = size;
            arr->arr[posi] = data;
            return true;
        }
    }
    else
    {
        arr->arr[posi] = data;
        return true;
    }

    return false;
}

size_t arrSize(Array_ptr arr)
{
    return arr->size;
};

Array_ptr releaseArray(Array_ptr arr)
{
    releaser(arr->arr);
    return (Array_ptr)releaser(arr);
}
