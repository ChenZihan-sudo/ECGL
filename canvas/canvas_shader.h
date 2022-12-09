#ifndef CANVAS_SHADER_H
#define CANVAS_SHADER_H

#include "canvas.h"

//* Shader system

//* Shader status info
extern Subpath_id CurrentSubpathID;
extern uint8_t ShaderStatus;
extern LinkList_ptr *CurrentShaderInfo;
enum ShaderStatus_type
{
    stSTROKE,
    stFILL
};

typedef struct ShaderContainer ShaderContainer_t;
typedef struct ShaderContainer *ShaderContainer_ptr;
struct ShaderContainer
{
    Subpath_id subpathID;
    int x;
    int y;
    int TYPE;
    void *data;
    bool anticlockwise;
};
ShaderContainer_ptr newShaderContainer(int x, int y, bool anticlockwise, int containerType, void *data);
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt);

enum ShaderContainer_type
{
    SPOINT_RGB888,
    SPOINT_RGBA32,
    SLINE,

    // For filling use
    FPOINT,
    FLINE,

    // Need resolve to points when fill
    SARC,
    SROUNDRECT
};

//* Iterator
enum IteratorType
{
    SHADER_INFO = 0x00,
};
typedef struct Iterator Iterator_t;
typedef struct Iterator *Iterator_ptr;
struct Iterator
{
    int iteratorType;
    void *data1;
    void *data2;
    int val1;
    int val2;
    int val3;
    int val4;
    int val5;
};

Iterator_ptr newShaderInfoIterator(CanvaHandle_ptr hdl);
ShaderContainer_ptr nextShaderContainer(Iterator_ptr itor);
int currentShaderInfoItorY(Iterator_ptr itor);
int currentShaderInfoItorYChanged(Iterator_ptr itor);
bool shaderInfoIterateEnd(Iterator_ptr itor);

//* Point RGBA32
typedef struct sPointRGBA32 sPointRGBA32_t;
typedef struct sPointRGBA32 *sPointRGBA32_ptr;
struct sPointRGBA32
{
    bool keyPoint;
    uint8_t alpha;
};
ShaderContainer_ptr newSPointRGBA32(int x, int y, bool keyPoint, uint8_t alpha, bool anticlockwise);

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, int y, bool anticlockwise);

//* Line
typedef struct sLine sLine_t;
typedef struct sLine *sLine_ptr;
struct sLine
{
    int x1;
    int y1;
    bool antialiasing;
};
ShaderContainer_ptr newSLine(int x0, int y0, int x1, int y1, bool antialiasing, bool anticlockwise);

//* Arc
typedef struct sArc sArc_t;
typedef struct sArc *sArc_ptr;
struct sArc
{
    int radius;
    float startAngle;
    float endAngle;
    bool antialiasing;
};
ShaderContainer_ptr newSArc(int x, int y, int radius, float startAngle, float endAngle,
                            bool antialiasing, bool anticlockwise);

//* RoundRect
typedef struct sRoundRect sRoundRect_t;
typedef struct sRoundRect *sRoundRect_ptr;
struct sRoundRect
{
    int width;
    int height;
    int topLeft;
    int topRight;
    int bottomLeft;
    int bottomRight;
    bool antialiasing;
};
ShaderContainer_ptr newSRoundRect(int x, int y, int width, int height, int topLeft,
                                  int topRight, int bottomRight, int bottomLeft, bool antialiasing);

//* For Fill use
typedef struct FillNode FillNode_t;
typedef struct FillNode *FillNode_ptr;
enum FillNode_type
{
    FN_LINE,
    FN_POINT
};
struct FillNode
{
    uint8_t TYPE;

    // For FN_LINE use
    float ax;
    float bx;
    float by;
    float tm;

    // For FN_POINT use
    int x;
    int y;

    bool anticlockwise; // For NONZERO use
};
FillNode_ptr newFillNode_line(float ax, float bx, float by, float tm, bool anticlockwise);
FillNode_ptr newFillNode_point(int x, int y, bool anticlockwise);
void writeFPoint(LinkList_ptr *shaderInfo, int x, int y, bool anticlockwise);
void writeFLine(LinkList_ptr *shaderInfo, int x0, int y0, int x1, int y1, bool anticlockwise);

//* Write to shader buffer API
bool shaderPointCompare(void *data1, void *data2);
void writeSPoint(LinkList_ptr *shaderInfo, int x, int y, bool anticlockwise);
void writeSPointAA(LinkList_ptr *shaderInfo, int x, int y, uint8_t alpha, bool keyPoint, bool anticlockwise);
void writeSLine(LinkList_ptr *shaderInfo, int x0, int y0, int x1, int y1, bool antialiasing, bool anticlockwise);
void writeSArc(LinkList_ptr *shaderInfo, int x, int y, int radius, float startAngle,
               float endAngle, bool antialiasing, bool anticlockwise);
void writeSRoundRect(LinkList_ptr *shaderInfo, int x, int y, int width, int height,
                     int topLeft, int topRight, int bottomRight, int bottomLeft, bool antialiasing);

#endif
