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
// int val1; -> store the current line y posi
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

// In each loop, use this turn to next Y row
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

// Give current Y row. Use this before call nextShaderInfo().
int currentShaderInfoItorY(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val1;
    return 0;
};

// Boolean giving whether the iteration is over
bool shaderInfoIterateEnd(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val2;
    return false;
}

//* Shader Container
ShaderContainer_ptr newShaderContainer(int x, int containerType, void *data)
{
    ShaderContainer_ptr scon = (ShaderContainer_ptr)malloc(sizeof(ShaderContainer_t));
    scon->x = x;
    scon->TYPE = containerType;
    scon->data = data;
    return scon;
}
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt)
{
    releaser(pt->data);
    return (ShaderContainer_ptr)releaser(pt);
};

//* Point RGBA32
ShaderContainer_ptr newSPointRGBA32(int x, bool keyPoint, uint8_t alpha)
{
    sPointRGBA32_ptr pt = nullptr;
    pt = (sPointRGBA32_ptr)malloc(sizeof(sPointRGBA32_t));
    pt->keyPoint = keyPoint;
    pt->alpha = alpha;
    return newShaderContainer(x, SPOINT_RGBA32, (void *)pt);
}

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x)
{
    return newShaderContainer(x, SPOINT_RGB888, nullptr);
}

//* Line
ShaderContainer_ptr newSLine(int x0, int x1, int y1, bool antialiasing)
{
    sLine_ptr pt = nullptr;
    pt = (sLine_ptr)malloc(sizeof(sLine_t));
    pt->x1 = x1;
    pt->y1 = y1;
    pt->antialiasing = antialiasing;
    return newShaderContainer(x0, SLINE, (void *)pt);
}

//* Arc
ShaderContainer_ptr newSArc(int x, int radius, float startAngle,
                            float endAngle, bool anticlockwise, bool antialiasing)
{
    sArc_ptr pt = nullptr;
    pt = (sArc_ptr)malloc(sizeof(sArc_t));
    pt->radius = radius;
    pt->startAngle = startAngle;
    pt->endAngle = endAngle;
    pt->anticlockwise = anticlockwise;
    pt->antialiasing = antialiasing;
    return newShaderContainer(x, SARC, (void *)pt);
}

//* Write to shader buffer API

bool shaderPointCompare(void *data1, void *data2)
{
    ShaderContainer_ptr scon1 = (ShaderContainer_ptr)data1;
    ShaderContainer_ptr scon2 = (ShaderContainer_ptr)data2;
    return scon1->x < scon2->x;
}

void writeSPoint(LinkList_ptr *shaderInfo, int x, int y)
{
    // Create rgb888 point shader container
    ShaderContainer_ptr scon = newSPointRGB888(x);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}

void writeSPointAA(LinkList_ptr *shaderInfo, int x, int y, uint8_t alpha, bool keyPoint)
{
    // Create rgba32 point shader container
    ShaderContainer_ptr scon = newSPointRGBA32(x, keyPoint, alpha);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}

void writeSLine(LinkList_ptr *shaderInfo, int x0, int y0, int x1, int y1, bool antialiasing)
{
    ShaderContainer_ptr scon = newSLine(x0, x1, y1, antialiasing);
    shaderInfo[y0] = sortNewLinkListNode(shaderInfo[y0], (void *)scon, shaderPointCompare);
}

void writeSArc(LinkList_ptr *shaderInfo, int x, int y, int radius, float startAngle,
               float endAngle, bool anticlockwise, bool antialiasing)
{
    ShaderContainer_ptr scon = newSArc(x, radius, startAngle, endAngle, anticlockwise, antialiasing);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}