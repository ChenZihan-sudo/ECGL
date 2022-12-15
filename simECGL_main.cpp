#include "GLFW/glfw3.h"
#include <iostream>
#include "canvas/canvas_driver.h"
#include "canvas/canvas.h"
#include "canvas/canvas_config.h"
#include "canvas/canvas_shader.h"
#include "display/display_config.h"
#include "canvas/canvas_graphics.h"
#include "simECGL/simECGL.h"
#include <time.h>

int simECGL_setup();
int simECGL_main();

int main(void)
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(mainECGLWindow))
    {
        simECGL_setup();
        glDrawPixels(DP_WIDTH, DP_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glImage);
        glfwSwapBuffers(mainECGLWindow); // Swap front and back buffers
        glfwPollEvents();                // Poll for and process events
    }

    glfwTerminate();

    return 0;
}

int simECGL_setup()
{
    if (LoadCurrentFrame)
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i(-1, -1);

        //! Custom one. Show border.
        strokeRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0xFF0000);

        simECGL_main();

        LoadCurrentFrame = false;
    }
    return 0;
};

int simECGL_main()
{
    clock_t start, stop;

    CanvaHandle_ptr ctx = nullptr;
    ctx = newCanva(ctx);

    beginPath(ctx);
    fillRect(250, 0, 500, 500, 0x888888);
    fillRect(0, 0, 250, 500, 0x222222);

    ctx->rgb888 = 0xFF0000;
    // arc(ctx, 100, 100, 20, 0.f * PI, 1.7 * PI, false);
    // roundRect(ctx, 100, 200, 100, 30, 1, 10);
    // stroke(ctx);
    // drawCircleAA(ctx, 100, 200, 20, true);
    // write_display_memory_on();

    moveTo(ctx, 100, 150);
    lineTo(ctx, 200, 100);
    lineTo(ctx, 50, 400);

    moveTo(ctx, 100, 150);
    arcTo(ctx, 200, 100, 50, 400, 30);

    // arc(ctx, 300, 300, 30, 0.012496 * PI, 0.791154 * PI, false);

    stroke(ctx);
    //fill(ctx);

    write_display_memory_on();

    Iterator_ptr itor = newShaderInfoIterator(ctx);
    int curY;
    ShaderContainer_ptr scon = nullptr;
    while (!shaderInfoIterateEnd(itor))
    {
        curY = currentShaderInfoItorY(itor);
        scon = nextShaderContainer(itor);
        printf("Y:%d\n", curY);
        if (scon != nullptr)
        {
            switch (scon->TYPE)
            {
            case SPOINT_RGB888:
            {
                printf("SPOINT_RGB888\n");
            }
            case SPOINT_RGBA32:
            {
                printf("SPOINT_RGBA32\n");
            }
            break;
            case FPOINT:
            {
                printf("FPOINT\n");
            }
            break;
            case SLINE:
            {
                printf("SLINE\n");
                sLine_ptr sli = (sLine_ptr)scon->data;
                printf("LINE: %d %d %d %d\n", scon->x, scon->y, sli->x1, sli->y1);
            }
            break;
            case SARC:
            {
                printf("SARC\n");
            }
            break;
            case SROUNDRECT:
            {
                printf("SROUNDRECT\n");
            }
            break;
            }
        }
    }

    printf("---------------------------\n");

    return 0;
}

// int simECGL_main()
// {
//     clock_t start, stop;

//     CanvaHandle_ptr ctx = nullptr;
//     ctx = newCanva(ctx);

//     beginPath(ctx);
//     fillRect(0, 0, 200, 499, 0xFFFFFF);

//     //!!! Useful
//     moveTo(ctx, 100, 100);
//     lineTo(ctx, 200, 200);
//     lineTo(ctx, 300, 200);
//     lineTo(ctx, 250, 150);
//     lineTo(ctx, 150, 300);
//     // arc(ctx, 100, 200, 30, 0.f * PI, 1.f * PI, false);
//     // lineTo(ctx, 100, 100);
//     closePath(ctx);
//     // lineTo(ctx, 70, 200);
//     // lineTo(ctx, 150, 300);
//     arc(ctx, 40, 200, 30, 0.f * PI, 2.f * PI, false);

//     roundRect(ctx, 200, 300, 100, -100, 4, 10, 80, 30, 100);

//     // strokeLineAA2(ctx, 0, 0, 100, 60);
//     // ctx->rgb888 = 0xFF0000;
//     // strokeLineAA(ctx, 0, 100, 100, 160);

//     ShaderStatus = stSTROKE;

//     // start = clock();
//     // drawCircleAA(ctx, 100, 100, 50, true);
//     // stop = clock();
//     // double duration = ((double)(stop - start)) / CLK_TCK;
//     // printf("%f\n", duration);

//     // arc(ctx, 100, 200, 30, 0.f * PI, 2.f * PI, false);
//     ctx->antialiasing = false;

//     ctx->rgb888 = 0x0000FF;
//     fill(ctx, EVENODD);

//     stroke(ctx);

