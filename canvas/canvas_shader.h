#ifndef CANVAS_SHADER_H
#define CANVAS_SHADER_H

#include "canvas.h"

//* Shader system

typedef int PRIORITY_id;
extern PRIORITY_id priority;

typedef struct ShaderContainer ShaderContainer_t;
typedef struct ShaderContainer *ShaderContainer_ptr;
struct ShaderContainer
{
    int x;
    int TYPE;
    void *data;
    PRIORITY_id priority;
};
ShaderContainer_ptr newShaderContainer(int x, int containerType, void *data, PRIORITY_id priority);
ShaderContainer_ptr releaseShaderContainer(ShaderContainer_ptr pt);

enum ShaderContainer_enum
{
    SPOINT_RGB888 = 0x00,
    SPOINT_RGBA32 = 0x01,
    // SLINE = 0x02
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

//* Shader priority data container
typedef struct SPriorityCon SPriorityCon_t;
typedef struct SPriorityCon *SPriorityCon_ptr;
//* Shader priority data container
struct SPriorityCon
{
    int x;
    PRIORITY_id id;
};
SPriorityCon_ptr newSPriorityCon(int x, PRIORITY_id id);

//* Point RGBA32
typedef struct sPointRGBA32 sPointRGBA32_t;
typedef struct sPointRGBA32 *sPointRGBA32_ptr;
struct sPointRGBA32
{
    bool keyPoint;
    uint8_t alpha;
};
ShaderContainer_ptr newSPointRGBA32(int x, bool keyPoint, uint8_t alpha, PRIORITY_id priority);

//* Point RGB888
ShaderContainer_ptr newSPointRGB888(int x, PRIORITY_id priority);

#endif
