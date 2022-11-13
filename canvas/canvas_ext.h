#ifndef CANVAS_EXT_H
#define CANVAS_EXT_H

#include "../display/display_config.h"
#include "canvas.h"

canvas_err_t roundRectStroke(CanvaHandle_ptr handle, int x, int y, int width, int height,
                             int radiiCount, int *radii);

canvas_err_t roundRect(CanvaHandle_ptr handle, int x, int y, int width, int height,
                       int radiiCount, int *radii);

#endif