#ifndef CANVAS_DRIVER_CPP
#define CANVAS_DRIVER_CPP

// #ifdef __cplusplus
// extern "C"
//{
// #endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "../display/display_config.h"
#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas.h"

#include "../simECGL/simECGL.h"

// #ifdef __cplusplus
// }
// #endif

#ifndef __cplusplus
#define true 1
#define false 0
#define nullptr NULL
#define bool short
#endif

// * Color format conversion

RGB565 RGBRange_to_RGB565(uint16_t MaxRange, uint16_t Red, uint16_t Green, uint16_t Blue)
{
    MaxRange++;
    Red = Red > MaxRange ? MaxRange : Red;
    Green = Green > MaxRange ? MaxRange : Green;
    Blue = Blue > MaxRange ? MaxRange : Blue;

    uint16_t red = ((1 << 5) * Red / MaxRange);
    uint16_t green = ((1 << 6) * Green / MaxRange);
    uint16_t blue = ((1 << 5) * Blue / MaxRange);
    uint16_t hex = ((red << 11) & 0xFFFF) |
                   ((green << 5) & 0xFFFF) |
                   (blue & 0xFFFF);
    return hex;
}

// RGB888 to RGB565
RGB565 RGB888_to_RGB565(RGB888 rgb888)
{
    uint16_t Red = (rgb888 & RGB888_RED) >> 19;
    uint16_t Green = (rgb888 & RGB888_GREEN) >> 10;
    uint16_t Blue = (rgb888 & RGB888_BLUE) >> 3;
    uint16_t rgb565 = (Red << 11) | (Green << 5) | Blue;
    return rgb565;
}

// RGB565 to RGB888
RGB888 RGB565_to_RGB888(RGB565 rgb565)
{
    uint32_t Red = (rgb565 & RGB565_RED) >> 8;
    uint32_t Green = (rgb565 & RGB565_GREEN) >> 3;
    uint32_t Blue = (rgb565 & RGB565_BLUE) << 3;
    uint32_t rgb888 = (Red << 16) | (Green << 8) | Blue;
    return rgb888;
};

RGBA32 RGB888_to_RGBA32_alphaHex(RGB888 rgb888, uint8_t alphaHex)
{
    RGBA32 rgba32 = (rgb888 << 8) | alphaHex;
    return rgba32;
}

RGBA32 RGB888_to_RGBA32_alphaPercentage(RGB888 rgb888, uint16_t alphaPercentage)
{
    alphaPercentage = ((1 << 8) * alphaPercentage / 100);
    if (alphaPercentage == 256)
        alphaPercentage--;
    RGBA32 rgba32 = (rgb888 << 8) | alphaPercentage;
    return rgba32;
}

uint8_t percentage_to_256(uint16_t percentage)
{
    percentage = ((1 << 8) * percentage / 100);
    if (percentage == 256)
        percentage--;
    return (uint8_t)percentage;
}

RGB888 alphaBlend(RGB888 background, RGBA32 foreground)
{
    int alphaWeight = foreground & RGBA32_ALPHA;

    uint8_t bgRed = (background & RGB888_RED) >> 16;
    uint8_t bgGreen = (background & RGB888_GREEN) >> 8;
    uint8_t bgBlue = (background & RGB888_BLUE);

    uint8_t fgRed = (foreground & RGBA32_RED) >> 24;
    uint8_t fgGreen = (foreground & RGBA32_GREEN) >> 16;
    uint8_t fgBlue = (foreground & RGBA32_BLUE) >> 8;

    uint32_t Red = (alphaWeight * fgRed + (256 - alphaWeight) * bgRed) >> 8;
    uint32_t Green = (alphaWeight * fgGreen + (256 - alphaWeight) * bgGreen) >> 8;
    uint32_t Blue = (alphaWeight * fgBlue + (256 - alphaWeight) * bgBlue) >> 8;

    uint32_t rgb888 = (Red << 16) | (Green << 8) | Blue;

    return rgb888;
}

// * MCU Inner Display Memory
#if USE_INNER_DISPLAY_MEMORY
int IDM_xBegin;
int IDM_xEnd;
int IDM_yBegin;
int IDM_yEnd;

