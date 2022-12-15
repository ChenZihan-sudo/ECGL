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
#include "canvas_shader.h"
#include "canvas_graphics.h"
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
        printf("LINETO2\n");
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

bool outAreaboundTruncated(int *penx, int *peny)
{
    bool outAreaBound = false;
    int x = *penx, y = *peny;

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

    *penx = x;
    *peny = y;

    return outAreaBound;
};

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
        hd->needNewSubpath = true;
        hd->subpathID = 0;
        hd->beginPenx = 0;
        hd->beginPeny = 0;
        hd->scanLineMin = 0;
        hd->scanLineMax = DISPLAY_HEIGHT - 1;
    }
    else
    {
        // Destory shaderInfo / Edge Info
    }
};

void checkAndCloseCurrentSubpath(CanvaHandle_ptr hd)
{
    // Check current subpath and if path not closed,
    // add FLine from begin coordinate to current pen coordinate for filling purpose
    if (hd->penx != hd->beginPenx || hd->peny != hd->beginPeny)
    {
        int ax, ay, bx, by;
        bool anticlockwise;

        if (hd->peny > hd->beginPeny)
        {
            ax = hd->beginPenx;
            ay = hd->beginPeny;
            bx = hd->penx;
            by = hd->peny;
            anticlockwise = true;
        }
        else
        {
            ax = hd->penx;
            ay = hd->peny;
            bx = hd->beginPenx;
            by = hd->beginPeny;
            anticlockwise = false;
        }

        CurrentSubpathID = hd->subpathID;
        writeFLine(hd->shaderInfo, ax, ay, bx, by, anticlockwise);
    }
}

void moveTo(CanvaHandle_ptr hd, int x, int y)
{
    // TODO: This limitation is temporary,
    // TODO: it unfollow HTML standard and will reconstruct in future.
    bool outAreaBound = outAreaboundTruncated(&x, &y);

    checkAndCloseCurrentSubpath(hd);

    hd->penx = x;
    hd->peny = y;
    hd->beginPenx = x;
    hd->beginPeny = y;

    hd->apiCalled = true;

    hd->subpathID++;
    hd->needNewSubpath = false;
};

void lineTo(CanvaHandle_ptr hd, int x, int y)
{
    // TODO: This limitation is temporary,
    // TODO: it unfollow HTML standard and will reconstruct in future.
    bool outAreaBound = outAreaboundTruncated(&x, &y);

    if (hd->needNewSubpath)
    {
        moveTo(hd, x, y);
        return;
    }

    int ax, ay, bx, by;
    bool anticlockwise;

    if (hd->peny > y)
    {
        ax = x;
        ay = y;
        bx = hd->penx;
        by = hd->peny;
        anticlockwise = true;
    }
    else
    {
        ax = hd->penx;
        ay = hd->peny;
        bx = x;
        by = y;
        anticlockwise = false;
    }

    CurrentSubpathID = hd->subpathID;
    writeSLine(hd->shaderInfo, ax, ay, bx, by, hd->antialiasing, anticlockwise);

    hd->penx = x;
    hd->peny = y;

    if (hd->beginPenx == x && hd->beginPeny == y)
        moveTo(hd, x, y);
};

