#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "../display/display_config.h"
#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas_shader.h"
#include "canvas.h"
#include "canvas_graphics.h"

// HINT: Not a API Function.
int parts8EncodeTool(float pointX, float pointY)
{
    //* This can be used in arc or strokeline, etc.

    // HINT: How this 8 arc parts be encoded.
    //* Use 3 bits 'ABC' to represent a arc part, the first bit 'A' is !XGreatY, second bit 'B' !positiveX and third bit 'C' !positiveY.
    //* By this 3 bits a arc code can get 8 possiblity and these results are not same. Use this can generate arc code stored in parts8Encode below.
    int parts8Encode[8] = {0, 4, 6, 2, 3, 7, 5, 1};

    bool XGreatY = flGREATE(fabs(pointX), fabs(pointY));
    bool positiveX = flGREATE(pointX, 0.f);
    bool positiveY = flGREATE(pointY, 0.f);

    for (size_t i = 0; i < 8; i++)
    {
        bool comp1 = parts8Encode[i] & (0b100 | 0x00000000);
        bool comp2 = parts8Encode[i] & (0b010 | 0x00000000);
        bool comp3 = parts8Encode[i] & (0b001 | 0x00000000);

        if (!XGreatY == comp1 && !positiveX == comp2 && !positiveY == comp3)
            return i;
    }

    return 0;
}

// HINT: Not a API Function.
// Way of calculation by https://www.academia.edu/30605701/Volume_and_surface_area_of_a_right_circular_cone_cut_by_a_plane_parallel_to_its_symmetrical_axis_Analysis_of_cut_cone_with_a_hyperbolic_section_
void circularConeFiler_writeColor(int x, int y, RGB888 color, float distance)
{
    // * This filter can be used when lineWidth = 1
    // float circularConeFilter[16] = {0.816670, 0.803552, 0.764825, 0.702447, 0.619998, 0.523599, 0.424875, 0.331953, 0.248111, 0.175448, 0.115264, 0.068198, 0.034262, 0.012797, 0.002325, 0.000000};
    uint8_t circularConeFilterHex256[16] = {0xd0, 0xcd, 0xc3, 0xb3, 0x9e, 0x86, 0x6c, 0x55, 0x3f, 0x2d, 0x1d, 0x11, 0x9, 0x3, 0x1};
    // uint8_t circularConeFilterHex256Enhanced[16] = { 0xf5, 0xf1, 0xe5, 0xd3, 0xba, 0x9d, 0x7f, 0x64, 0x4a, 0x35, 0x23, 0x14, 0xa, 0x4, 0x1 };
    uint8_t alphaHex = circularConeFilterHex256[(int)(fabs(distance) * 10.f)];
    IDM_writeAlphaBlendColor(x, y, color, alphaHex);
};

// HINT: Not a API Function.
void tranStrokeLinePosi(int id, int *x, int *y)
{
    int bufx = *x;
    int bufy = *y;
    switch (id)
    {
    case 4:
    {
        *x = bufy;
        *y = bufx;
        break;
    };
    case 6:
    {
        *x = bufy;
        *y = -bufx;
        break;
    };
    case 2:
    {
        *x = -bufx;
        break;
    };
    case 3:
    {
        *x = -bufx;
        *y = -bufy;
        break;
    };
    case 7:
    {
        *x = -bufy;
        *y = -bufx;
        break;
    };
    case 5:
    {
        *x = -bufy;
        *y = bufx;
        break;
    };
    case 1:
    {
        *y = -bufy;
        break;
    };
    }
}

// HINT: Not a API Function.
void tranbackStrokeLinePosi(int id, int *x, int *y)
{
    int bufx = *x;
    int bufy = *y;
    switch (id)
    {
    case 4:
    {
        *x = bufy;
        *y = bufx;
        break;
    };
    case 6:
    {
        *x = -bufy;
        *y = bufx;
        break;
    };
    case 2:
    {
        *x = -bufx;
        break;
    };
    case 3:
    {
        *x = -bufx;
        *y = -bufy;
        break;
    };
    case 7:
    {
        *x = -bufy;
        *y = -bufx;
        break;
    };
    case 5:
    {
        *x = bufy;
        *y = -bufx;
        break;
    };
    case 1:
    {
        *y = -bufy;
        break;
    };
    }
}

