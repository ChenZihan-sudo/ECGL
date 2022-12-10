#ifndef CANVAS_H
#define CANVAS_H

#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas_config.h"
#include "canvas_err.h"

//* Edge Table
typedef struct XET XET_t;
typedef struct XET *XET_ptr;
/* OPTIMIZE: When using pathInfo[i], it's data type is XET,
but in this data type we don't need the variable float tm, so remove tm. */
struct XET
{
    float ax;
    float bx;
    float by;
    float tm;
};

//* Canvas Main Structure
typedef struct CanvaHandle CanvaHandle_t;
typedef struct CanvaHandle *CanvaHandle_ptr;
typedef int Subpath_id;
struct CanvaHandle
{
    int lineWidth;

    /** @param lineBrushType
     *  @param 0 Default Brush
     *  @param 1 Horizontal/Vertical Brush
     *  @param 2 Square Brush
     */
    int lineBrushType;

    RGB888 rgb888;

    // Current pen position
    int penx;
    int peny;

    // Pen position when first call api function
    int beginPenx;
    int beginPeny;

    Subpath_id subpathID;
    LinkList_ptr *shaderInfo; // Store the shaderContainer info.

    // Flag: when this flag is set, will enable antialising
    // if the display device support grayscale display.
    bool antialiasing;
    // ? Flag: whether api function called first time.
    bool apiCalled;
    // Flag: when this flag is set, certain APIs create a new subpath
    // rather than extending the previous one.
    bool needNewSubpath;

    size_t scanLineMin;
    size_t scanLineMax;

    // HINT: For future use
    int fillStyle;
    int strokeStyle;
};
CanvaHandle_ptr newCanva(CanvaHandle_ptr canva);
CanvaHandle_ptr releaseCanva(CanvaHandle_ptr canva);

bool setBeginPoint(CanvaHandle_ptr hdl, int beginPenx, int beginPeny);
void movePen(CanvaHandle_ptr hdl, int penx, int peny);
bool scanLineRangeUpdate(CanvaHandle_ptr hdl, int y);
bool outAreaboundTruncated(int *penx, int *peny);

//* Point
typedef struct Point Point_t;
typedef struct Point *Point_ptr;
struct Point
{
    int x;
    int y;
};
Point_ptr newPoint(Point_ptr pt, int x, int y);

//* Not API Functions.
bool compareET_lineTo(void *data1, void *data2);     // This function called by "void lineTo(CanvaHandle_ptr phd, int x, int y)" only.
bool fill_compareFillNode(void *data1, void *data2); // This function called by "void fill(CanvaHandle_ptr phd)" only.
int parts8EncodeTool(float pointX, float pointY);
void circularConeFiler_writeColor(int x, int y, RGB888 color, float distance);
void tranStrokeLinePosi(int id, int *x, int *y);
void tranbackStrokeLinePosi(int id, int *x, int *y);
void arcDraw(int arcId, int drawX, int drawY, int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b);
bool arcDrawBorderChecker(int arcId, int drawX, int drawY, int drawPointX, int drawPointY);

//* API Functions.
void beginPath(CanvaHandle_ptr hdl);
void moveTo(CanvaHandle_ptr hdl, int x, int y);
void lineTo(CanvaHandle_ptr hdl, int x, int y);

void strokeLine(CanvaHandle_ptr hdl, int x0, int y0, int x1, int y1);
void strokeLineAA(CanvaHandle_ptr hdl, int x0, int y0, int x1, int y1);
void strokeLineAA2(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1);
void stroke(CanvaHandle_ptr hdl);

enum FloodFillRepresent_type
{
    BOUNDARY_COLOR = 0x00,      // Boundary representation color type
    OLD_SEED_POINT_COLOR = 0x01 // Or Interior dot representation color type
};
void floodFill(int type, int seedX, int seedY, RGB888 typeColor, RGB888 fillColor);
enum FillRule_type
{
    NONZERO,
    EVENODD
};
void fill(CanvaHandle_ptr hdl, ...);

void drawCircle(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise);
void drawCircleAA(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise);
void arcInstance(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise, bool antialiasing);
void arc(CanvaHandle_ptr phd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise);
void closePath(CanvaHandle_ptr hdl);

void pathDestory(CanvaHandle_ptr hdl);
void *releaser(void *pt);

#endif