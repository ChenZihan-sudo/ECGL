// #ifdef __cplusplus
//  extern "C"
//{
// #endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

#include "../display/display_config.h"
#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas_ext.h"
#include "canvas_shader.h"
#include "canvas.h"

// #ifdef __cplusplus
// }
// #endif

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr NULL
#define bool short
#endif

// * Canvas Functions

/// @brief Set Begin Point if it's the first call after beginPath()
/// @return True if set success
bool setBeginPoint(CanvaHandle_ptr hd, int beginPenx, int beginPeny)
{
    if (!hd->apiCalled)
    {
        hd->beginPenx = beginPenx;
        hd->beginPeny = beginPeny;
        hd->apiCalled = true;
        return true;
    }
    return false;
};

bool connectLine(CanvaHandle_ptr hd, int penx, int peny)
{
    if (hd->apiCalled)
    {
        lineTo(hd, penx, peny);
        return true;
    }
    return false;
}

void movePen(CanvaHandle_ptr hd, int penx, int peny)
{
    hd->penx = penx;
    hd->peny = peny;
}

//! BUG: 在被line调用时 可以进行优化
bool scanLineRangeUpdate(CanvaHandle_ptr hd, int y)
{
    if (y < hd->scanLineMin)
    {
        hd->scanLineMin = y;
        printf("[D] scanLineMin be set to %d\n", hd->scanLineMin);
        return true;
    }
    else if (y > hd->scanLineMax)
    {
        hd->scanLineMax = y;
        printf("[D] scanLineMax be set to %d\n", hd->scanLineMax);
        return true;
    }

    return false;
}

// * Point
Point_ptr newPoint(Point_ptr pt, int x, int y)
{
    if (pt == nullptr)
    {
        pt = (Point_ptr)malloc(sizeof(Point_t));
        pt->x = x;
        pt->y = y;
        return pt;
    }
    return nullptr;
};

// * Canvas
CanvaHandle_ptr newCanva(CanvaHandle_ptr canva)
{
    if (canva == nullptr)
        canva = (CanvaHandle_ptr)malloc(sizeof(CanvaHandle_t));
    return canva;
};

// TODO Uncompleted.
CanvaHandle_ptr releaseCanva(CanvaHandle_ptr canva)
{
    return (canva = nullptr);
};

// * Canvas Functions

// // HINT: Not a API Function. This function called by lineTo() only.
// bool compareET_lineTo(void *data1, void *data2)
// {
//     XET_ptr xetptr1 = (XET_ptr)data1;
//     XET_ptr xetptr2 = (XET_ptr)data2;
//     return xetptr1->by < xetptr2->by;
// }

bool outAreaBoundTruncated(int *px, int *py)
{
    bool outAreaBound = false;
    int x = *px, y = *py;

    if (x < 0)
    {
        outAreaBound = true;
        x = 0;
    }
    else if (x > DISPLAY_WIDTH - 1)
    {
        outAreaBound = true;
        x = DISPLAY_WIDTH - 1;
    }

    if (y < 0)
    {
        outAreaBound = true;
        y = 0;
    }
    else if (y > DISPLAY_HEIGHT - 1)
    {
        outAreaBound = true;
        y = DISPLAY_HEIGHT - 1;
    }

    *px = x;
    *py = y;

    return outAreaBound;
};

// TODO: 写警告或错误返回
void beginPath(CanvaHandle_ptr hd)
{
    if (hd != nullptr)
    {
        hd->shaderInfo = (LinkList_ptr *)calloc(DISPLAY_HEIGHT, sizeof(LinkList_ptr));
        hd->lineWidth = 1;
        hd->lineBrushType = CANVAS_DEFAULT_BRUSH;
        hd->rgb888 = 0x000000;
        hd->penx = 0;
        hd->peny = 0;
        hd->apiCalled = false;
        hd->antialiasing = true;
        hd->beginPenx = 0;
        hd->beginPeny = 0;
        hd->scanLineMin = DISPLAY_HEIGHT - 1;
        hd->scanLineMax = 0;
    }
    else
    {
        // Destory shaderInfo / Edge Info
    }
};

void moveTo(CanvaHandle_ptr hd, int x, int y)
{
    // bool outAreaBound = outAreaBoundTruncated(&x, &y);
    hd->penx = x;
    hd->peny = y;
    hd->beginPenx = x;
    hd->beginPeny = y;
    hd->apiCalled = true;
};

void lineTo(CanvaHandle_ptr hd, int x, int y)
{
    // bool outAreaBound = outAreaBoundTruncated(&x, &y);
    int ax, ay, bx, by;
    bool anticlockwise;

    //! Do test.
    scanLineRangeUpdate(hd, y);
    scanLineRangeUpdate(hd, hd->peny);

    if (hd->peny > y)
    {
        // hd->scanLineMin = y < hd->scanLineMin ? y : hd->scanLineMin;
        // hd->scanLineMax = hd->peny > hd->scanLineMax ? hd->peny : hd->scanLineMax;
        ax = x;
        ay = y;
        bx = hd->penx;
        by = hd->peny;
        anticlockwise = true;
    }
    else
    {
        // hd->scanLineMin = hd->peny < hd->scanLineMin ? hd->peny : hd->scanLineMin;
        // hd->scanLineMax = y > hd->scanLineMax ? y : hd->scanLineMax;
        ax = hd->penx;
        ay = hd->peny;
        bx = x;
        by = y;
        anticlockwise = false;
    }

    writeSLine(hd->shaderInfo, ax, ay, bx, by, hd->antialiasing, anticlockwise);

    hd->penx = x;
    hd->peny = y;
};