//     // Iterator_ptr itor = newShaderInfoIterator(ctx);
//     // int curY;
//     // ShaderContainer_ptr scon = nullptr;
//     // while (!shaderInfoIterateEnd(itor))
//     // {
//     //     curY = currentShaderInfoItorY(itor);
//     //     scon = nextShaderContainer(itor);
//     //     printf("Y:%d\n", curY);
//     //     if (scon != nullptr)
//     //     {
//     //         switch (scon->TYPE)
//     //         {
//     //         case SPOINT_RGB888:
//     //         {
//     //             printf("SPOINT_RGB888\n");
//     //         }
//     //         case SPOINT_RGBA32:
//     //         {
//     //             printf("SPOINT_RGBA32\n");
//     //         }
//     //         break;
//     //         case FPOINT:
//     //         {
//     //             printf("FPOINT\n");
//     //         }
//     //         break;
//     //         case SLINE:
//     //         {
//     //             printf("SLINE\n");
//     //             sLine_ptr sli = (sLine_ptr)scon->data;
//     //             printf("LINE: %d %d %d %d\n", scon->x, scon->y, sli->x1, sli->y1);
//     //         }
//     //         break;
//     //         case SARC:
//     //         {
//     //             printf("SARC\n");
//     //         }
//     //         break;
//     //         case SROUNDRECT:
//     //         {
//     //             printf("SROUNDRECT\n");
//     //         }
//     //         break;
//     //         }
//     //     }
//     // }

//     // printf("---------------------------\n");

//     write_display_memory_on();

//     return 0;
// }

// moveTo(ctx, 0, 0);
// lineTo(ctx, 10, 10);
// int radiiq = 10;
// roundRect(ctx, 100, 100, 150, 100, 1, &radiiq);

// lineTo(ctx, 100, 50);
// lineTo(ctx, 50, 150);
// stroke(ctx);

// moveTo(ctx, 250, 50);
// lineTo(ctx, 300, 90);
// lineTo(ctx, 300, 100);
// lineTo(ctx, 250, 100);
// lineTo(ctx, 250, 50);
// arc(ctx, 100, 200, 30, 0.f * PI, 1.5f * PI, false);
// ctx->rgb888 = 0xFF0000;

// ctx->rgb888 = 0x00FF00;
// stroke(ctx);

// moveTo(ctx, 150, 40);
// lineTo(ctx, 50, 80);
// lineTo(ctx, 100, 90);
// lineTo(ctx, 100, 100);
// lineTo(ctx, 150, 90);
// closePath(ctx);
// ctx->rgb888 = 0x00FF00;
// stroke(ctx);
// ctx->rgb888 = 0xFF0000;
// fill(ctx);
// write_display_memory_on();

// ctx->rgb888 = 0xFFFFFF;
// int radii[4] = {0, 10, 10, 10};
// roundRect(ctx, 150, 350, 30, 30, 4, radii);
// stroke(ctx);
// write_display_memory_on();
// floodFill(OLD_SEED_POINT_COLOR, 255, 55, 0x000000, 0x00FF00);

// ctx->rgb888 = 0xFF00FF;
// strokeLineAA(ctx, 50, 50, 100, 150);
// write_display_memory_on();

// strokeLine(ctx, 80, 50, 130, 150);
// write_display_memory_on();

// moveTo(ctx, 100, 100);
// lineTo(ctx, 200, 200);
// lineTo(ctx, 300, 200);
// lineTo(ctx, 250, 150);
// lineTo(ctx, 150, 300);
// arc(ctx, 100, 200, 30, 0.f * PI, 1.f * PI, false);
// closePath(ctx);

// moveTo(ctx, 100, 200);
// lineTo(ctx, 150, 200);
// lineTo(ctx, 200, 100);
// lineTo(ctx, 250, 200);
// lineTo(ctx, 300, 200);
// lineTo(ctx, 200, 300);
// closePath(ctx);

// 五角星
//  moveTo(ctx, 10, 70);
//  lineTo(ctx, 130, 70);
//  lineTo(ctx, 40, 10);
//  lineTo(ctx, 70, 130);
//  lineTo(ctx, 100, 10);
//  lineTo(ctx, 10, 70);

// moveTo(ctx, 100, 100);
// lineTo(ctx, 200, 200);
// lineTo(ctx, 300, 200);
// lineTo(ctx, 250, 150);
// lineTo(ctx, 150, 300);
// lineTo(ctx, 100, 100);

// lineTo(ctx, 60, 200);
// lineTo(ctx, 300, 499);
// closePath(ctx);

// moveTo(ctx, 100, 10);
// lineTo(ctx, 50, 80);
// lineTo(ctx, 100, 100);
// lineTo(ctx, 100, 10); //=> equival to lineTo

// lineTo(ctx, 50, 30);
// lineTo(ctx, 100, 100);

// ---6!
// moveTo(ctx, 50, 40);
// lineTo(ctx, 70, 40);
// lineTo(ctx, 70, 80);
// lineTo(ctx, 50, 80);
// closePath(ctx);
// moveTo(ctx, 50, 90);
// lineTo(ctx, 70, 90);
// lineTo(ctx, 70, 100);
// lineTo(ctx, 50, 100);
// closePath(ctx);
// moveTo(ctx, 100, 40);
// lineTo(ctx, 140, 40);
// lineTo(ctx, 140, 100);
// lineTo(ctx, 100, 100);
// lineTo(ctx, 100, 60);
// lineTo(ctx, 130, 60);
// lineTo(ctx, 130, 50);
// lineTo(ctx, 100, 50);
// closePath(ctx);
// moveTo(ctx, 110, 70);
// lineTo(ctx, 130, 70);
// lineTo(ctx, 130, 90);
// lineTo(ctx, 110, 90);
// closePath(ctx);
// ---