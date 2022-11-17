#ifndef CANVAS_EXT_H
#define CANVAS_EXT_H

#include "../display/display_config.h"
#include "canvas.h"

canvas_err_t roundRectInstance(CanvaHandle_ptr phd, int x, int y,
                               int width, int height, int topLeft, int topRight, int bottomRight, int bottomLeft);

canvas_err_t roundRect(CanvaHandle_ptr phd, int x, int y, int width, int height, int radiiCount, int *radii);

#endif