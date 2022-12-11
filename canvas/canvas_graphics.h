#include "../display/display_config.h"
#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas.h"

int parts8EncodeTool(float pointX, float pointY);
void circularConeFiler_writeColor(int x, int y, RGB888 color, float distance);
void tranStrokeLinePosi(int id, int *x, int *y);
void tranbackStrokeLinePosi(int id, int *x, int *y);

void strokeLineAA(CanvaHandle_ptr hdl, int x0, int y0, int x1, int y1);
void strokeLineAA2(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1);
void strokeLine(CanvaHandle_ptr hdl, int x0, int y0, int x1, int y1);

enum FloodFillRepresent_type
{
    BOUNDARY_COLOR = 0x00,      // Boundary representation color type
    OLD_SEED_POINT_COLOR = 0x01 // Or Interior dot representation color type
};
void floodFill(int type, int seedX, int seedY, RGB888 typeColor, RGB888 fillColor);

void arcDrawFP(int arcId, int drawX, int drawY, int x, int y, bool anticlockwise);
void arcDrawAA(int arcId, int x0, int y0, int x1, int y1, int x, int y, uint8_t h0, uint8_t h1, RGB888 color);
void arcDraw(int arcId, int drawX, int drawY, int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b);
bool arcDrawBorderChecker(int arcId, int drawX, int drawY, int drawPointX, int drawPointY);

void arcInstance(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise, bool antialiasing);
void drawCircle(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise);
void drawCircleAA(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise);

canvas_err_t roundRectInstance(CanvaHandle_ptr hd, int x, int y,
                               int width, int height, int upperLeft, int upperRight, int lowerRight, int lowerLeft);
