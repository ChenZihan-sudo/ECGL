#ifndef CANVAS_EXT_CPP
#define CANVAS_EXT_CPP

// extern "C"
//{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "../display/display_config.h"
#include "canvas_shader.h"
#include "canvas.h"
#include "canvas_ext.h"
#include "canvas_err.h"
//}

// Swap
void swapi(int *a, int *b)
{
    int c = *a;
    *a = *b;
    *b = c;
}

void swapf(float *a, float *b)
{
    float c = *a;
    *a = *b;
    *b = c;
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

canvas_err_t roundRectInstance(CanvaHandle_ptr hd, int x, int y,
                               int width, int height, int upperLeft, int upperRight, int lowerRight, int lowerLeft)
{
    // Draw a straight line to the point (x + w − upperRight["x"], y).
    strokeLine(hd, x + upperLeft, y, x + width - upperRight, y);
    // Draw an arc to the point (x + w, y + upperRight["y"]).
    arcInstance(hd, x + width - upperRight, y + upperRight, upperRight, 1.5f * PI, 2.f * PI, false, true);
    // Draw a straight line to the point (x + w, y + h − lowerRight["y"]).
    strokeLine(hd, x + width, y + upperRight, x + width, y + height - lowerRight);
    // Draw an arc to the point (x + w − lowerRight["x"], y + h).
    arcInstance(hd, x + width - lowerRight, y + height - lowerRight, lowerRight, 0.f * PI, 0.5f * PI, false, true);
    // Draw a straight line to the point (x + lowerLeft["x"], y + h).
    strokeLine(hd, x + width - lowerRight, y + height, x + lowerLeft, y + height);
    // Draw an arc to the point (x, y + h − lowerLeft["y"]).
    arcInstance(hd, x + lowerLeft, y + height - lowerLeft, lowerLeft, 0.5f * PI, 1.f * PI, false, true);
    // Draw a straight line to the point (x, y + upperLeft["y"]).
    strokeLine(hd, x, y + height - lowerLeft, x, y + upperLeft);
    // Draw an arc to the point (x + upperLeft["x"], y).
    arcInstance(hd, x + upperLeft, y + upperLeft, upperLeft, 1.f * PI, 1.5f * PI, false, true);
    return CANVAS_OK;
};

#endif