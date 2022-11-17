#ifndef CANVAS_SHADER_H
#define CANVAS_SHADER_H

#include "canvas.h"

//* Shader system

typedef struct ShaderContainer ShaderContainer_t;
typedef struct ShaderContainer *ShaderContainer_ptr;
struct ShaderContainer
{
    int x;
    int y;
    int TYPE;
    void *data;
    
    // For FPOINT use
    bool filled;
};
ShaderContainer_ptr newShaderContainer(int x, int y, int containerType, void *data);
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt);

enum ShaderContainer_enum
{
    SPOINT_RGB888,
    SPOINT_RGBA32,
    SLINE,
    FPOINT, // For filling use

    // Need resolve to points when fill
    SARC,
    SROUNDRECT,
};

//* For Fill use
typedef struct FillNode FillNode_t;
typedef struct FillNode *FillNode_ptr;
struct FillNode
{
    // For line use
    float ax;
    float bx;
    float by;
    float tm;

    // For point use
    int x;
    int y;
};
FillNode_ptr newFillNode_line(float ax, float bx, float by, float tm);
FillNode_ptr newFillNode_point(int x, int y);
void writeFPoint(LinkList_ptr *shaderInfo, int x, int y);

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
};

Iterator_ptr newShaderInfoIterator(CanvaHandle_ptr hdl);
ShaderContainer_ptr nextShaderInfo(Iterator_ptr itor);
int currentShaderInfoItorY(Iterator_ptr itor);
bool shaderInfoIterateEnd(Iterator_ptr itor);

// HINT 可以优化的: 减少点集的访问内存次数,ShaderContainer使用union保存RGBA32点
//* Point RGBA32
typedef struct sPointRGBA32 sPointRGBA32_t;
typedef struct sPointRGBA32 *sPointRGBA32_ptr;
struct sPointRGBA32
{
    bool keyPoint;
    uint8_t alpha;
};
ShaderContainer_ptr newSPointRGBA32(int x, int y, bool keyPoint, uint8_t alpha);

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, int y);

//* Line
typedef struct sLine sLine_t;
typedef struct sLine *sLine_ptr;
struct sLine
{
    int x1;
    int y1;
    bool antialiasing;
};
ShaderContainer_ptr newSLine(int x0, int y0, int x1, int y1, bool antialiasing);

//* Arc
typedef struct sArc sArc_t;
typedef struct sArc *sArc_ptr;
struct sArc
{
    int radius;
    float startAngle;
    float endAngle;
    bool anticlockwise;
    bool antialiasing;
};
ShaderContainer_ptr newSArc(int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise, bool antialiasing);

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
ShaderContainer_ptr newSRoundRect(int x, int y, int width, int height,
                                  int topLeft, int topRight, int bottomRight, int bottomLeft, bool antialiasing);

//* Write to shader buffer API
bool shaderPointCompare(void *data1, void *data2);
void writeSPoint(LinkList_ptr *shaderInfo, int x, int y);
void writeSPointAA(LinkList_ptr *shaderInfo, int x, int y, uint8_t alpha, bool keyPoint);
void writeSLine(LinkList_ptr *shaderInfo, int x0, int y0, int x1, int y1, bool antialiasing);
void writeSArc(LinkList_ptr *shaderInfo, int x, int y, int radius,
               float startAngle, float endAngle, bool anticlockwise, bool antialiasing);
void writeSRoundRect(LinkList_ptr *shaderInfo, int x, int y, int width, int height,
                     int topLeft, int topRight, int bottomRight, int bottomLeft, bool antialiasing);

#endif