#if INNER_DISPLAY_MEMORY_FULL_SIZE
const size_t sizeofDisplayMemory = 2 * DISPLAY_WIDTH * DISPLAY_HEIGHT;
IDM_t displayMemory[sizeofDisplayMemory] = {0};
// Changed by simECGL
void write_display_memory_on()
{
    simECGL_glWindowDisplay();
}

canvas_err_t IDM_writeColor(int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b)
{
    size_t addr = 2 * ((IDM_yBegin + y) * DISPLAY_WIDTH + (IDM_xBegin + x));
    if (addr >= sizeofDisplayMemory)
        return CANVAS_IDM_OUT_AREA_WRITE;
    displayMemory[addr] = colorHigh8b;
    displayMemory[addr + 1] = colorLow8b;
    return CANVAS_OK;
}

// canvas_err_t IDM_writeColor(int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b)
// {
//     int aY = DISPLAY_HEIGHT - y;
//     IDM_writeColorIn(x, aY, colorHigh8b, colorLow8b);
//     return CANVAS_OK;
// }

/*
 * The method for setting the coordinate origin is as follows.
 * set axisUpperXLowerY 0 if upper is Y axis and lower is X axis.
 * Set axisUpperXLowerY 1 if upper is X axis and lower is Y axis.
 * (coordOrigin)0· --------------- ·3(coordOrigin)
 *                 ---------------
 *                 ----DISPLAY----
 *                 ---------------
 * (coordOrigin)1· --------------- ·2(coordOrigin)
 */
//* Adjust the origin point of coordinate and the direction of x and y axis
canvas_err_t IDM_writeColorWithAxisInfo(int x, int y, uint8_t colorHigh8b, uint8_t colorLow8b,
                                        int coordOrigin, bool axisUpperXLowerY)
{
    switch (coordOrigin)
    {
    case 0:
    {
        switch (axisUpperXLowerY)
        {
        case true:
        {
            return IDM_writeColor(x, y, colorHigh8b, colorLow8b);
        }
        break;
        case false:
        {
            return IDM_writeColor(y, x, colorHigh8b, colorLow8b);
        }
        break;
        }
    }
    break;
    case 1:
    {
        int aY = DISPLAY_HEIGHT - y;
        switch (axisUpperXLowerY)
        {
        case true:
        {
            return IDM_writeColor(aY, x, colorHigh8b, colorLow8b);
        }
        break;
        case false:
        {
            return IDM_writeColor(x, aY, colorHigh8b, colorLow8b);
        }
        break;
        }
    }
    break;
    case 2:
    {
        int aX = DISPLAY_WIDTH - x;
        int aY = DISPLAY_HEIGHT - y;
        switch (axisUpperXLowerY)
        {
        case true:
        {
            return IDM_writeColor(aY, aX, colorHigh8b, colorLow8b);
        }
        break;
        case false:
        {
            return IDM_writeColor(aX, aY, colorHigh8b, colorLow8b);
        }
        break;
        }
    }
    break;
    case 3:
    {
        int aX = DISPLAY_WIDTH - x;
        switch (axisUpperXLowerY)
        {
        case true:
        {
            return IDM_writeColor(aX, y, colorHigh8b, colorLow8b);
        }
        break;
        case false:
        {
            return IDM_writeColor(y, aX, colorHigh8b, colorLow8b);
        }
        break;
        }
    }
    break;
    }

    return CANVAS_FAIL;
}

canvas_err_t IDM_writeAlphaBlendColor(int x, int y, RGB888 color, uint8_t alphaHex)
{
    RGB888 rgb888 = alphaBlend(IDM_readColor_to_RGB888(x, y), RGB888_to_RGBA32_alphaHex(color, alphaHex));
    uint16_t rgb565 = RGB888_to_RGB565(rgb888);
    uint8_t colorH8b = (rgb565 >> 8) & 0xFF;
    uint8_t colorL8b = rgb565 & 0xFF;
    return IDM_writeColor(x, y, colorH8b, colorL8b);
};

