#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "canvas_shader.h"
#include "canvas.h"

// * Iterators

// * PathInfo Iterator Structure
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
                    LinkList_ptr nextll = hdl->pathInfo[itor->val1];
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

// HINT 可优化的: 去除存储y的值

static PRIORITY_id priority = 0;

//* Shader Container
ShaderContainer_ptr newShaderContainer(int x, int y, int containerType, void *data, PRIORITY_id priority)
{
    ShaderContainer_ptr scon = (ShaderContainer_ptr)malloc(sizeof(ShaderContainer_t));
    scon->x = x;
    scon->y = y;
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
ShaderContainer_ptr newSPointRGBA32(int x, int y, bool keyPoint, uint8_t alpha, PRIORITY_id priority)
{
    sPointRGBA32_ptr pt = nullptr;
    pt = (sPointRGBA32_ptr)malloc(sizeof(sPointRGBA32_t));
    pt->keyPoint = keyPoint;
    pt->alpha = alpha;
    return newShaderContainer(x, y, SPOINT_RGBA32, (void *)pt, priority);
}

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, int y)
{
    return newShaderContainer(x, y, SPOINT_RGB888, nullptr, priority);
}

// //* Line
// ShaderContainer_ptr newSLine(int x0, int y0, int x1, int y1, bool antialiasing, PRIORITY_id priority)
// {
//     sLine_ptr pt = nullptr;
//     pt = (sLine_ptr)malloc(sizeof(sLine_t));
//     pt->x1 = x1;
//     pt->y1 = y1;
//     pt->antialiasing = antialiasing;
//     return newShaderContainer(x0, y0, SLINE, (void *)pt, priority);
// }