// * [1] Stroke line antialiasing algorihtm, better quality but lower effective
// HINT: ??????????????? Reference about antialiasing: ?????????????????????????????????M???????????????????????????????????? 1998???
// Optimise: By http://www.cqvip.com/qk/97969a/200906/30513907.html
// Cone weighted area aliasing. (Use gupta sproull algorithm)
void strokeLineAA(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0, dy = y1 - y0;
    if ((dy <= 0 && dx >= 0) || (dy <= 0 && dx <= 0))
    {
        int bx0 = x0;
        int by0 = y0;
        x0 = x1;
        y0 = y1;
        x1 = bx0;
        y1 = by0;
        dx = x1 - x0;
        dy = y1 - y0;
    }
    float k = (float)dy / (float)dx;
    bool kCck = 0 <= fabs(k) && fabs(k) <= 1;
    int ex = dx, ey = dy;
    int arcEncode[8] = {0, 4, 6, 2, 3, 7, 5, 1};
    int id = arcEncode[parts8EncodeTool((float)dx, (float)dy)];
    tranStrokeLinePosi(id, &ex, &ey);

    RGB888 rgb888 = hd->rgb888;

    int a = 0 - ey;
    int b = ex;
    int x = 0, y = 0;

    int delta1 = 2 * a;
    int delta2 = 2 * (a + b);
    int d = 2 * a + b;
    int t = 0;

    float dval = 1.f / (2.f * sqrt(pow(a, 2.f) + pow(b, 2.f)));
    float q = -2.f * b * dval; // The distance between pixel and line

    int rx, ry;

    rx = x;
    ry = y;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, 0);
    rx = x;
    ry = y + 1;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q);
    rx = x;
    ry = y - 1;
    tranbackStrokeLinePosi(id, &rx, &ry);
    circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q);

    while (x < ex)
    {
        if (d < 0)
        {
            t = d + b;
            y++;
            d += delta2;
        }
        else
        {
            t = d - b;
            d += delta1;
        }
        x++;

        rx = x;
        ry = y;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, t * dval);
        rx = x;
        ry = y + 1;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q - t * dval);
        rx = x;
        ry = y - 1;
        tranbackStrokeLinePosi(id, &rx, &ry);
        circularConeFiler_writeColor(x0 + rx, y0 + ry, rgb888, q + t * dval);
    }

    // TODO: Remove This Test: Line Begin/End Point
    // IDM_writeAlphaBlendColor(x0, y0, 0x00FF00, 0xFF);
    // IDM_writeAlphaBlendColor(x1, y1, 0x00FF00, 0xFF);
}

// * [2] Stroke line antialiasing algorihtm, better quality but lower effective
// Supersampling + Wu Antialiasing algorithm
void strokeLineAA2(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1)
{
    int sizex = 2 * (x1 - x0 + 1) + 1;
    int sizey = 2 * (y1 - y0 + 1) + 1;
    uint32_t **frameBuf = (uint32_t **)calloc(sizey, sizeof(uint32_t *));
    for (size_t i = 0; i < sizey; i++)
    {
        frameBuf[i] = (uint32_t *)calloc(sizex, sizeof(uint32_t));
    }

    int px0 = x0;
    int py0 = y0;
    int px1 = x1;
    int py1 = y1;

    x0 = 0;
    y0 = 0;
    x1 = sizex - 1;
    y1 = sizey - 1;

    // Use Bresenham Algorithm
    int dx = x1 - x0, dy = y1 - y0, x = 0, y = 0;
    float k = (float)dy / (float)dx, e = 0.f;
    for (size_t i = 0; i < dx; i++)
    {
        x++;
        uint8_t a = (1.0 - e) * 0xFF;

        // IDM_writeAlphaBlendColor(x, y, 0xFF0000, (1.0 - e) * 255.0);
        // IDM_writeAlphaBlendColor(x, y + 1, 0xFF0000, 0xFF - a);

        frameBuf[y][x] = a;
        frameBuf[y + 1][x] = 0xFF - a;

        e += k;
        if (e >= 1.f)
        {
            y++;
            e--;
        }
    }

    for (size_t i = py0; i < py1; i++)
    {
        for (size_t j = px0; j < px1; j++)
        {
            uint32_t value = 0;
            value += 1 * frameBuf[2 * i][2 * j];
            value += 2 * frameBuf[2 * i][2 * j + 1];
            value += 1 * frameBuf[2 * i][2 * j + 2];

            value += 2 * frameBuf[2 * i + 1][2 * j];
            value += 4 * frameBuf[2 * i + 1][2 * j + 1];
            value += 2 * frameBuf[2 * i + 1][2 * j + 2];

            value += 1 * frameBuf[2 * i + 2][2 * j];
            value += 2 * frameBuf[2 * i + 2][2 * j + 1];
            value += 1 * frameBuf[2 * i + 2][2 * j + 2];

            value /= 7;
            if (value > 0xFF)
                value = 0xFF;

            IDM_writeAlphaBlendColor(j, i, 0xFF0000, value);
        }
    }
}