void arcTo(CanvaHandle_ptr hd, int x1, int y1, int x2, int y2, int radius)
{
    // Ref: https://html.spec.whatwg.org/multipage/canvas.html#dom-context-2d-arcto

    // Ensure there is a subpath for (x1, y1).
    if (hd->needNewSubpath)
        moveTo(hd, x1, y1);
    if (radius < 0)
        return;

    int x0 = hd->penx, y0 = hd->peny;
    if ((x0 == x1 && y0 == y1) || (x1 == x2 && y1 == y2) || (radius == 0))
    {
        lineTo(hd, x1, y1); // line to point (x1,y1)
        printf("TYPE 1\n");
        return;
    }

    int x01 = x1 - x0, y01 = y1 - y0, x12 = x2 - x1, y12 = y2 - y1;
    float k01 = (float)y01 / (float)x01, k12 = (float)y12 / (float)x12;

    if (flEQUAL(k01, k12))
    {
        lineTo(hd, x1, y1); // line to point (x1,y1)
        printf("TYPE 2\n");
        return;
    }

    // y = kx + b;
    // Define l01: (x0,y0)<->(x1,y1), its tangent line l1.
    // Define l12: (x1,y1)<->(x2,y2), its tangent line l2.

    // Calculate line equations of l01, l12, arc origin point
    float b01 = y1 - k01 * x1;
    float b12 = y2 - k12 * x2;
    float m01 = radius * sqrt(pow(k01, 2.f) + 1.0);
    float m12 = radius * sqrt(pow(k12, 2.f) + 1.0);

    // Deal with the sign of m01 and m12, it have 4 possiblities.
    // Divide slope situations into 6 pieces as follow, and for any situations two lines can divide space into 4 blocks.
    // These steps as follow is to find a suitable space block by k01 and k12 to give sign of m01 and m12.
    // The rules can be find by manually test which one space block should give what kind of sign
    // and its listed in switch statement as follow.

    // Let (x1,y1) be the ralative origin point to (x0,y0), (x2,y2)
    int ox0 = x0 - x1, ox2 = x2 - x1;
    uint8_t key = (k01 >= 0.0) << 1 | (k12 >= 0.0);
    uint8_t keySpaceBlock = ((ox0 > 0) << 1) | (ox2 > 0);
    uint8_t dataArr[12] = {1, 2, 4, 3,
                           3, 4, 2, 1,
                           3, 2, 4, 1};
    switch (key)
    {
    case 0b00:
    {
        if (k01 < k12)
            key = dataArr[4 + keySpaceBlock];
        else
            key = dataArr[0 + keySpaceBlock];
        break;
    }
    case 0b01:
        key = dataArr[4 + keySpaceBlock];
        break;
    case 0b10:
        key = dataArr[0 + keySpaceBlock];
        break;
    case 0b11:
    {
        if (k01 > k12)
            key = dataArr[0 + keySpaceBlock];
        else
            key = dataArr[8 + keySpaceBlock];
    }
    break;
    }

    switch (key)
    {
    case 1:
        m01 = -m01;
        break;
    case 3:
        m12 = -m12;
        break;
    case 4:
    {
        m01 = -m01;
        m12 = -m12;
        break;
    }
    }

    float ox = (b12 - b01 + (m01 - m12)) / (k01 - k12);
    float oy = k01 * ox + b01 - m01;
    // Calculate two lines tangent with l01, l12, separately.
    float kl1 = -1.0 / k01;
    float kl2 = -1.0 / k12;
    float bl1 = oy - kl1 * ox;
    float bl2 = oy - kl2 * ox;
    // Calculate intersection points.
    float ix1 = (bl1 - b01) / (k01 - kl1);
    float iy1 = k01 * ix1 + b01;
    float ix2 = (bl2 - b12) / (k12 - kl2);
    float iy2 = k12 * ix2 + b12;
    // Transform point to angle
    // Transform P(ix1,iy1) and P(ix2,iy2) relative to origin point P(ox,oy)
    int rix1 = ix1 - ox;
    int riy1 = iy1 - oy;
    int rix2 = ix2 - ox;
    int riy2 = iy2 - oy;

    ArcAngles_t arcAngles = transformArcBeginEndPointToAngle(rix1, riy1, rix2, riy2, radius);
    float angleBegin = arcAngles.angleBegin, angleEnd = arcAngles.angleEnd;

    // TODO: Remove this below.
    IDM_writeAlphaBlendColor(ox, oy, 0x00FF00, 0xFF);
    IDM_writeAlphaBlendColor(ix1, iy1, 0x00FF00, 0xFF);
    IDM_writeAlphaBlendColor(ix2, iy2, 0x00FF00, 0xFF);
    // printf("k01 %f k12 %f\n", k01, k12);
    // printf("kl1 %f kl2 %f\n", kl1, kl2);
    // printf("ox:%f oy:%f\n", ox, oy);
    // printf("ix1:%f iy1:%f ix2:%f iy2:%f\n", ix1, iy1, ix2, iy2);

    if (angleBegin > angleEnd)
        swapf(&angleBegin, &angleEnd);

    bool anticlockwise = false;
    if (angleEnd - angleBegin > PI)
        anticlockwise = true;

    arc(hd, ox, oy, radius, angleBegin, angleEnd, anticlockwise);
}

