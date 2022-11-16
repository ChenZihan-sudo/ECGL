#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "canvas_shader.h"
#include "canvas.h"

// * Iterators

// HINT: PathInfo Iterator Structure
// void *data1; -> store the CanvaHandle_ptr data
// void *data2; -> current shaderInfo data pointer, store the LinkList_ptr data
// int val1; -> store the current line posi y
// int val2; -> the end flag of iterator

Iterator_ptr newShaderInfoIterator(CanvaHandle_ptr hdl)
{
    Iterator_ptr itor = nullptr;
    itor = (Iterator_ptr)malloc(sizeof(Iterator_t));
    itor->data1 = (void *)hdl;
    itor->data2 = nullptr;
    itor->val1 = hdl->scanLineMin;
    itor->val2 = 0;
    itor->iteratorType = SHADER_INFO;
    return itor;
}

ShaderContainer_ptr nextShaderInfo(Iterator_ptr itor)
{
    ShaderContainer_ptr scon = nullptr;
    if (itor != nullptr)
    {
        if (itor->iteratorType == SHADER_INFO)
        {
            if (itor->val2 == 0)
            {
                CanvaHandle_ptr hdl = (CanvaHandle_ptr)itor->data1;
                LinkList_ptr info = nullptr;

                if (itor->data2 == nullptr)
                {
                    LinkList_ptr nextll = hdl->shaderInfo[itor->val1];
                    itor->data2 = (void *)nextll;
                    info = nextll;
                }
                else
                    info = (LinkList_ptr)itor->data2;

                if (info != nullptr)
                {
                    scon = (ShaderContainer_ptr)info->data;

                    if (info->next != nullptr)
                        itor->data2 = (void *)info->next;
                    else
                    {
                        itor->data2 = nullptr;
                        itor->val1++;
                    }
                }
                else
                    itor->val1++;

                int min = hdl->scanLineMin;
                int max = hdl->scanLineMax;
                if (!(min <= itor->val1 && itor->val1 <= max))
                    itor->val2 = 1;
            }
        }
    }
    return scon;
}

int currentShaderInfoItorY(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val1;
    return 0;
};

bool shaderInfoIterateEnd(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val2;
    return false;
}

//* priority and it's container

static PRIORITY_id priority = 0;

SPriorityCon_ptr newSPriorityCon(int x, PRIORITY_id id)
{
    SPriorityCon_ptr spcon = (SPriorityCon_ptr)malloc(sizeof(SPriorityCon_t));
    spcon->x = x;
    spcon->id = id;
    return spcon;
};

//* Shader Container
ShaderContainer_ptr newShaderContainer(int x, int containerType, void *data, PRIORITY_id priority)
{
    ShaderContainer_ptr scon = (ShaderContainer_ptr)malloc(sizeof(ShaderContainer_t));
    scon->x = x;
    scon->TYPE = containerType;
    scon->data = data;
    scon->priority = priority;
    return scon;
}
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt)
{
    releaser(pt->data);
    return (ShaderContainer_ptr)releaser(pt);
};

//* Point RGBA32
ShaderContainer_ptr newSPointRGBA32(int x, bool keyPoint, uint8_t alpha, PRIORITY_id priority)
{
    sPointRGBA32_ptr pt = nullptr;
    pt = (sPointRGBA32_ptr)malloc(sizeof(sPointRGBA32_t));
    pt->keyPoint = keyPoint;
    pt->alpha = alpha;
    return newShaderContainer(x, SPOINT_RGBA32, (void *)pt, priority);
}

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x)
{
    return newShaderContainer(x, SPOINT_RGB888, nullptr, priority);
}

bool shaderArrInsert(Array_ptr a, SPriorityCon_ptr spcon)
{
    if (a != nullptr)
    {
        void **ary = a->arr;

        int left = 0;
        int right = a->size - 1;
        int mid = (left + right) / 2;
        int ins = spcon->x;
        int insPosi = -1;

        if (ins < ((SPriorityCon_ptr)ary[left])->x)
            return -1;
        else if (ins > ((SPriorityCon_ptr)ary[right])->x)
            return right + 1;
        else
            while (1)
            {
                if (ins < ((SPriorityCon_ptr)ary[mid])->x)
                    right = mid;
                else
                    left = mid;
                mid = (left + right) / 2;

                if (right - left == 1)
                {
                    insPosi = right;
                    break;
                }
            }

        if (!arrWrite(a, a->size, nullptr))
            return false;
        if (insPosi == a->size - 1)
            ary[insPosi] = (void *)spcon;
        else
        {
            insPosi++;
            void *buf = ary[insPosi];
            ary[insPosi] = (void *)spcon;
            for (size_t i = insPosi + 1; i < a->size; i++)
            {
                void *abuf = ary[i];
                ary[i] = buf;
                buf = abuf;
            }
        }
        return true;
    }
    return false;
}

int shaderArrSearch(Array_ptr a, int x)
{
    // Use Binary Search
    if (a != nullptr)
    {
        void **Arr = a->arr;
        int left = 0;
        int right = a->size - 1;
        int mid = (left + right) / 2;

        while (1)
        {
            SPriorityCon_ptr spcon = (SPriorityCon_ptr)Arr[mid];

            if (x == spcon->x)
                return mid;

            if (x > spcon->x)
                left = mid + 1;
            else
                right = mid - 1;

            if (left > right)
                return -1;

            mid = (left + right) / 2;
        }
    }

    return -1;
}

bool shaderPointCompare(void *data1, void *data2)
{
    ShaderContainer_ptr scon1 = (ShaderContainer_ptr)data1;
    ShaderContainer_ptr scon2 = (ShaderContainer_ptr)data2;
    return scon1->x < scon2->x;
}

void writeSPoint(CanvaHandle_ptr hd, int x, int y)
{
    LinkList_ptr *shaderInfo = hd->shaderInfo;
    Array_ptr *shaderArr = hd->shaderArr;

    // Create rgb888 point shader container
    ShaderContainer_ptr scon = newSPointRGB888(x);
    sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);

    // Sync save priority data into array
    if (shaderArr[y] == nullptr)
        shaderArr[y] = newArray(1);

    int posi = shaderArrSearch(shaderArr[y], x);
    if (posi != -1)
        // Modify data due to priority
        ((SPriorityCon_ptr)shaderArr[posi])->id = priority;
    else
    {
        // Insert new data
        SPriorityCon_ptr spcon = newSPriorityCon(x, priority);
        shaderArrInsert(shaderArr[y], spcon);
    }
}

void writeSPointA(CanvaHandle_ptr hd, int x, int y, uint8_t alpha, bool keyPoint)
{
    LinkList_ptr *shaderInfo = hd->shaderInfo;
    Array_ptr *shaderArr = hd->shaderArr;

    // Create rgba32 point shader container
    ShaderContainer_ptr scon = newSPointRGBA32(x, keyPoint, alpha, priority);
    sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);

    // Sync save priority data into array
    if (shaderArr[y] == nullptr)
        shaderArr[y] = newArray(1);

    int posi = shaderArrSearch(shaderArr[y], x);
    if (posi != -1)
        // Modify data due to priority
        ((SPriorityCon_ptr)shaderArr[posi])->id = priority;
    else
    {
        // Insert new data
        SPriorityCon_ptr spcon = newSPriorityCon(x, priority);
        shaderArrInsert(shaderArr[y], spcon);
    }
}