// * Stroke line bresenhamal algorihtm
// Stroke vertical/horizontal lines line or pixel style UI recommend.
void strokeLine(CanvaHandle_ptr hd, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0, dy = y1 - y0;
    if ((dy <= 0 && dx >= 0) || (dy <= 0 && dx <= 0))
    {
        int bx0 = x0;
        int by0 = y0;
        x0 = x1;
        y0 = y1;
        x1 = bx0;
        y1 = by0;
        dx = x1 - x0;
        dy = y1 - y0;
    }
    int e, x = x0, y = y0;
    float k = (float)dy / (float)dx;

    bool xCck = dx >= 0;
    bool kCck = 0 <= fabs(k) && fabs(k) <= 1;

    if (!xCck)
        dx = -(x1 - x0);

    if (kCck)
        e = 2 * dy - dx;
    else
        e = 2 * dx - dy;

    size_t i, totalCount;
    if (kCck)
        totalCount = dx;
    else
        totalCount = dy;

    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    bool brush1kCck = -1 <= k && k <= 1;

    for (i = 0; i <= totalCount; i++)
    {
        // if (hd->lineWidth > 1)
        // {
        //     switch (hd->lineBrushType)
        //     {
        //     case 1:
        //     {
        //         int mid = hd->lineWidth / 2;
        //         if (!brush1kCck)
        //         {
        //             for (int i = 0; i < mid; i++)
        //             {
        //                 IDM_writeColor(x + i, y, colorH8b, colorL8b);
        //                 IDM_writeColor(x - i, y, colorH8b, colorL8b);
        //             }
        //         }
        //         else
        //         {
        //             for (int i = 0; i < mid; i++)
        //             {
        //                 IDM_writeColor(x, y + i, colorH8b, colorL8b);
        //                 IDM_writeColor(x, y - i, colorH8b, colorL8b);
        //             }
        //         }
        //         IDM_writeColor(x, y, colorH8b, colorL8b);
        //     }
        //     break;
        //     case 2:
        //     {
        //         int mid = hd->lineWidth / 2;
        //         for (int i = -mid; i < mid + 1; i++)
        //         {
        //             for (int j = -mid; j < mid + 1; j++)
        //             {
        //                 IDM_writeColor(x + j, y + i, colorH8b, colorL8b);
        //                 IDM_writeColor(x + j, y + i, colorH8b, colorL8b);
        //             }
        //         }
        //     }
        //     break;
        //     }
        // }
        // else
        IDM_writeColor(x, y, colorH8b, colorL8b);

        if (e >= 0)
        {
            if (kCck)
            {
                e = e + 2 * dy - 2 * dx;
                y++;
            }
            else
            {
                e = e + 2 * dx - 2 * dy;
                if (xCck)
                    x++;
                else
                    x--;
            }
        }
        else
        {
            if (kCck)
                e = e + 2 * dy;
            else
                e = e + 2 * dx;
        }

        if (kCck)
            if (xCck)
                x++;
            else
                x--;
        else
            y++;
    }

    // TODO: Remove This Test: Line Begin/End Point
    // uint16_t _color = RGB888_to_RGB565(0x00FF00);
    // uint8_t _colorH8b = (_color >> 8) & 0xFF;
    // uint8_t _colorL8b = _color & 0xFF;
    // IDM_writeColor(x0, y0, _colorH8b, _colorL8b);
    // IDM_writeColor(x1, y1, _colorH8b, _colorL8b);
}

