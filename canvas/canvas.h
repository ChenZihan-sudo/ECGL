#ifndef CANVAS_H
#define CANVAS_H

#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas_config.h"
#include "canvas_err.h"

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

struct ArcAngles
{
    float angleBegin;
    float angleEnd;
};
typedef struct ArcAngles ArcAngles_t;
typedef struct ArcAngles *ArcAngles_ptr;
ArcAngles_t transformArcBeginEndPointToAngle(int beginPointX, int beginPointY, int endPointX, int endPointY, int radius);

//* API Functions.
void beginPath(CanvaHandle_ptr hdl);
void moveTo(CanvaHandle_ptr hdl, int x, int y);
void lineTo(CanvaHandle_ptr hdl, int x, int y);
void arcTo(CanvaHandle_ptr hd, int x1, int y1, int x2, int y2, int radius);
void arc(CanvaHandle_ptr phd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise);
canvas_err_t roundRect(CanvaHandle_ptr hd, int x, int y, int width, int height, int radiiCount, ...);
void closePath(CanvaHandle_ptr hdl);

void stroke(CanvaHandle_ptr hdl);
enum FillRule_type
{
    NONZERO,
    EVENODD
};
void fill(CanvaHandle_ptr hdl, ...);

void pathDestory(CanvaHandle_ptr hdl);
void *releaser(void *pt);

#endif