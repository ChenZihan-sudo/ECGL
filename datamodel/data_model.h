#ifndef DATA_MODEL_H
#define DATA_MODEL_H

// * Link List
typedef struct LinkList LinkList_t;
typedef struct LinkList *LinkList_ptr;
struct LinkList
{
    void *data;
    LinkList_ptr next;
};
LinkList_ptr newLinkListNode(LinkList_ptr linklist, void *data);
LinkList_ptr sortNewLinkListNode(LinkList_ptr linklist, void *data,
                                 bool (*compare)(void *data1, void *data2));
LinkList_ptr sortLinkList(LinkList_ptr linklist,
                          bool (*compare)(void *data1, void *data2));
LinkList_ptr deleteLinkListNode(LinkList_ptr linklist, LinkList_ptr nodePtr);
LinkList_ptr readLinkListNode(const LinkList_ptr currentNode);
void releaseLinkListNode(LinkList_ptr linklist);

// * Stack
typedef struct Stack Stack_t;
typedef struct Stack *Stack_ptr;
struct Stack
{
    size_t capacity;
    size_t size;
    size_t stFrame;
    void **stArr;
    bool allowExCapacity;
};
Stack_ptr newStack(Stack_ptr st, size_t capacity, bool allowExCapacity);
bool stackPush(Stack_ptr st, void *data);
bool stackPop(Stack_ptr st);
size_t stackSize(Stack_ptr st);
void *stackTop(Stack_ptr st);
bool stackEmpty(Stack_ptr st);
Stack_ptr releaseStack(Stack_ptr st);

// HINT: This structure not finshed yet.
#if 0
// * StackX
typedef struct StackX StackX_t;
typedef struct StackX *StackX_ptr;
struct StackX
{
    LinkList_ptr headNode;
    size_t maxCapacity; // HINT: Set to 0 will regard as no limit
    size_t size;
};
StackX_ptr newStackX(StackX_ptr st);
bool stackXPush(StackX_ptr st, void *data);
bool stackXPop(StackX_ptr st);
size_t stackXSize(StackX_ptr st);
void *stackXTop(StackX_ptr st);
bool stackXEmpty(StackX_ptr st);
StackX_ptr releaseStackX(StackX_ptr st);
#endif

// * Queue
typedef struct Queue Queue_t;
typedef struct Queue *Queue_ptr;
struct Queue
{
    int headPosi;
    int tailPosi;
    int maxPosi;
    int capacity;
    int size;
    void **queArr;
};
Queue_ptr newQueue(Queue_ptr que, int capacity);
int queueTranPosi(Queue_ptr que, int posi);
bool queuePush(Queue_ptr que, void *data);
bool queuePop(Queue_ptr que);
void *queueFront(Queue_ptr que);
void *queueBack(Queue_ptr que);
bool queueEmpty(Queue_ptr que);
bool queueFull(Queue_ptr que);
int queueSize(Queue_ptr que);
Queue_ptr releaseQueue(Queue_ptr queue);

// * QueueX
typedef struct QueueX QueueX_t;
typedef struct QueueX *QueueX_ptr;
struct QueueX
{
    LinkList_ptr headNode;
    LinkList_ptr tailPtr;
    size_t maxCapacity; // HINT: Set to 0 will regard as no limit
    size_t size;
};
QueueX_ptr newQueueX(QueueX_ptr que);
bool queueXPush(QueueX_ptr que, void *data);
bool queueXPop(QueueX_ptr que);
void *queueXFront(QueueX_ptr que);
void *queueXBack(QueueX_ptr que);
bool queueXEmpty(QueueX_ptr que);
bool queueXFull(QueueX_ptr que);
size_t queueXSize(QueueX_ptr que);
QueueX_ptr releaseQueueX(QueueX_ptr queue);

// * Array
typedef struct Array Array_t;
typedef struct Array *Array_ptr;
struct Array
{
    size_t size;
    size_t maxCapacity; // HINT: Set to 0 will regard as no limit
    void **arr;
};
Array_ptr newArray(size_t size);
void *arrData(Array_ptr arr, size_t posi);
void *arrWrite(Array_ptr arr, size_t posi, void *value);
bool arrFull(Array_ptr arr);
size_t arrSize(Array_ptr arr);

#endif