// HINT: Not a API Function.
int parts8EncodeTool(float pointX, float pointY)
{
    //* This can be used in arc or strokeline, etc.

    // HINT: How this 8 arc parts be encoded.
    //* Use 3 bits 'ABC' to represent a arc part, the first bit 'A' is !XGreatY, second bit 'B' !positiveX and third bit 'C' !positiveY.
    //* By this 3 bits a arc code can get 8 possiblity and these results are not same. Use this can generate arc code stored in parts8Encode below.
    int parts8Encode[8] = {0, 4, 6, 2, 3, 7, 5, 1};

    bool XGreatY = flGREATE(fabs(pointX), fabs(pointY));
    bool positiveX = flGREATE(pointX, 0.f);
    bool positiveY = flGREATE(pointY, 0.f);

    for (size_t i = 0; i < 8; i++)
    {
        bool comp1 = parts8Encode[i] & (0b100 | 0x00000000);
        bool comp2 = parts8Encode[i] & (0b010 | 0x00000000);
        bool comp3 = parts8Encode[i] & (0b001 | 0x00000000);

        if (!XGreatY == comp1 && !positiveX == comp2 && !positiveY == comp3)
            return i;
    }

    return 0;
}

// HINT: Not a API Function.
void circularConeFiler_writeColor(int x, int y, RGB888 color, float distance)
{
    // * This filter can be used when lineWidth = 1
    // float circularConeFilter[16] = {0.816670, 0.803552, 0.764825, 0.702447, 0.619998, 0.523599, 0.424875, 0.331953, 0.248111, 0.175448, 0.115264, 0.068198, 0.034262, 0.012797, 0.002325, 0.000000};
    uint8_t circularConeFilterHex256[16] = {0xd0, 0xcd, 0xc3, 0xb3, 0x9e, 0x86, 0x6c, 0x55, 0x3f, 0x2d, 0x1d, 0x11, 0x9, 0x3, 0x1};
    // uint8_t circularConeFilterHex256Enhanced[16] = { 0xf5, 0xf1, 0xe5, 0xd3, 0xba, 0x9d, 0x7f, 0x64, 0x4a, 0x35, 0x23, 0x14, 0xa, 0x4, 0x1 };
    uint8_t alphaHex = circularConeFilterHex256[(int)(fabs(distance) * 10.f)];
    IDM_writeAlphaBlendColor(x, y, color, alphaHex);
};

// HINT: Not a API Function.
void tranStrokeLinePosi(int id, int *x, int *y)
{
    int bufx = *x;
    int bufy = *y;
    switch (id)
    {
    case 4:
    {
        *x = bufy;
        *y = bufx;
        break;
    };
    case 6:
    {
        *x = bufy;
        *y = -bufx;
        break;
    };
    case 2:
    {
        *x = -bufx;
        break;
    };
    case 3:
    {
        *x = -bufx;
        *y = -bufy;
        break;
    };
    case 7:
    {
        *x = -bufy;
        *y = -bufx;
        break;
    };
    case 5:
    {
        *x = -bufy;
        *y = bufx;
        break;
    };
    case 1:
    {
        *y = -bufy;
        break;
    };
    }
}

// HINT: Not a API Function.
void tranbackStrokeLinePosi(int id, int *x, int *y)
{
    int bufx = *x;
    int bufy = *y;
    switch (id)
    {
    case 4:
    {
        *x = bufy;
        *y = bufx;
        break;
    };
    case 6:
    {
        *x = -bufy;
        *y = bufx;
        break;
    };
    case 2:
    {
        *x = -bufx;
        break;
    };
    case 3:
    {
        *x = -bufx;
        *y = -bufy;
        break;
    };
    case 7:
    {
        *x = -bufy;
        *y = -bufx;
        break;
    };
    case 5:
    {
        *x = bufy;
        *y = -bufx;
        break;
    };
    case 1:
    {
        *y = -bufy;
        break;
    };
    }
}

