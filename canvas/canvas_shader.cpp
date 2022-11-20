#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "canvas_shader.h"
#include "canvas.h"

// * Shader status info
uint8_t ShaderStatus = stSTROKE;
LinkList_ptr *CurrentShaderInfo = nullptr;

// * Iterators

// HINT: PathInfo Iterator Structure
// void *data1; -> store the CanvaHandle_ptr data
// void *data2; -> current shaderInfo data pointer, store the LinkList_ptr data
// int val1; -> store the current line y posi
// int val2; -> the end flag of iterator
// int val3; -> the flag of whether current line y posi increased when call nextShaderContainer()

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
ShaderContainer_ptr nextShaderContainer(Iterator_ptr itor)
{
    ShaderContainer_ptr scon = nullptr;
    if (itor != nullptr)
    {
        if (itor->iteratorType == SHADER_INFO)
        {
            itor->val3 = 0;
            if (itor->val2 == 0) // End flag of iterator
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
                        itor->val3 = 1;
                    }
                }
                else
                {
                    itor->val1++;
                    itor->val3 = 1;
                }

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

// Giving whether line y posi increased when call nextShaderContainer().
int currentShaderInfoItorYChanged(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val3;
    return 0;
}

// Boolean giving whether the iteration is over
bool shaderInfoIterateEnd(Iterator_ptr itor)
{
    if (itor != nullptr)
        if (itor->iteratorType == SHADER_INFO)
            return itor->val2;
    return false;
}

//* Shader Container
ShaderContainer_ptr newShaderContainer(int x, int y, int containerType, void *data)
{
    ShaderContainer_ptr scon = (ShaderContainer_ptr)malloc(sizeof(ShaderContainer_t));
    scon->x = x;
    scon->y = y;
    scon->TYPE = containerType;
    scon->data = data;
    return scon;
}
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt)
{
    releaser(pt->data);
    return (ShaderContainer_ptr)releaser(pt);
};

//* For Fill use
FillNode_ptr newFillNode_line(float ax, float bx, float by, float tm)
{
    FillNode_ptr pt = (FillNode_ptr)malloc(sizeof(FillNode_t));
    pt->TYPE = FN_LINE;
    pt->ax = ax;
    pt->bx = bx;
    pt->by = by;
    pt->tm = tm;
    return pt;
}

FillNode_ptr newFillNode_point(int x, int y)
{
    FillNode_ptr pt = (FillNode_ptr)malloc(sizeof(FillNode_t));
    pt->TYPE = FN_POINT;
    pt->x = x;
    pt->y = y;
    return pt;
}

void writeFPoint(LinkList_ptr *shaderInfo, int x, int y)
{
    ShaderContainer_ptr scon = newShaderContainer(x, y, FPOINT, nullptr);
    shaderInfo[y] = newLinkListNode(shaderInfo[y], (void *)scon);
}

//* Point RGBA32
ShaderContainer_ptr newSPointRGBA32(int x, int y, bool keyPoint, uint8_t alpha)
{
    sPointRGBA32_ptr pt = nullptr;
    pt = (sPointRGBA32_ptr)malloc(sizeof(sPointRGBA32_t));
    pt->keyPoint = keyPoint;
    pt->alpha = alpha;
    return newShaderContainer(x, y, SPOINT_RGBA32, (void *)pt);
}

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, int y)
{
    return newShaderContainer(x, y, SPOINT_RGB888, nullptr);
}

//* Line
ShaderContainer_ptr newSLine(int x0, int y0, int x1, int y1, bool antialiasing)
{
    sLine_ptr pt = nullptr;
    pt = (sLine_ptr)malloc(sizeof(sLine_t));
    pt->x1 = x1;
    pt->y1 = y1;
    pt->antialiasing = antialiasing;
    return newShaderContainer(x0, y0, SLINE, (void *)pt);
}

//* Arc
ShaderContainer_ptr newSArc(int x, int y, int radius, float startAngle,
                            float endAngle, bool anticlockwise, bool antialiasing)
{
    sArc_ptr pt = nullptr;
    pt = (sArc_ptr)malloc(sizeof(sArc_t));
    pt->radius = radius;
    pt->startAngle = startAngle;
    pt->endAngle = endAngle;
    pt->anticlockwise = anticlockwise;
    pt->antialiasing = antialiasing;
    return newShaderContainer(x, y, SARC, (void *)pt);
}

//* RoundRect
ShaderContainer_ptr newSRoundRect(int x, int y, int width, int height,
                                  int topLeft, int topRight, int bottomRight, int bottomLeft, bool antialiasing)
{
    sRoundRect_ptr pt = nullptr;
    pt = (sRoundRect_ptr)malloc(sizeof(sRoundRect_t));
    pt->width = width;
    pt->height = height;
    pt->topLeft = topLeft;
    pt->topRight = topRight;
    pt->bottomLeft = bottomLeft;
    pt->bottomRight = bottomRight;
    pt->antialiasing = antialiasing;
    return newShaderContainer(x, y, SROUNDRECT, (void *)pt);
}

//* Write to shader buffer API
// HINT Except point and line container, don't need to give value y,
// HINT so you can remove it if necessary to increase efficiency a little.
bool shaderPointCompare(void *data1, void *data2)
{
    ShaderContainer_ptr scon1 = (ShaderContainer_ptr)data1;
    ShaderContainer_ptr scon2 = (ShaderContainer_ptr)data2;
    return scon1->x < scon2->x;
}

void writeSPoint(LinkList_ptr *shaderInfo, int x, int y)
{
    // Create rgb888 point shader container
    ShaderContainer_ptr scon = newSPointRGB888(x, y);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}

void writeSPointAA(LinkList_ptr *shaderInfo, int x, int y, uint8_t alpha, bool keyPoint)
{
    // Create rgba32 point shader container
    ShaderContainer_ptr scon = newSPointRGBA32(x, y, keyPoint, alpha);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}

void writeSLine(LinkList_ptr *shaderInfo, int x0, int y0, int x1, int y1, bool antialiasing)
{
    ShaderContainer_ptr scon = newSLine(x0, y0, x1, y1, antialiasing);
    shaderInfo[y0] = sortNewLinkListNode(shaderInfo[y0], (void *)scon, shaderPointCompare);
}

void writeSArc(LinkList_ptr *shaderInfo, int x, int y, int radius, float startAngle,
               float endAngle, bool anticlockwise, bool antialiasing)
{
    ShaderContainer_ptr scon = newSArc(x, y, radius, startAngle, endAngle, anticlockwise, antialiasing);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}

void writeSRoundRect(LinkList_ptr *shaderInfo, int x, int y, int width, int height,
                     int topLeft, int topRight, int bottomRight, int bottomLeft, bool antialiasing)
{
    ShaderContainer_ptr scon = newSRoundRect(x, y, width, height, topLeft, topRight, bottomRight, bottomLeft, antialiasing);
    shaderInfo[y] = sortNewLinkListNode(shaderInfo[y], (void *)scon, shaderPointCompare);
}