// TODO: OPTIMITE THIS CODE AS FOLLOW.

// * High effective flood fill algorithm.
// HINT: Reference: http://www.williammalone.com/articles/html5-canvas-javascript-paint-bucket-tool/
/// @details
/// FloodFillRepresent_type
/// {
///     BOUNDARY_COLOR = 0x00,      // Boundary representation color type
///     OLD_SEED_POINT_COLOR = 0x01 // Or Interior dot representation color type
/// };
/// @param fillType BOUNDARY_COLOR or OLD_SEED_POINT_COLOR
/// @param typeColor Representation type of color, the color of the parameter fillType you set
void floodFill(int fillType, int seedX, int seedY, RGB888 typeColor, RGB888 fillColor)
{
    Stack_ptr st = nullptr;
    // This stack data model can expand capacity automatic,
    // so set capacity whatever depend on yourself.
    st = newStack(st, 100, true);

    Point_ptr pt = nullptr;
    pt = newPoint(pt, seedX, seedY);
    stackPush(st, (void *)pt);

    RGB565 type565Color = RGB888_to_RGB565(typeColor);
    RGB565 fill565Color = RGB888_to_RGB565(fillColor);

    uint8_t fillcolorH8b = (fill565Color >> 8) & 0xFF;
    uint8_t fillcolorL8b = fill565Color & 0xFF;

    while (stackSize(st))
    {
        bool reachLeft = false;
        bool reachRight = false;

        Point_ptr p = (Point_ptr)stackTop(st);
        stackPop(st);

        // write_display_memory_on();
        // printf("%d %d\n", p->x, p->y);

        RGB565 curColor = IDM_readColor_to_RGB565(p->x, p->y);

        while (1)
        {
            bool judge1 = false;
            switch (fillType)
            {
            case BOUNDARY_COLOR:
                judge1 = p->y > 0 && type565Color != curColor;
                break;
            case OLD_SEED_POINT_COLOR:
                judge1 = p->y > 0 && type565Color == curColor;
                break;
            }
            if (!judge1)
                break;

            p->y--;
            curColor = IDM_readColor_to_RGB565(p->x, p->y);
        }

        bool judge2 = false;
        switch (fillType)
        {
        case BOUNDARY_COLOR:
            judge2 = p->y > 0 && type565Color == curColor;
            break;
        case OLD_SEED_POINT_COLOR:
            judge2 = p->y > 0 && type565Color != curColor;
            break;
        }
        if (judge2)
            p->y++;

        curColor = IDM_readColor_to_RGB565(p->x, p->y);

        while (1)
        {
            bool judge3 = false;
            switch (fillType)
            {
            case BOUNDARY_COLOR:
                judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color != curColor && fill565Color != curColor;
                break;
            case OLD_SEED_POINT_COLOR:
                judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color == curColor;
                //? If BUG, try use this: judge3 = p->y < DISPLAY_HEIGHT - 1 && type565Color == curColor && fill565Color != curColor;
                break;
            }
            if (!judge3)
                break;

            IDM_writeColor(p->x, p->y, fillcolorH8b, fillcolorL8b);
            // write_display_memory_on();
            // printf("%d %d Size:%d\n", p->x, p->y, stackSize(st));

            if (p->x > 0)
            {
                curColor = IDM_readColor_to_RGB565(p->x - 1, p->y);

                bool judge4 = false;
                switch (fillType)
                {
                case BOUNDARY_COLOR:
                    judge4 = type565Color != curColor && fill565Color != curColor && !reachLeft;
                    break;
                case OLD_SEED_POINT_COLOR:
                    judge4 = type565Color == curColor && !reachLeft;
                    //? If BUG, try use this: judge4 = type565Color == curColor && fill565Color != curColor && !reachLeft;
                    break;
                }
                if (judge4)
                {
                    Point_ptr pl = nullptr;
                    pl = newPoint(pl, p->x - 1, p->y);
                    stackPush(st, (void *)pl);
                    reachLeft = true;
                }
            }

            if (p->x < DISPLAY_WIDTH - 1)
            {
                curColor = IDM_readColor_to_RGB565(p->x + 1, p->y);

                bool judge5 = false;
                switch (fillType)
                {
                case BOUNDARY_COLOR:
                    judge5 = type565Color != curColor && fill565Color != curColor && !reachRight;
                    break;
                case OLD_SEED_POINT_COLOR:
                    judge5 = type565Color == curColor && !reachRight;
                    //? If BUG, try use this: judge5 = type565Color == curColor && fill565Color != curColor && !reachRight;
                    break;
                }
                if (judge5)
                {
                    Point_ptr pr = nullptr;
                    pr = newPoint(pr, p->x + 1, p->y);
                    stackPush(st, (void *)pr);
                    reachRight = true;
                }
            }

            p->y++;
            curColor = IDM_readColor_to_RGB565(p->x, p->y);
        }

        // p = releasePoint(p);
        p = (Point_ptr)releaser(p);
    }

    st = releaseStack(st);
};