// HINT: 反走样参考 Reference about antialiasing: 孙家广．计算机图形学［M］．北京：清华大学出版社 1998．
// 可优化 使用WU反走样算法 or by https://scholar.google.com/scholar?q=%E4%B8%80%E7%A7%8D%E5%9F%BA%E4%BA%8E%E5%8A%A0%E6%9D%83%E5%8C%BA%E5%9F%9F%E9%87%87%E6%A0%B7%E7%9A%84%E7%9B%B4%E7%BA%BF%E5%8F%8D%E8%B5%B0%E6%A0%B7%E7%94%9F%E6%88%90%E7%AE%97%E6%B3%95
// Stroke line with anti-aliasing. (Use gupta sproull algorithm)
void strokeLineAA(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0, dy = y1 - y0;
    if ((dy <= 0 && dx >= 0) || (dy <= 0 && dx <= 0))
    {
        int bx0 = x0;
        int by0 = y0;
        x0 = x1;
        y0 = y1;
        x1 = bx0;
        y1 = by0;
        dx = x1 - x0;
        dy = y1 - y0;
    }
    float k = (float)dy / (float)dx;
    bool kCck = 0 <= fabs(k) && fabs(k) <= 1;
    int ex = dx, ey = dy;
    int arcEncode[8] = {0, 4, 6, 2, 3, 7, 5, 1};
    int id = arcEncode[parts8EncodeTool((float)dx, (float)dy)];
    tranStrokeLinePosi(id, &ex, &ey);

    RGB888 rgb888 = hd->rgb888;

    int a = 0 - ey;
    int b = ex;
    int x = 0, y = 0;

    int delta1 = 2 * a;
    int delta2 = 2 * (a + b);
    int d = 2 * a + b;
    int t = 0;

    float dval = 1.f / (2.f * sqrt(pow(a, 2.f) + pow(b, 2.f)));
    float q = -2.f * b * dval; // The distance between pixel and line

    int rx, ry;

    rx = x;
    ry = y;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, 0);
    rx = x;
    ry = y + 1;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q);
    rx = x;
    ry = y - 1;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q);

    while (x < ex)
    {
        if (d < 0)
        {
            t = d + b;
            y++;
            d += delta2;
        }
        else
        {
            t = d - b;
            d += delta1;
        }
        x++;

        rx = x;
        ry = y;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, t * dval);
        rx = x;
        ry = y + 1;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q - t * dval);
        rx = x;
        ry = y - 1;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q + t * dval);
    }

    // TODO: Remove This: Line Begin/End Point
    // IDM_writeAlphaBlendColor(x0, y0, 0x00FF00, 0xFF);
    // IDM_writeAlphaBlendColor(x1, y1, 0x00FF00, 0xFF);
}

void strokeLine(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1)
{
    // Use Bresenham Algorithm
    int dx = x1 - x0, dy = y1 - y0;
    if ((dy <= 0 && dx >= 0) || (dy <= 0 && dx <= 0))
    {
        int bx0 = x0;
        int by0 = y0;
        x0 = x1;
        y0 = y1;
        x1 = bx0;
        y1 = by0;
        dx = x1 - x0;
        dy = y1 - y0;
    }
    int e, x = x0, y = y0;
    float k = (float)dy / (float)dx;

    bool xCck = dx >= 0;
    bool kCck = 0 <= fabs(k) && fabs(k) <= 1;

    if (!xCck)
        dx = -(x1 - x0);

    if (kCck)
        e = 2 * dy - dx;
    else
        e = 2 * dx - dy;

    size_t i, totalCount;
    if (kCck)
        totalCount = dx;
    else
        totalCount = dy;

    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    bool brush1kCck = -1 <= k && k <= 1;

    for (i = 0; i <= totalCount; i++)
    {
        if (hd->lineWidth > 1)
        {
            switch (hd->lineBrushType)
            {
            case 1:
            {
                int mid = hd->lineWidth / 2;
                if (!brush1kCck)
                {
                    for (int i = 0; i < mid; i++)
                    {
                        IDM_writeColor(x + i, y, colorH8b, colorL8b);
                        IDM_writeColor(x - i, y, colorH8b, colorL8b);
                    }
                }
                else
                {
                    for (int i = 0; i < mid; i++)
                    {
                        IDM_writeColor(x, y + i, colorH8b, colorL8b);
                        IDM_writeColor(x, y - i, colorH8b, colorL8b);
                    }
                }
                IDM_writeColor(x, y, colorH8b, colorL8b);
            }
            break;
            case 2:
            {
                int mid = hd->lineWidth / 2;
                for (int i = -mid; i < mid + 1; i++)
                {
                    for (int j = -mid; j < mid + 1; j++)
                    {
                        IDM_writeColor(x + j, y + i, colorH8b, colorL8b);
                        IDM_writeColor(x + j, y + i, colorH8b, colorL8b);
                    }
                }
            }
            break;
            }
        }
        else
            IDM_writeColor(x, y, colorH8b, colorL8b);

        if (e >= 0)
        {
            if (kCck)
            {
                e = e + 2 * dy - 2 * dx;
                y++;
            }
            else
            {
                e = e + 2 * dx - 2 * dy;
                if (xCck)
                    x++;
                else
                    x--;
            }
        }
        else
        {
            if (kCck)
                e = e + 2 * dy;
            else
                e = e + 2 * dx;
        }

        if (kCck)
            if (xCck)
                x++;
            else
                x--;
        else
            y++;
    }

    // TODO: Remove This: Line Begin/End Point
    // uint16_t _color = RGB888_to_RGB565(0x00FF00);
    // uint8_t _colorH8b = (_color >> 8) & 0xFF;
    // uint8_t _colorL8b = _color & 0xFF;
    // IDM_writeColor(x0, y0, _colorH8b, _colorL8b);
    // IDM_writeColor(x1, y1, _colorH8b, _colorL8b);
}

