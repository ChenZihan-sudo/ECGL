#ifndef CANVAS_SHADER_H
#define CANVAS_SHADER_H

#include "canvas.h"

//* Shader system

typedef size_t PRIORITY_id;
extern PRIORITY_id priority;

typedef struct ShaderContainer ShaderContainer_t;
typedef struct ShaderContainer *ShaderContainer_ptr;
struct ShaderContainer
{
    int x, y;
    int TYPE;
    void *data;
    PRIORITY_id priority;
};
ShaderContainer_ptr newShaderContainer(int x, int y, int containerType, void *data, PRIORITY_id priority);
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt);

enum ShaderContainer_enum
{
    SPOINT_RGB888 = 0x00,
    SPOINT_RGBA32 = 0x01,
    SLINE = 0x02
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
ShaderContainer_ptr newSPointRGBA32(int x, int y, bool keyPoint, uint8_t alpha, PRIORITY_id priority);

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, int y, PRIORITY_id priority);

//* Line
typedef struct sLine sLine_t;
typedef struct sLine *sLine_ptr;
struct sLine
{
    int x1;
    int y1;
    bool antialiasing;
};
ShaderContainer_ptr newSLine(int x0, int y0, int x1, int y1, bool antialiasing, PRIORITY_id priority);

#endif