ArcAngles_t transformArcBeginEndPointToAngle(int beginPointX, int beginPointY, int endPointX, int endPointY, int radius)
{
    ArcAngles_t arcAngles;
    int ptX = beginPointX, ptY = beginPointY, key;
    float result;
    for (size_t i = 0; i < 2; i++)
    {
        if (i == 1)
        {
            ptX = endPointX;
            ptY = endPointY;
        }

        key = (ptX > 0) << 1 | (ptY > 0);
        switch (key)
        {
        case 0b00:
            result = PI + asinf((float)abs(ptY) / radius);
            break;
        case 0b01:
            result = PI_0P5 + asinf((float)abs(ptX) / radius);
            break;
        case 0b10:
            result = PI_1P5 + asinf((float)abs(ptX) / radius);
            break;
        case 0b11:
            result = asinf((float)abs(ptY) / radius);
            break;
        };

        if (i == 0)
        {
            arcAngles.angleBegin = result;
        }
        else
        {
            arcAngles.angleEnd = result;
        }
    }
    return arcAngles;
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
                arcInstance(hd, scon->x, curY, sarc->radius, sarc->startAngle,
                            sarc->endAngle, scon->anticlockwise, sarc->antialiasing);
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
    // printf("FILLRULE:%d\n", fillRule);

    // Close current subpath
    checkAndCloseCurrentSubpath(hd);

    // Prepare shader status info
    CurrentSubpathID = hd->subpathID;
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
                        sarc->startAngle, sarc->endAngle, scon->anticlockwise, false);
        }
        break;
        case SROUNDRECT:
        {
            sRoundRect_ptr srrect = (sRoundRect_ptr)scon->data;
            roundRectInstance(hd, scon->x, curY, srrect->width, srrect->height,
                              srrect->topLeft, srrect->topRight, srrect->bottomRight, srrect->bottomLeft);
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

        // printf("curY:%d\n", curY);

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
            case FLINE:
            {
                sLine_ptr sli = (sLine_ptr)scon->data;
                float tm = 0.f;
                if (sli->y1 - (float)curY != 0.f)
                    tm = ((float)sli->x1 - (float)scon->x) / ((float)sli->y1 - (float)curY);
                fn = newFillNode_line((float)scon->x, (float)sli->x1, (float)sli->y1, tm, scon->anticlockwise);
            }
            break;
            }

            // Add node to AET
            if (fn != nullptr)
                AET = newLinkListNode(AET, (void *)fn);
        }

        // //?TEST
        // if (AET != nullptr)
        // {
        //     printf("Current AET:\n");
        //     LinkList_ptr lla = AET;
        //     for (;;)
        //     {
        //         FillNode_ptr fn = (FillNode_ptr)lla->data;
        //         if (fn->TYPE == FN_LINE)
        //         {
        //             printf("%f %d %f %f\n", fn->ax, (int)curY, fn->bx, fn->by);
        //         }
        //         if (lla->next != nullptr)
        //             lla = lla->next;
        //         else
        //             break;
        //     }
        //     printf("------------\n");
        // }

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
                            // printf("Delete Point %d %d\n", fn->x, fn->y);
                            goto deleteNode;
                        }
                    }
                    break;
                    case FN_LINE:
                    {
                        if (fn->by <= (float)curY)
                        {
                            // printf("Delete LINE %f %d %f %f\n", fn->ax, (int)curY, fn->bx, fn->by);
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
        // printf("Fragments!!!\n");
    }
    ShaderStatus = stSTROKE;
}

void arc(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise)
{
    // HINT this can be optimised if necessary.
    float startDrawPointX = ((float)radius * cos(startAngle));
    float startDrawPointY = ((float)radius * sin(startAngle));

    float endDrawPointX = ((float)radius * cos(endAngle));
    float endDrawPointY = ((float)radius * sin(endAngle));

    printf("sX %f sY %f eX %f eY %f\n", startDrawPointX, startDrawPointY, endDrawPointX, endDrawPointY);

    // // ! Not sure about this
    // if (anticlockwise)
    //     connectLine(hd, x + endDrawPointX, y + endDrawPointY);
    // else
    //     connectLine(hd, x + startDrawPointX, y + startDrawPointY);

    writeSArc(hd->shaderInfo, x, y, radius, startAngle, endAngle, hd->antialiasing, anticlockwise);

    // // ! Do test here.
    // // * Set Begin Point if it's the first call after beginPath()
    // if (anticlockwise)
    //     setBeginPoint(hd, x + startDrawPointX, y + startDrawPointY);
    // else
    //     setBeginPoint(hd, x + endDrawPointX, y + endDrawPointY);

    // // * Move the pen to the end point
    // if (anticlockwise)
    //     movePen(hd, x + startDrawPointX, y + startDrawPointY);
    // else
    //     movePen(hd, x + endDrawPointX, y + endDrawPointY);
}