void stroke(CanvaHandle_ptr hd)
{
    RGB888 rgb888 = hd->rgb888;
    uint8_t colorH8b = (rgb888 >> 8) & 0xFF;
    uint8_t colorL8b = rgb888 & 0xFF;

    int curY;
    Iterator_ptr itor = newShaderInfoIterator(hd);
    ShaderContainer_ptr scon = nullptr;
    while (!shaderInfoIterateEnd(itor))
    {
        curY = currentShaderInfoItorY(itor);
        scon = nextShaderContainer(itor);
        if (scon != nullptr)
        {
            switch (scon->TYPE)
            {
            case SPOINT_RGB888:
                IDM_writeColor(scon->x, curY, colorH8b, colorL8b);
                break;
            case SPOINT_RGBA32:
            {
                sPointRGBA32_ptr sp32 = (sPointRGBA32_ptr)scon->data;
                IDM_writeAlphaBlendColor(scon->x, curY, rgb888, sp32->alpha);
            }
            break;
            case SLINE:
            {
                sLine_ptr sli = (sLine_ptr)scon->data;

                if (sli->antialiasing)
                    strokeLineAA(hd, scon->x, curY, sli->x1, sli->y1);
                else
                    strokeLine(hd, scon->x, curY, sli->x1, sli->y1);
            }
            break;
            case SARC:
            {
                sArc_ptr sarc = (sArc_ptr)scon->data;

                // if (sarc->antialiasing)
                // else

                arcInstance(hd, scon->x, curY, sarc->radius,
                            sarc->startAngle, sarc->endAngle, sarc->anticlockwise);
            }
            break;
            case SROUNDRECT:
            {
                sRoundRect_ptr srrect = (sRoundRect_ptr)scon->data;

                // if (srrect->antialiasing)
                // else

                roundRectInstance(hd, scon->x, curY, srrect->width, srrect->height,
                                  srrect->topLeft, srrect->topRight, srrect->bottomRight, srrect->bottomLeft);
            }
            break;
            }
        }
    }
};

// TODO: OPTIMITE THIS CODE BELOW
// ? High effective flood fill algorithm reference from: http://www.williammalone.com/articles/html5-canvas-javascript-paint-bucket-tool/
// enum FloodFillRepresent_type
// {
//     BOUNDARY_COLOR = 0x00,      // Boundary representation color type
//     OLD_SEED_POINT_COLOR = 0x01 // Or Interior dot representation color type
// };
/// @param fillType BOUNDARY_COLOR or OLD_SEED_POINT_COLOR
/// @param typeColor Representation type of color, the color of the parameter fillType you set
void floodFill(int fillType, int seedX, int seedY, RGB888 typeColor, RGB888 fillColor)
{
    Stack_ptr st = nullptr;
    // This stack data model can expand capacity automatic,
    // so set capacity whatever depend on yourself.
    st = newStack(st, 100, true);

    Point_ptr pt = nullptr;
    pt = newPoint(pt, seedX, seedY);
    stackPush(st, (void *)pt);

    RGB565 type565Color = RGB888_to_RGB565(typeColor);
    RGB565 fill565Color = RGB888_to_RGB565(fillColor);

    uint8_t fillcolorH8b = (fill565Color >> 8) & 0xFF;
    uint8_t fillcolorL8b = fill565Color & 0xFF;

    while (stackSize(st))
    {
        bool reachLeft = false;
        bool reachRight = false;

        Point_ptr p = (Point_ptr)stackTop(st);
        stackPop(st);

        // write_display_memory_on();
        // printf("%d %d\n", p->x, p->y);

        RGB565 curColor = IDM_readColor_to_RGB565(p->x, p->y);

        while (1)
        {
            bool judge1 = false;
            switch (fillType)
            {
            case BOUNDARY_COLOR:
                judge1 = p->y > 0 && type565Color != curColor;
                break;
            case OLD_SEED_POINT_COLOR:
                judge1 = p->y > 0 && type565Color == curColor;
                break;
            }
            if (!judge1)
                break;

            p->y--;
            curColor = IDM_readColor_to_RGB565(p->x, p->y);
        }

        bool judge2 = false;
        switch (fillType)
        {
        case BOUNDARY_COLOR:
            judge2 = p->y > 0 && type565Color == curColor;
            break;
        case OLD_SEED_POINT_COLOR:
            judge2 = p->y > 0 && type565Color != curColor;
            break;
        }
        if (judge2)
            p->y++;

        curColor = IDM_readColor_to_RGB565(p->x, p->y);

        while (1)
        {
            bool judge3 = false;
            switch (fillType)
            {
            case BOUNDARY_COLOR:
                judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color != curColor && fill565Color != curColor;
                break;
            case OLD_SEED_POINT_COLOR:
                judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color == curColor;
                //? If BUG, try use this: judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color == curColor && fill565Color != curColor;
                break;
            }
            if (!judge3)
                break;

            IDM_writeColor(p->x, p->y, fillcolorH8b, fillcolorL8b);
            // write_display_memory_on();
            // printf("%d %d Size:%d\n", p->x, p->y, stackSize(st));

            if (p->x > 0)
            {
                curColor = IDM_readColor_to_RGB565(p->x - 1, p->y);

                bool judge4 = false;
                switch (fillType)
                {
                case BOUNDARY_COLOR:
                    judge4 = type565Color != curColor && fill565Color != curColor && !reachLeft;
                    break;
                case OLD_SEED_POINT_COLOR:
                    judge4 = type565Color == curColor && !reachLeft;
                    //? If BUG, try use this: judge4 = type565Color == curColor && fill565Color != curColor && !reachLeft;
                    break;
                }
                if (judge4)
                {
                    Point_ptr pl = nullptr;
                    pl = newPoint(pl, p->x - 1, p->y);
                    stackPush(st, (void *)pl);
                    reachLeft = true;
                }
            }

            if (p->x < DISPLAY_WIDTH - 1)
            {
                curColor = IDM_readColor_to_RGB565(p->x + 1, p->y);

                bool judge5 = false;
                switch (fillType)
                {
                case BOUNDARY_COLOR:
                    judge5 = type565Color != curColor && fill565Color != curColor && !reachRight;
                    break;
                case OLD_SEED_POINT_COLOR:
                    judge5 = type565Color == curColor && !reachRight;
                    //? If BUG, try use this: judge5 = type565Color == curColor && fill565Color != curColor && !reachRight;
                    break;
                }
                if (judge5)
                {
                    Point_ptr pr = nullptr;
                    pr = newPoint(pr, p->x + 1, p->y);
                    stackPush(st, (void *)pr);
                    reachRight = true;
                }
            }

            p->y++;
            curColor = IDM_readColor_to_RGB565(p->x, p->y);
        }

        // p = releasePoint(p);
        p = (Point_ptr)releaser(p);
    }

    st = releaseStack(st);
};

