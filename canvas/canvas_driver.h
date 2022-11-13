#ifndef CANVAS_DRIVER_H
#define CANVAS_DRIVER_H

#include <stdint.h>
#include "../datamodel/data_model.h"
#include "../display/display_config.h"
#include "canvas_config.h"
#include "canvas_err.h"

// 16bits rgb color format
typedef uint16_t RGB565;
// 24bits rgb color format
typedef uint32_t RGB888;
// 24bits rgb color format with 8bit alpha channel
typedef uint32_t RGBA32;

#define RGB565_RED 0xF800
#define RGB565_GREEN 0x07E0
#define RGB565_BLUE 0x001F

#define RGB888_RED 0xFF0000
#define RGB888_GREEN 0x00FF00
#define RGB888_BLUE 0x0000FF

#define RGBA32_RED 0xFF000000
#define RGBA32_GREEN 0x00FF0000
#define RGBA32_BLUE 0x0000FF00
#define RGBA32_ALPHA 0x000000FF

RGB565 RGBRange_to_RGB565(uint16_t MaxRange, uint16_t Red, uint16_t Green, uint16_t Blue);
RGB565 RGB888_to_RGB565(RGB888 rgb888);
RGB888 RGB565_to_RGB888(RGB565 rgb565);
RGBA32 RGB888_to_RGBA32_alphaHex(RGB888 rgb888, uint8_t alpha);
RGBA32 RGB888_to_RGBA32_alphaPercentage(RGB888 rgb888, uint16_t alphaPercentage);
uint8_t percentage_to_256(uint16_t percentage);
RGBA32 alphaBlend(RGB888 background, RGBA32 foreground);

// * MCU Inner Display Memory
#if USE_INNER_DISPLAY_MEMORY
typedef uint8_t IDM_t;
extern int IDM_xBegin;
extern int IDM_xEnd;
extern int IDM_yBegin;
extern int IDM_yEnd;

#if INNER_DISPLAY_MEMORY_FULL_SIZE
extern IDM_t displayMemory[2 * DISPLAY_WIDTH * DISPLAY_HEIGHT];
void write_display_memory_on();
canvas_err_t IDM_writeColor(int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b);
canvas_err_t IDM_writeColorWithAxisInfo(int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b, int coordOrigin, bool axisUpperXLowerY);
canvas_err_t IDM_writeAlphaBlendColor(int x, int y, RGB888 color, uint8_t alphaHex);
canvas_err_t IDM_writeAlphaBlendColorWithAxisInfo(int x, int y, RGB888 color, uint8_t alphaHex, int coordOrigin, bool axisUpperXLowerY);
RGB565 IDM_readColor_to_RGB565(int x, int y);
RGB888 IDM_readColor_to_RGB888(int x, int y);
#else
#ifndef IDM_ROW_TRANSFER_SIZE
#define IDM_ROW_TRANSFER_SIZE 1
#endif
extern IDM_t displayMemory[2 * DISPLAY_WIDTH * IDM_ROW_TRANSFER_SIZE];
#endif
#endif

// * Framework Rate Tester
#if USE_FRAMEWORK_RATE_TESTER
typedef struct FPSTester FPSTester_t;
typedef struct FPSTester *FPSTester_ptr;
struct FPSTester
{
    bool isValid;
    int FPS;
    uint32_t lastTick;
};
FPSTester_ptr newFPSTester(FPSTester_ptr hdl);
int increaseFrameCount(FPSTester_ptr hdl);
FPSTester_ptr releaseFPSTester(FPSTester_ptr hdl);
#endif

// * Canvas Global Variable
extern int lineWidth;

// * Canvas Functions
canvas_err_t setDrawArea(int xBegin, int xEnd, int yBegin, int yEnd);
void canvasTruncateOverflow(bool *truncated, int *x, int *y, int *width, int *height);
canvas_err_t strokeRect(int x, int y, int width, int height, RGB888 hex);
canvas_err_t fillRect(int x, int y, int width, int height, RGB888 hex);

#endif