canvas_err_t roundRect(CanvaHandle_ptr hd, int x, int y, int width, int height, int radiiCount, ...)
{
    if (!(0 < radiiCount && radiiCount <= 4))
        return CANVAS_FAIL_DATA_UNVALID;

    // Get the radii
    va_list vaList = nullptr;
    va_start(vaList, radiiCount);
    int radii[4] = {0};
    for (size_t i = 0; i < 4; i++)
        if ((radii[i] = va_arg(vaList, int)) < 0)
            return CANVAS_FAIL_DATA_UNVALID;
    va_end(vaList);

    int upperLeft = 0, upperRight = 0, lowerRight = 0, lowerLeft = 0;

    // [top-left-and-bottom-right-and-top-right-and-bottom-left]
    // [top-left-and-bottom-right, top-right-and-bottom-left]
    // [top-left, top-right-and-bottom-left, bottom-right]
    // [top-left, top-right, bottom-right, bottom-left]
    switch (radiiCount)
    {
    case 0:
        break;
    case 1:
    {
        upperLeft = radii[0];
        upperRight = radii[0];
        lowerRight = radii[0];
        lowerLeft = radii[0];
    }
    break;
    case 2:
    {
        upperLeft = radii[0];
        lowerRight = radii[0];
        upperRight = radii[1];
        lowerLeft = radii[1];
    }
    break;
    case 3:
    {
        upperLeft = radii[0];
        upperRight = radii[1];
        lowerLeft = radii[1];
        lowerRight = radii[2];
    }
    break;
    case 4:
    {
        upperLeft = radii[0];
        upperRight = radii[1];
        lowerRight = radii[2];
        lowerLeft = radii[3];
    }
    break;
    default:
        return CANVAS_FAIL_DATA_UNVALID;
        break;
    }

    // If width or height is negative, turn over the roundrect.
    if (height < 0)
    {
        y += height;
        swapi(&upperLeft, &lowerLeft);
        swapi(&upperRight, &lowerRight);
        height = -height;
    }
    if (width < 0)
    {
        x += width;
        swapi(&upperLeft, &upperRight);
        swapi(&lowerLeft, &lowerRight);
        width = -width;
    }

    // Corner curves must not overlap.
    int top = upperLeft + upperRight;
    int right = upperRight + lowerRight;
    int bottom = lowerRight + lowerLeft;
    int left = upperLeft + lowerLeft;

    // Let scale be the minimum value of the ratios w / top, h / right, w / bottom, h / left.
    float scale = width / (float)top;
    float v = height / (float)right;
    if (flLESS(v, scale))
        scale = v;
    v = width / (float)bottom;
    if (flLESS(v, scale))
        scale = v;
    v = height / (float)left;
    if (flLESS(v, scale))
        scale = v;

    // If scale is less than 1, then set the x and y members of upperLeft, upperRight, lowerLeft, and lowerRight
    // to their current values multiplied by scale.
    if (flLESS(scale, 1.f))
    {
        upperLeft = (float)upperLeft * scale;
        upperRight = (float)upperRight * scale;
        lowerLeft = (float)lowerLeft * scale;
        lowerRight = (float)lowerRight * scale;
    }

    // Move to the point (x + upperLeft["x"], y).
    moveTo(hd, x + upperLeft, y);
    writeSRoundRect(hd->shaderInfo, x, y, width, height, upperLeft, upperRight, lowerRight, lowerLeft, hd->antialiasing);

    // Create a new subpath with the point (x, y) as the only point in the subpath.
    moveTo(hd, x, y);
}

void closePath(CanvaHandle_ptr hd)
{
    if (!(hd->penx == hd->beginPenx && hd->peny == hd->beginPeny))
    {
        // printf("close: %d %d\n", hd->beginPenx, hd->beginPeny);
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