// HINT: Not a API Function. This function called by fill() only.
bool fill_compareFillNode(void *data1, void *data2)
{
    FillNode_ptr fn1 = (FillNode_ptr)data1;
    FillNode_ptr fn2 = (FillNode_ptr)data2;
    float compare1, compare2;
    switch (fn1->TYPE)
    {
    case FN_LINE:
        compare1 = fn1->ax;
        break;
    case FN_POINT:
        compare1 = (float)fn1->x;
        break;
    }
    switch (fn2->TYPE)
    {
    case FN_LINE:
        compare2 = fn2->ax;
        break;
    case FN_POINT:
        compare2 = (float)fn2->x;
        break;
    }

    return flLESSE(compare1, compare2);
}

void fill(CanvaHandle_ptr hd, ...)
{
    // Get the fill rule
    va_list vaList = nullptr;
    va_start(vaList, hd);
    int fillRule = va_arg(vaList, int);
    if (fillRule != NONZERO)
        fillRule = EVENODD;
    va_end(vaList);
    printf("FILLRULE:%d\n", fillRule);

    // Prepare shader info
    ShaderStatus = stFILL;
    CurrentShaderInfo = hd->shaderInfo;

    // Prepare Color data
    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    // Prepare iterator
    int curY;
    Iterator_ptr itor = newShaderInfoIterator(hd);
    ShaderContainer_ptr scon = nullptr;

    // Resolve some graphic to point
    while (!shaderInfoIterateEnd(itor))
    {
        curY = currentShaderInfoItorY(itor);
        scon = nextShaderContainer(itor);
        if (scon == nullptr)
            continue;

        switch (scon->TYPE)
        {
        case SARC:
        {
            sArc_ptr sarc = (sArc_ptr)scon->data;

            arcInstance(hd, scon->x, curY, sarc->radius,
                        sarc->startAngle, sarc->endAngle, sarc->anticlockwise);
        }
        break;
        }
    }
    releaser(itor);

    // Use Scanline Fill Algorithm
    LinkList_ptr AET = NULL;
    int curYChanged = 0;
    itor = newShaderInfoIterator(hd);
    while (!shaderInfoIterateEnd(itor))
    {
        curY = currentShaderInfoItorY(itor);
        scon = nextShaderContainer(itor);
        curYChanged = currentShaderInfoItorYChanged(itor);

        printf("curY:%d\n", curY);

        if (scon != nullptr)
        {
            // Generate node
            FillNode_ptr fn = nullptr;
            switch (scon->TYPE)
            {
            case FPOINT:
            {
                fn = newFillNode_point(scon->x, curY, scon->anticlockwise);
            }
            break;
            case SPOINT_RGBA32:
            {
                sPointRGBA32_ptr sp = (sPointRGBA32_ptr)scon->data;
                if (sp->keyPoint)
                    fn = newFillNode_point(scon->x, curY, scon->anticlockwise);
            }
            break;
            case SPOINT_RGB888:
            {
                fn = newFillNode_point(scon->x, curY, scon->anticlockwise);
            }
            break;
            case SLINE:
            {
                sLine_ptr sli = (sLine_ptr)scon->data;
                float tm = 0.f;
                if (sli->y1 - (float)curY != 0.f)
                    tm = ((float)sli->x1 - (float)scon->x) / ((float)sli->y1 - (float)curY);
                fn = newFillNode_line((float)scon->x, (float)sli->x1, (float)sli->y1, tm, sli->anticlockwise);
            }
            break;
            }

            // Add node to AET
            if (fn != nullptr)
                AET = newLinkListNode(AET, (void *)fn);
        }

        //?TEST
        if (AET != nullptr)
        {
            printf("Current AET:\n");
            LinkList_ptr lla = AET;
            for (;;)
            {
                FillNode_ptr fn = (FillNode_ptr)lla->data;
                if (fn->TYPE == FN_LINE)
                {
                    printf("%f %d %f %f\n", fn->ax, (int)curY, fn->bx, fn->by);
                }
                if (lla->next != nullptr)
                    lla = lla->next;
                else
                    break;
            }
            printf("------------\n");
        }

        if (curYChanged)
        {
            if (AET != nullptr)
            {
                // Check nodes in AET and remove it if need.
                LinkList_ptr node = AET;
                LinkList_ptr lastNode = nullptr;
                LinkList_ptr next = nullptr;
                for (;;)
                {
                    next = node->next;
                    FillNode_ptr fn = (FillNode_ptr)node->data;
                    // The main propose of using goto is to made
                    // the code not repeat and improve efficiency.
                    switch (fn->TYPE)
                    {
                    case FN_POINT:
                    {
                        if (fn->y < curY)
                        {
                            printf("Delete Point %d %d\n", fn->x, fn->y);
                            goto deleteNode;
                        }
                    }
                    break;
                    case FN_LINE:
                    {
                        if (fn->by <= (float)curY)
                        {
                            printf("Delete LINE %f %d %f %f\n", fn->ax, (int)curY, fn->bx, fn->by);
                            goto deleteNode;
                        }
                    }
                    break;
                    }

                    while (false)
                    {
                    deleteNode:
                        next = node->next;
                        free(fn);
                        free(node);
                        node = nullptr;
                        if (lastNode != nullptr)
                            lastNode->next = next;
                        else
                            AET = next;
                    }

                    if (next != nullptr)
                    {
                        if (node != nullptr)
                            lastNode = node;
                        node = next;
                    }
                    else
                        break;
                }
            }

            // Sort the nodes
            AET = sortLinkList(AET, fill_compareFillNode);

            // Draw the pixel
            LinkList_ptr nodes = AET;
            if (nodes != nullptr)
            {
                if (fillRule == NONZERO)
                {
                    printf("NONZERO RUN\n");
                    int num = 0;
                    int lastDrawPoint, curDrawPoint;
                    FillNode_ptr fn = (FillNode_ptr)nodes->data;

                    switch (fn->TYPE)
                    {
                    case FN_LINE:
                        lastDrawPoint = (int)fn->ax + 1;
                        break;
                    case FN_POINT:
                        lastDrawPoint = fn->x + 1;
                        break;
                    }
                    if (!fn->anticlockwise)
                        num++;
                    else
                        num--;
                    nodes = readLinkListNode(nodes);

                    while (nodes != nullptr)
                    {
                        fn = (FillNode_ptr)nodes->data;
                        switch (fn->TYPE)
                        {
                        case FN_LINE:
                            curDrawPoint = (int)fn->ax + 1;
                            break;
                        case FN_POINT:
                            curDrawPoint = fn->x + 1;
                            break;
                        }

                        // Draw
                        if (num != 0)
                        {
                            for (int drawX = lastDrawPoint; drawX <= curDrawPoint; drawX++)
                            {
                                IDM_writeColor(drawX, curY, colorH8b, colorL8b);
                            }
                        }

                        if (!fn->anticlockwise)
                            num++;
                        else
                            num--;
                        lastDrawPoint = curDrawPoint;
                        nodes = readLinkListNode(nodes);
                    }
                }
                else
                {
                    // EVENODD
                    while (nodes != nullptr)
                    {
                        int drawBegin, drawEnd;

                        FillNode_ptr fnBegin = (FillNode_ptr)nodes->data;
                        switch (fnBegin->TYPE)
                        {
                        case FN_LINE:
                            drawBegin = (int)fnBegin->ax + 1;
                            break;
                        case FN_POINT:
                            drawBegin = fnBegin->x + 1;
                            break;
                        }

                        // Read next node
                        nodes = readLinkListNode(nodes);
                        if (nodes == nullptr)
                            break;

                        FillNode_ptr fnEnd = (FillNode_ptr)nodes->data;
                        switch (fnEnd->TYPE)
                        {
                        case FN_LINE:
                            drawEnd = (int)fnEnd->ax;
                            break;
                        case FN_POINT:
                            drawEnd = fnEnd->x;
                            break;
                        }
                        printf("ANTICLOCKWISE:%d\n", fnEnd->anticlockwise);

                        printf("%d %d\n", drawBegin, drawEnd);

                        // Draw
                        for (int drawX = drawBegin; drawX <= drawEnd; drawX++)
                        {
                            IDM_writeColor(drawX, curY, colorH8b, colorL8b);
                        }

                        nodes = readLinkListNode(nodes);
                    }
                }

                // Update the AET nodes
                nodes = AET;
                while (nodes != nullptr)
                {
                    FillNode_ptr fn = (FillNode_ptr)nodes->data;
                    if (fn->TYPE == FN_LINE)
                        fn->ax += fn->tm;
                    nodes = readLinkListNode(nodes);
                }
            }
        }
    }
    releaser(itor);
    // Clean up memory fragments if necessary.
    if (AET != nullptr)
    {
        printf("Fragments!!!\n");
    }
    ShaderStatus = stSTROKE;
}