// HINT: Not a API Function. This Function Called by arc() Only.
void arcDrawAA(int arcId, int x0, int y0, int x1, int y1, int x, int y, uint8_t h0, uint8_t h1, RGB888 color)
{
    switch (arcId)
    {
    case 0:
    {
        IDM_writeAlphaBlendColor(y0 + x, x0 + y, color, h0);
        IDM_writeAlphaBlendColor(y1 + x, x1 + y, color, h1);
        break;
    }
    case 4:
    {
        IDM_writeAlphaBlendColor(x0 + x, y0 + y, color, h0);
        IDM_writeAlphaBlendColor(x1 + x, y1 + y, color, h1);
        break;
    }
    case 6:
    {
        IDM_writeAlphaBlendColor(-x0 + x, y0 + y, color, h0);
        IDM_writeAlphaBlendColor(-x1 + x, y1 + y, color, h1);
        break;
    }
    case 2:
    {
        IDM_writeAlphaBlendColor(-y0 + x, x0 + y, color, h0);
        IDM_writeAlphaBlendColor(-x1 + x, y1 + y, color, h1);
        break;
    }
    case 3:
    {
        IDM_writeAlphaBlendColor(-y0 + x, -x0 + y, color, h0);
        IDM_writeAlphaBlendColor(-y1 + x, -x1 + y, color, h1);
        break;
    }
    case 7:
    {
        IDM_writeAlphaBlendColor(-x0 + x, -y0 + y, color, h0);
        IDM_writeAlphaBlendColor(-x1 + x, -y1 + y, color, h1);
        break;
    }
    case 5:
    {
        IDM_writeAlphaBlendColor(x0 + x, -y0 + y, color, h0);
        IDM_writeAlphaBlendColor(x1 + x, -y1 + y, color, h1);
        break;
    }
    case 1:
    {
        IDM_writeAlphaBlendColor(y0 + x, -x0 + y, color, h0);
        IDM_writeAlphaBlendColor(y1 + x, -x1 + y, color, h1);
        break;
    }
    }
}

// HINT: Not a API Function. This Function Called by arc() Only.
void arcDraw(int arcId, int drawX, int drawY, int x, int y, uint8_t colorH8b, uint8_t colorL8b)
{
    switch (arcId)
    {
    case 0:
        IDM_writeColor(drawY + x, drawX + y, colorH8b, colorL8b);
        break;
    case 4:
        IDM_writeColor(drawX + x, drawY + y, colorH8b, colorL8b);
        break;
    case 6:
        IDM_writeColor(-drawX + x, drawY + y, colorH8b, colorL8b);
        break;
    case 2:
        IDM_writeColor(-drawY + x, drawX + y, colorH8b, colorL8b);
        break;
    case 3:
        IDM_writeColor(-drawY + x, -drawX + y, colorH8b, colorL8b);
        break;
    case 7:
        IDM_writeColor(-drawX + x, -drawY + y, colorH8b, colorL8b);
        break;
    case 5:
        IDM_writeColor(drawX + x, -drawY + y, colorH8b, colorL8b);
        break;
    case 1:
        IDM_writeColor(drawY + x, -drawX + y, colorH8b, colorL8b);
        break;
    }
}

