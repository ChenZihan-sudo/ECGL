#ifndef CANVAS_EXT_CPP
#define CANVAS_EXT_CPP

// extern "C"
//{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../display/display_config.h"
#include "canvas_shader.h"
#include "canvas.h"
#include "canvas_ext.h"
//}

// TODO 存在问题，见MDN标准Canvas: roundRect()

canvas_err_t roundRect(CanvaHandle_ptr hd, int x, int y, int width, int height, int radiiCount, int *radii)
{
    // all-corners
    // [top-left-and-bottom-right, top-right-and-bottom-left]
    // [top-left, top-right-and-bottom-left, bottom-right]
    // [top-left, top-right, bottom-right, bottom-left]
    int topLeft = 0, topRight = 0, bottomRight = 0, bottomLeft = 0;

    switch (radiiCount)
    {
    case 0:
        break;
    case 1:
    {
        topLeft = radii[0];
        topRight = radii[0];
        bottomRight = radii[0];
        bottomLeft = radii[0];
    }
    break;
    case 2:
    {
        topLeft = radii[0];
        topRight = radii[1];
        bottomRight = radii[0];
        bottomLeft = radii[1];
    }
    break;
    case 3:
    {
        topLeft = radii[0];
        topRight = radii[1];
        bottomRight = radii[2];
        bottomLeft = radii[1];
    }
    break;
    case 4:
    {
        topLeft = radii[0];
        topRight = radii[1];
        bottomRight = radii[2];
        bottomLeft = radii[3];
    }
    break;
    default:
        return CANVAS_FAIL_DATA_UNVALID;
        break;
    }

    // ! Do test.
    scanLineRangeUpdate(hd, y);

    // ! Uncertain border radius
    // Priority 1>2>3>4
    int maxLimit = width > height ? height : width;
    topLeft = maxLimit < topLeft ? maxLimit : topLeft;
    topRight = maxLimit < topRight ? maxLimit : topRight;
    bottomRight = maxLimit < bottomRight ? maxLimit : bottomRight;
    bottomLeft = maxLimit < bottomLeft ? maxLimit : bottomLeft;

    topRight = width - topLeft < topRight ? width - topLeft : topRight;
    bottomLeft = height - topLeft < bottomLeft ? height - topLeft : bottomLeft;
    bottomRight = height - topRight < bottomRight ? height - topRight : bottomRight;

    printf("=>%d %d %d %d\n", topLeft, topRight, bottomRight, bottomLeft);

    writeSRoundRect(hd->shaderInfo, x, y, width, height,
                    topLeft, topRight, bottomRight, bottomLeft, hd->antialiasing);

    movePen(hd, x, y);
}

/// @brief
/// @param hd
/// @param x
/// @param y
/// @param width
/// @param height
/// @param radiiCount
/// @param radii
/// @return canvas_err_t
canvas_err_t roundRectInstance(CanvaHandle_ptr hd, int x, int y,
                               int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft)
{

    strokeLine(hd, x + topLeft, y, x + width - topRight, y);
    strokeLine(hd, x + bottomLeft, y + height, x + width - bottomRight, y + height);
    strokeLine(hd, x, y + topLeft, x, y + height - bottomLeft);
    strokeLine(hd, x + width, y + topRight, x + width, y + height - bottomRight);

    arcInstance(hd, x + topLeft, y + topLeft, topLeft, 1.f * PI, 1.5f * PI, false);
    arcInstance(hd, x + width - topRight, y + topRight, topRight, 1.5f * PI, 2.f * PI, false);
    arcInstance(hd, x + bottomLeft, y + height - bottomLeft, bottomLeft, 0.5f * PI, 1.001f * PI, false);
    arcInstance(hd, x + width - bottomRight, y + height - bottomRight, bottomRight, 0.0f * PI, 0.5f * PI, false);

    return CANVAS_OK;
};

#endif