void drawCircle(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise)
{
    // Color
    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    // Draw circle
    int drawX = radius;
    int drawY = 0;
    int d = 3 - 2 * radius;

    while (drawX + 1 > drawY)
    {
        switch (ShaderStatus)
        {
        case stSTROKE:
        {
            IDM_writeColor(drawX + x, drawY + y, colorH8b, colorL8b);
            IDM_writeColor(drawY + x, drawX + y, colorH8b, colorL8b);
            IDM_writeColor(-drawY + x, drawX + y, colorH8b, colorL8b);
            IDM_writeColor(-drawX + x, drawY + y, colorH8b, colorL8b);
            IDM_writeColor(-drawX + x, -drawY + y, colorH8b, colorL8b);
            IDM_writeColor(-drawY + x, -drawX + y, colorH8b, colorL8b);
            IDM_writeColor(drawY + x, -drawX + y, colorH8b, colorL8b);
            IDM_writeColor(drawX + x, -drawY + y, colorH8b, colorL8b);
        }
        break;
        case stFILL:
        {
            writeFPoint(CurrentShaderInfo, drawX + x, drawY + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, drawY + x, drawX + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, -drawY + x, drawX + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, -drawX + x, drawY + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, -drawX + x, -drawY + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, -drawY + x, -drawX + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, drawY + x, -drawX + y, anticlockwise);
            writeFPoint(CurrentShaderInfo, drawX + x, -drawY + y, anticlockwise);
        }
        break;
        }

        if (d >= 0)
        {
            d = d + 4 * drawY - 4 * drawX + 10;
            drawX--;
        }
        else
        {
            d = d + 4 * drawY + 6;
        }
        drawY++;
    }
};