canvas_err_t IDM_writeAlphaBlendColorWithAxisInfo(int x, int y, RGB888 color, uint8_t alphaHex, int coordOrigin, bool axisUpperXLowerY)
{
    RGB888 rgb888 = alphaBlend(IDM_readColor_to_RGB888(x, y), RGB888_to_RGBA32_alphaHex(color, alphaHex));
    uint16_t rgb565 = RGB888_to_RGB565(rgb888);
    uint8_t colorH8b = (rgb565 >> 8) & 0xFF;
    uint8_t colorL8b = rgb565 & 0xFF;
    return IDM_writeColorWithAxisInfo(x, y, colorH8b, colorL8b, coordOrigin, axisUpperXLowerY);
};

RGB565 IDM_readColor_to_RGB565(int x, int y)
{
    RGB565 rgb565 = 0;
    size_t addr = 2 * ((IDM_yBegin + y) * DISPLAY_WIDTH + (IDM_xBegin + x));

    if (addr < sizeofDisplayMemory)
    {
        uint8_t colorHigh8b = displayMemory[addr];
        uint8_t colorLow8b = displayMemory[addr + 1];
        rgb565 = ((uint16_t)colorHigh8b) << 8 | ((uint16_t)colorLow8b) << 0;
    }
    else
        printf("[CANVAS] IDM_readColor_to_RGB565: CANVAS_IDM_OUT_AREA_READ\n");

    return rgb565;
};

RGB888 IDM_readColor_to_RGB888(int x, int y)
{
    RGB888 rgb888 = 0;
    size_t addr = 2 * ((IDM_yBegin + y) * DISPLAY_WIDTH + (IDM_xBegin + x));
    if (addr < sizeofDisplayMemory)
    {
        uint8_t colorHigh8b = displayMemory[addr];
        uint8_t colorLow8b = displayMemory[addr + 1];
        RGB565 rgb565 = ((uint16_t)colorHigh8b) << 8 | ((uint16_t)colorLow8b) << 0;
        rgb888 = RGB565_to_RGB888(rgb565);
    }
    else
        printf("[CANVAS] IDM_readColor_to_RGB888: CANVAS_IDM_OUT_AREA_READ\n");
    return rgb888;
};

#else
IDM_t displayMemory[2 * DISPLAY_WIDTH * IDM_ROW_TRANSFER_SIZE] = {0};
#endif
#endif

// * Framework Rate Tester
#if USE_FRAMEWORK_RATE_TESTER
FPSTester_ptr newFPSTester(FPSTester_ptr hdl)
{
    // if (hdl == nullptr)
    //{
    //     hdl = (FPSTester_ptr)malloc(sizeof(FPSTester_t));
    //     hdl->isValid = false;
    //     hdl->lastTick = xTaskGetTickCount();
    //     hdl->FPS = 0;
    // }

    return hdl;
};

int increaseFrameCount(FPSTester_ptr hdl)
{
    // if (hdl->isValid)
    //{
    //     hdl->isValid = false;
    //     hdl->lastTick = xTaskGetTickCount();
    //     hdl->FPS = 0;
    // }

    // hdl->FPS++;

    // if ((xTaskGetTickCount() - hdl->lastTick) * portTICK_PERIOD_MS >= 1000)
    //{
    //     hdl->isValid = true;
    //     return hdl->FPS;
    // }

    return -1;
};

FPSTester_ptr releaseFPSTester(FPSTester_ptr hdl)
{
    if (hdl != nullptr)
        free(hdl);
    return (hdl = nullptr);
}
#endif

// * Canvas Basic Functions
int lineWidth = 1;
canvas_err_t setDrawArea(int xBegin, int xEnd, int yBegin, int yEnd)
{
    bool areaTruncated = false;

    if (xBegin >= DISPLAY_WIDTH)
    {
        xBegin = DISPLAY_WIDTH - 1;
        areaTruncated = true;
    }
    if (xEnd >= DISPLAY_WIDTH)
    {
        xEnd = DISPLAY_WIDTH - 1;
        areaTruncated = true;
    }

    if (yBegin >= DISPLAY_HEIGHT)
    {
        yBegin = DISPLAY_HEIGHT - 1;
        areaTruncated = true;
    }
    if (yEnd >= DISPLAY_HEIGHT)
    {
        yEnd = DISPLAY_HEIGHT - 1;
        areaTruncated = true;
    }

#if USE_INNER_DISPLAY_MEMORY
    IDM_xBegin = xBegin;
    IDM_xEnd = xEnd;
    IDM_yBegin = yBegin;
    IDM_yEnd = yEnd;
#else
    spi_master_write_command(COLUMN_ADDRESS_SET);
    spi_master_write_address(xBegin, xEnd);

    spi_master_write_command(ROW_ADDRESS_SET);
    spi_master_write_address(yBegin, yEnd);
#endif

    if (areaTruncated)
        return CANVAS_AREA_TRUNCATED;
    return CANVAS_OK;
}