// HINT: Not a API Function. This Function Called by arc() Only.
void arcDrawFP(int arcId, int drawX, int drawY, int x, int y, bool anticlockwise)
{
    switch (arcId)
    {
    case 0:
        writeFPoint(CurrentShaderInfo, drawX + x, drawY + y, anticlockwise);
        break;
    case 4:
        writeFPoint(CurrentShaderInfo, drawY + x, drawX + y, anticlockwise);
        break;
    case 6:
        writeFPoint(CurrentShaderInfo, -drawY + x, drawX + y, anticlockwise);
        break;
    case 2:
        writeFPoint(CurrentShaderInfo, -drawX + x, drawY + y, anticlockwise);
        break;
    case 3:
        writeFPoint(CurrentShaderInfo, -drawX + x, -drawY + y, anticlockwise);
        break;
    case 7:
        writeFPoint(CurrentShaderInfo, -drawY + x, -drawX + y, anticlockwise);
        break;
    case 5:
        writeFPoint(CurrentShaderInfo, drawY + x, -drawX + y, anticlockwise);
        break;
    case 1:
        writeFPoint(CurrentShaderInfo, drawX + x, -drawY + y, anticlockwise);
        break;
    }
}

// HINT: Not a API Function. This Function Called by arc() Only.
bool arcDrawBorderChecker(int arcId, int drawX, int drawY, int drawPointX, int drawPointY)
{
    //* Check only x side or y side is not enough and will lose precise, so we need check x and y side both. :)
    switch (arcId)
    {
    case 0:
        return drawY < drawPointX || drawX > drawPointY;
        break;
    case 4: // Based on ID 4
    {
        return drawX < drawPointX || drawY > drawPointY;
        break;
    }
    case 6:
    {
        printf("%d %d %d\n", drawY, drawPointY, drawY < drawPointY);
        printf("|->%d %d %d\n", -drawX, drawPointX, -drawX < drawPointX);
        return -drawX < drawPointX || drawY < drawPointY;
        break;
    }
    case 2:
        return -drawY < drawPointX || drawX < drawPointY;
        break;
    case 3:
        return -drawY > drawPointX || -drawX < drawPointY;
        break;
    case 7:
        return -drawX > drawPointX || -drawY < drawPointY;
        break;
    case 5:
        return drawX > drawPointX || -drawY > drawPointY;
        break;
    case 1:
        return drawY > drawPointX || -drawX > drawPointY;
        break;
    }
    return false;
}