void arcDrawFP(int arcId, int drawX, int drawY, int x, int y, bool anticlockwise)
{
    switch (arcId)
    {
    case 0:
        writeFPoint(CurrentShaderInfo, drawX + x, drawY + y, anticlockwise);
        break;
    case 4:
        writeFPoint(CurrentShaderInfo, drawY + x, drawX + y, anticlockwise);
        break;
    case 6:
        writeFPoint(CurrentShaderInfo, -drawY + x, drawX + y, anticlockwise);
        break;
    case 2:
        writeFPoint(CurrentShaderInfo, -drawX + x, drawY + y, anticlockwise);
        break;
    case 3:
        writeFPoint(CurrentShaderInfo, -drawX + x, -drawY + y, anticlockwise);
        break;
    case 7:
        writeFPoint(CurrentShaderInfo, -drawY + x, -drawX + y, anticlockwise);
        break;
    case 5:
        writeFPoint(CurrentShaderInfo, drawY + x, -drawX + y, anticlockwise);
        break;
    case 1:
        writeFPoint(CurrentShaderInfo, drawX + x, -drawY + y, anticlockwise);
        break;
    }
}

// HINT: Not a API Function. This Function Called by arc() Only.
void arcDraw(int arcId, int drawX, int drawY, int x, int y, uint8_t colorH8b, uint8_t colorL8b)
{
    switch (arcId)
    {
    case 0:
        IDM_writeColor(drawX + x, drawY + y, colorH8b, colorL8b);
        break;
    case 4:
        IDM_writeColor(drawY + x, drawX + y, colorH8b, colorL8b);
        break;
    case 6:
        IDM_writeColor(-drawY + x, drawX + y, colorH8b, colorL8b);
        break;
    case 2:
        IDM_writeColor(-drawX + x, drawY + y, colorH8b, colorL8b);
        break;
    case 3:
        IDM_writeColor(-drawX + x, -drawY + y, colorH8b, colorL8b);
        break;
    case 7:
        IDM_writeColor(-drawY + x, -drawX + y, colorH8b, colorL8b);
        break;
    case 5:
        IDM_writeColor(drawY + x, -drawX + y, colorH8b, colorL8b);
        break;
    case 1:
        IDM_writeColor(drawX + x, -drawY + y, colorH8b, colorL8b);
        break;
    }
}
// HINT: Not a API Function. This Function Called by arc() Only.
bool arcDrawBorderChecker(int arcId, int drawX, int drawY, int drawPointX, int drawPointY)
{
    //* Check only x side or y side is not enough and will lose precise, so we need check x and y side both. :)
    switch (arcId)
    {
    case 0:
        return drawX < drawPointX || drawY > drawPointY;
        break;
    case 4:
        return drawY < drawPointX || drawX > drawPointY;
        break;
    case 6:
        return -drawY < drawPointX || drawX < drawPointY;
        break;
    case 2:
        return -drawX < drawPointX || drawY < drawPointY;
        break;
    case 3:
        return -drawX > drawPointX || -drawY < drawPointY;
        break;
    case 7:
        return -drawY > drawPointX || -drawX < drawPointY;
        break;
    case 5:
        return drawY > drawPointX || -drawX > drawPointY;
        break;
    case 1:
        return drawX > drawPointX || -drawY > drawPointY;
        break;
    }
    return false;
}