void canvasTruncateOverflow(bool *truncated, int *x, int *y, int *width, int *height)
{
    *truncated = false;

    if (*x >= DISPLAY_WIDTH)
    {
        *x = DISPLAY_WIDTH - 1;
        *truncated = true;
    }
    if (*x + *width >= DISPLAY_WIDTH)
    {
        *width = DISPLAY_WIDTH - 1 - *x;
        *truncated = true;
    }

    if (*y >= DISPLAY_HEIGHT)
    {
        *y = DISPLAY_HEIGHT - 1;
        *truncated = true;
    }
    if (*y + *height >= DISPLAY_HEIGHT)
    {
        *height = DISPLAY_HEIGHT - 1 - *y;
        *truncated = true;
    }
}

canvas_err_t strokeRect(int x, int y, int width, int height, RGB888 hex)
{
    bool areaTruncated;
    canvasTruncateOverflow(&areaTruncated, &x, &y, &width, &height);
    setDrawArea(x, x + width, y, y + height);

    uint16_t color = RGB888_to_RGB565(hex);
#if !USE_INNER_DISPLAY_MEMORY
    uint8_t *data = (uint8_t *)malloc(2 * (width * height) * sizeof(uint8_t));
    size_t dataCount = 0;
#endif

    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            if (i < lineWidth || j < lineWidth || j >= width - lineWidth || i >= height - lineWidth)
            {
#if USE_INNER_DISPLAY_MEMORY
#if INNER_DISPLAY_MEMORY_FULL_SIZE
                size_t addr = 2 * ((IDM_yBegin + i) * DISPLAY_WIDTH + (IDM_xBegin + j));
                displayMemory[addr] = (color >> 8) & 0xFF;
                displayMemory[addr + 1] = color & 0xFF;
#else
#endif
#else
                data[dataCount++] = (color >> 8) & 0xFF;
                data[dataCount++] = color & 0xFF;
#endif
            }
        }
    }

#if !USE_INNER_DISPLAY_MEMORY
    display_memory_write_mode();
    spi_master_write_dataset(data, 2 * width * height);
    free(data);
#endif

    if (areaTruncated)
        return GRAPH_AREA_TRUNCATED;

    return CANVAS_OK;
}

canvas_err_t fillRect(int x, int y, int width, int height, RGB888 hex)
{
    bool areaTruncated;
    canvasTruncateOverflow(&areaTruncated, &x, &y, &width, &height);
    setDrawArea(x, x + width, y, y + height);

    uint16_t color = RGB888_to_RGB565(hex);
#if !USE_INNER_DISPLAY_MEMORY
    uint8_t *data = (uint8_t *)malloc(2 * (width * height) * sizeof(uint8_t));
    size_t dataCount = 0;
#endif

    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
#if USE_INNER_DISPLAY_MEMORY
#if INNER_DISPLAY_MEMORY_FULL_SIZE
            size_t addr = 2 * ((IDM_yBegin + i) * DISPLAY_WIDTH + (IDM_xBegin + j));
            displayMemory[addr] = (color >> 8) & 0xFF;
            displayMemory[addr + 1] = color & 0xFF;
#else
#endif
#else
            data[dataCount++] = (color >> 8) & 0xFF;
            data[dataCount++] = color & 0xFF;
#endif
        }
    }

#if !USE_INNER_DISPLAY_MEMORY
    display_memory_write_mode();
    spi_master_write_dataset(data, 2 * width * height);
    free(data);
#endif

    if (areaTruncated)
        return GRAPH_AREA_TRUNCATED;

    return CANVAS_OK;
}

#endif // !CANVAS_DRIVER_CPP