// * Arc (antialiasing) algorithm
// HINT: Antialiasing Reference: http://www.gissky.net/paper/UploadFiles_4495/201207/2012072420494963.pdf
// TODO: Bug exist, example: arc(ctx, 100, 100, 30, 0.55 * PI, 0.65 * PI, false);
void arcInstance(CanvaHandle_ptr hd, int x, int y, int radius, float startAngle, float endAngle, bool anticlockwise, bool antialiasing)
{
    //* We cut the whole circle into 8 parts. And encode this parts by function parts8EncodeTool().
    //* We store this parts code into array arcs which is the parts we need to draw.
    //* If the end point and begin point of arc in the same part, it's possible that we need maximum 9 arc parts to store the part we want to draw,
    //* and in these 9 parts have two same arc parts. The function arcDrawBorderChecker() will check the border of two arc parts.

    bool originanticlockwise = anticlockwise;

    // Swap if anticloskwise
    if (anticlockwise)
    {
        anticlockwise = false;
        float dataBuffer = startAngle;
        startAngle = endAngle;
        endAngle = dataBuffer;
    }

    float startDrawPointX = ((float)radius * cos(startAngle));
    float startDrawPointY = ((float)radius * sin(startAngle));

    float endDrawPointX = ((float)radius * cos(endAngle));
    float endDrawPointY = ((float)radius * sin(endAngle));

    printf("sX %d sY %d eX %d eY %d\n", (int)startDrawPointX,
           (int)startDrawPointY, (int)endDrawPointX, (int)endDrawPointY);

    int arcEncode[8] = {0, 4, 6, 2, 3, 7, 5, 1};
    int sCodePosi = parts8EncodeTool(startDrawPointX, startDrawPointY);
    int eCodePosi = parts8EncodeTool(endDrawPointX, endDrawPointY);
    int posi = sCodePosi;

    bool equalBreak = false;
    float sAngleComp = (startAngle / PI - (float)((int)(startAngle / PI) / 2) * 2.f);
    float eAngleComp = (endAngle / PI - (float)((int)(endAngle / PI) / 2) * 2.f);

    int arcs[9] = {0};
    int arcCount = 0;

    // Judge to draw the circle or arc.
    if (flGREATE(fabs(startAngle - endAngle), 2.f * PI))
    {
        memcpy(arcs, arcEncode, 8 * sizeof(int));
        arcCount = 8;
    }
    else
    {
        for (size_t i = 0; i < 9; i++)
        {
            arcs[i] = arcEncode[posi];
            arcCount++;
            if (posi == eCodePosi)
            {
                if ((sCodePosi == eCodePosi && !equalBreak) &&
                    ((sAngleComp < eAngleComp && anticlockwise) ||
                     (sAngleComp > eAngleComp && !anticlockwise)))
                {
                    equalBreak = true;
                }
                else
                    break;
            }
            posi = posi >= 7 ? 0 : posi + 1;
        }
    }

    // Color
    uint16_t color = RGB888_to_RGB565(hd->rgb888);
    uint8_t colorH8b = (color >> 8) & 0xFF;
    uint8_t colorL8b = color & 0xFF;

    // Draw arc
    int drawX = 0;
    int drawY = radius;
    int d = 1 - radius;

    if (antialiasing)
    {
        // For antialiasing
        int c = 160 - 128 * radius;
        uint8_t h0 = 0xFF, h1 = 0;
        int x0 = 0, y0 = radius;
        int x1 = 0, y1 = radius - 1;

        while (drawX < drawY + 1)
        {
            if (arcCount == 1)
            { // Special case, single one arc part.
                bool checker1 = arcDrawBorderChecker(arcs[0], drawX, drawY, startDrawPointX, startDrawPointY);
                bool checker2 = arcDrawBorderChecker(arcs[0], drawX, drawY, endDrawPointX, endDrawPointY);
                if (checker1 && !checker2)
                    if (ShaderStatus == stSTROKE)
                        arcDrawAA(arcs[0], x0, y0, x1, y1, x, y, h0, h1, hd->rgb888);
                    else // stFILL
                        arcDrawFP(arcs[0], drawX, drawY, x, y, originanticlockwise);
            }
            else
            {
                for (size_t i = 0; i < arcCount; i++)
                {
                    int arcId = arcs[i];
                    if (i == 0)
                    {
                        bool checker1 = arcDrawBorderChecker(arcId, drawX, drawY, startDrawPointX, startDrawPointY);
                        if (!checker1)
                            continue;
                    }
                    else if (i == arcCount - 1)
                    {
                        bool checker2 = arcDrawBorderChecker(arcId, drawX, drawY, endDrawPointX, endDrawPointY);
                        if (checker2)
                            continue;
                    }

                    if (ShaderStatus == stSTROKE)
                        arcDrawAA(arcId, x0, y0, x1, y1, x, y, h0, h1, hd->rgb888);
                    else // stFILL
                        arcDrawFP(arcId, drawX, drawY, x, y, originanticlockwise);
                }
            }

            if (c < 0)
            {
                float mod1 = (c - 16.0) / drawY;
                if (c < 32 - (drawY << 7))
                {
                    // E X+1,Y  G X+1,Y+1
                    h1 = -(uint8_t)mod1 - 128; // G
                    h0 = 0xFF - h1;            // E
                    x0 = drawX + 1;
                    y0 = drawY;
                    x1 = drawX + 1;
                    y1 = drawY + 1;
                }
                else
                {
                    // S X+1,Y-1  E X+1,Y
                    h0 = (uint8_t)mod1 + 128; // S
                    h1 = 0xFF - h0;           // E
                    x0 = drawX + 1;
                    y0 = drawY - 1;
                    x1 = drawX + 1;
                    y1 = drawY;
                }
            }
            else
            {
                float mod2 = (c - 16.0) / (drawY - 1.0);
                if (c < (drawY << 7) - 96)
                {
                    // S X+1,Y-1  E X+1,Y
                    h1 = 128 - (uint8_t)mod2; // E
                    h0 = 0xFF - h1;           // S
                    x0 = drawX + 1;
                    y0 = drawY - 1;
                    x1 = drawX + 1;
                    y1 = drawY;
                }
                else
                {
                    // S X+1,Y-1 T X+1,Y-2
                    h1 = (uint8_t)mod2 - 128; // T
                    h0 = 0xFF - h1;           // S
                    x0 = drawX + 1;
                    y0 = drawY - 1;
                    x1 = drawX + 1;
                    y1 = drawY - 2;
                }
            }

            if (d >= 0)
            {
                int f = 2 * (drawX - drawY) + 5;
                d += f;
                c += f << 7; // c>=0
                drawY--;
            }
            else
            {
                int f = 2 * drawX + 3;
                d += f;
                c += f << 7; // c<0
            }
            drawX++;
        }
    }
    else
    {
        // Use MidPoint algorithm.
        while (drawX < drawY + 1)
        {
            for (size_t i = 0; i < arcCount; i++)
            {
                int arcId = arcs[i];
                if (i == 0)
                {
                    bool checker1 = arcDrawBorderChecker(arcId, drawX, drawY, startDrawPointX, startDrawPointY);
                    if (!checker1)
                        continue;
                }
                else if (i == arcCount - 1)
                {
                    bool checker2 = arcDrawBorderChecker(arcId, drawX, drawY, endDrawPointX, endDrawPointY);
                    if (checker2)
                        continue;
                }

                if (ShaderStatus == stSTROKE)
                    arcDraw(arcId, drawX, drawY, x, y, colorH8b, colorL8b);
                else // stFILL
                    arcDrawFP(arcId, drawX, drawY, x, y, originanticlockwise);
            }

            if (d >= 0)
            {
                d += 2 * (drawX - drawY) + 5;
                drawY--;
            }
            else
            {
                d += 2 * drawX + 3;
            }
            drawX++;
        }
    }
    // TODO: Remove this
    IDM_writeColor(x, y, colorH8b, colorL8b);
}