void arcInstance(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise)
{
    //* We cut the whole circle into 8 parts. And encode this parts by function parts8EncodeTool().
    //* We store this parts code into array arcs which is the parts we need to draw.
    //* If the end point and begin point of arc in the same part, it's possible that we need maximum 9 arc parts to store the part we want to draw,
    //* and in these 9 parts have two same arc parts. The function arcDrawBorderChecker() will check the border of two arc parts.

    // Judge to draw the circle or arc.
    if (flGREATE(fabs(startAngle - endAngle), 2.f * PI))
    {
        drawCircle(hd, x, y, radius, anticlockwise);
        return;
    }

    bool originanticlockwise = anticlockwise;

    // Swap if anticloskwise
    if (anticlockwise)
    {
        anticlockwise = false;
        float dataBuffer = startAngle;
        startAngle = endAngle;
        endAngle = dataBuffer;
    }

    float startDrawPointX = ((float)radius * cos(startAngle));
    float startDrawPointY = ((float)radius * sin(startAngle));

    float endDrawPointX = ((float)radius * cos(endAngle));
    float endDrawPointY = ((float)radius * sin(endAngle));

    int arcEncode[8] = {0, 4, 6, 2, 3, 7, 5, 1};
    int sCodePosi = parts8EncodeTool(startDrawPointX, startDrawPointY);
    int eCodePosi = parts8EncodeTool(endDrawPointX, endDrawPointY);
    int posi = sCodePosi;

    bool equalBreak = false;
    float sAngleComp = (startAngle / PI - (float)((int)(startAngle / PI) / 2) * 2.f);
    float eAngleComp = (endAngle / PI - (float)((int)(endAngle / PI) / 2) * 2.f);

    int arcs[9] = {0};
    int arcCount = 0;
    for (size_t i = 0; i < 9; i++)
    {
        arcs[i] = arcEncode[posi];
        arcCount++;
        if (posi == eCodePosi)
        {
            if ((sCodePosi == eCodePosi && !equalBreak) &&
                ((sAngleComp < eAngleComp && anticlockwise) ||
                 (sAngleComp > eAngleComp && !anticlockwise)))
            {
                equalBreak = true;
            }
            else
                break;
        }
        posi = posi >= 7 ? 0 : posi + 1;
    }

    // Color
    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    // Draw arc
    int drawX = radius;
    int drawY = 0;
    int d = 3 - 2 * radius;

    // Use Bresenham and MidPoint algorithm.
    while (drawX + 1 > drawY)
    {
        for (size_t i = 0; i < arcCount; i++)
        {
            int arcId = arcs[i];
            if (i == 0)
            {
                bool checker1 = arcDrawBorderChecker(arcId, drawX, drawY, startDrawPointX, startDrawPointY);
                if (checker1)
                    goto drawarc;
            }
            else if (i == arcCount - 1)
            {
                bool checker2 = arcDrawBorderChecker(arcId, drawX, drawY, endDrawPointX, endDrawPointY);
                if (!checker2)
                    goto drawarc;
            }
            else
                goto drawarc;

            while (false)
            {
            drawarc:
                switch (ShaderStatus)
                {
                case stSTROKE:
                    arcDraw(arcId, drawX, drawY, x, y, colorH8b, colorL8b);
                    break;
                case stFILL:
                    arcDrawFP(arcId, drawX, drawY, x, y, originanticlockwise);
                    break;
                }
            }
        }

        if (d >= 0)
        {
            d = d + 4 * drawY - 4 * drawX + 10;
            drawX--;
        }
        else
        {
            d = d + 4 * drawY + 6;
        }
        drawY++;
    }

    // TODO: Remove this
    IDM_writeColor(x, y, colorH8b, colorL8b);
}

void arc(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise)
{
    // HINT this can be optimised if necessary.
    float startDrawPointX = ((float)radius * cos(startAngle));
    float startDrawPointY = ((float)radius * sin(startAngle));

    float endDrawPointX = ((float)radius * cos(endAngle));
    float endDrawPointY = ((float)radius * sin(endAngle));

    // ! Not sure about this
    if (anticlockwise)
        connectLine(hd, x + endDrawPointX, y + endDrawPointY);
    else
        connectLine(hd, x + startDrawPointX, y + startDrawPointY);

    writeSArc(hd->shaderInfo, x, y, radius, startAngle, endAngle, anticlockwise, hd->antialiasing);

    // ! Do test.
    scanLineRangeUpdate(hd, y);

    // ! Do test here.
    // * Set Begin Point if it's the first call after beginPath()
    if (anticlockwise)
        setBeginPoint(hd, x + startDrawPointX, y + startDrawPointY);
    else
        setBeginPoint(hd, x + endDrawPointX, y + endDrawPointY);

    // * Move the pen to the end point
    if (anticlockwise)
        movePen(hd, x + startDrawPointX, y + startDrawPointY);
    else
        movePen(hd, x + endDrawPointX, y + endDrawPointY);
}

void closePath(CanvaHandle_ptr hd)
{
    if (!(hd->penx == hd->beginPenx && hd->peny == hd->beginPeny))
    {
        printf("close: %d %d\n", hd->beginPenx, hd->beginPeny);
        lineTo(hd, hd->beginPenx, hd->beginPeny);
    }
}

void pathDestory(CanvaHandle_ptr hd){
    // for (int i = 0; i < 100; i++)
    // {
    //     releaseLinkListNode(hd->pathInfo[i]);
    //     free(hd->pathInfo[i]->data);
    // }
    // free(hd->pathInfo);
};

void *releaser(void *pt)
{
    if (pt != nullptr)
        free(pt);
    return (pt = nullptr);
}