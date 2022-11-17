#ifndef CANVAS_SHADER_H
#define CANVAS_SHADER_H

#include "canvas.h"

//* Shader system

typedef struct ShaderContainer ShaderContainer_t;
typedef struct ShaderContainer *ShaderContainer_ptr;
struct ShaderContainer
{
    int x;
    int TYPE;
    void *data;
};
ShaderContainer_ptr newShaderContainer(int x, int containerType, void *data);
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt);

enum ShaderContainer_enum
{
    SPOINT_RGB888 = 0x00,
    SPOINT_RGBA32 = 0x01,
    SLINE = 0x02,

    // Need resolve to points when fill
    SARC = 0x03,
    SROUNDRECT = 0x04,
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
};

Iterator_ptr newShaderInfoIterator(CanvaHandle_ptr hdl);
ShaderContainer_ptr nextShaderInfo(Iterator_ptr itor);
int currentShaderInfoItorY(Iterator_ptr itor);
bool shaderInfoIterateEnd(Iterator_ptr itor);

//* Point RGBA32
typedef struct sPointRGBA32 sPointRGBA32_t;
typedef struct sPointRGBA32 *sPointRGBA32_ptr;
struct sPointRGBA32
{
    bool keyPoint;
    uint8_t alpha;
};
ShaderContainer_ptr newSPointRGBA32(int x, bool keyPoint, uint8_t alpha);

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x);

//* Line
typedef struct sLine sLine_t;
typedef struct sLine *sLine_ptr;
struct sLine
{
    int x1;
    int y1;
    bool antialiasing;
};
ShaderContainer_ptr newSLine(int x0, int x1, int y1, bool antialiasing);

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
ShaderContainer_ptr newSArc(int x, int radius, float startAngle, float endAngle, bool anticlockwise, bool antialiasing);

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
ShaderContainer_ptr newSRoundRect(int x, int width, int height,
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