// * Draw circle
void drawCircle(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise)
{
    arcInstance(hd, x, y, radius, 0.0, 2.0 * PI, anticlockwise, false);
};

// * Draw circle
void drawCircleAA(CanvaHandle_ptr hd, int x, int y, int radius, bool anticlockwise)
{
    arcInstance(hd, x, y, radius, 0.0, 2.0 * PI, anticlockwise, true);
};

// * RoundRect Instance
canvas_err_t roundRectInstance(CanvaHandle_ptr hd, int x, int y,
                               int width, int height, int upperLeft, int upperRight, int lowerRight, int lowerLeft)
{
    // Draw a straight line to the point (x + w ??? upperRight["x"], y).
    strokeLine(hd, x + upperLeft, y, x + width - upperRight, y);
    // Draw an arc to the point (x + w, y + upperRight["y"]).
    arcInstance(hd, x + width - upperRight, y + upperRight, upperRight, 1.5f * PI, 2.f * PI, false, true);
    // Draw a straight line to the point (x + w, y + h ??? lowerRight["y"]).
    strokeLine(hd, x + width, y + upperRight, x + width, y + height - lowerRight);
    // Draw an arc to the point (x + w ??? lowerRight["x"], y + h).
    arcInstance(hd, x + width - lowerRight, y + height - lowerRight, lowerRight, 0.f * PI, 0.5f * PI, false, true);
    // Draw a straight line to the point (x + lowerLeft["x"], y + h).
    strokeLine(hd, x + width - lowerRight, y + height, x + lowerLeft, y + height);
    // Draw an arc to the point (x, y + h ??? lowerLeft["y"]).
    arcInstance(hd, x + lowerLeft, y + height - lowerLeft, lowerLeft, 0.5f * PI, 1.f * PI, false, true);
    // Draw a straight line to the point (x, y + upperLeft["y"]).
    strokeLine(hd, x, y + height - lowerLeft, x, y + upperLeft);
    // Draw an arc to the point (x + upperLeft["x"], y).
    arcInstance(hd, x + upperLeft, y + upperLeft, upperLeft, 1.f * PI, 1.5f * PI, false, true);
    return CANVAS_OK;
};