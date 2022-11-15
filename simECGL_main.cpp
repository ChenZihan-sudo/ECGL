#include "GLFW/glfw3.h"
#include <iostream>
#include "canvas/canvas_driver.h"
#include "canvas/canvas.h"
#include "canvas/canvas_ext.h"
#include "canvas/canvas_config.h"
#include "display/display_config.h"
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
    CanvaHandle_ptr ctx = nullptr;
    ctx = newCanva(ctx);
    beginPath(ctx);
    ctx->rgb888 = 0xFF0000;

    moveTo(ctx, 250, 50);
    lineTo(ctx, 300, 90);
    lineTo(ctx, 300, 100);
    lineTo(ctx, 250, 100);
    lineTo(ctx, 250, 50);

    ctx->rgb888 = 0x00FF00;
    // fill(ctx);
    ctx->rgb888 = 0xFF0000;
    stroke(ctx);
    floodFill(BOUNDARY_COLOR, 251, 51, 0xFF0000, 0x00FF00);

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
    // int radii[4] = { 0, 10, 10, 10 };
    // roundRect(ctx, 150, 50, 30, 30, 4, radii);
    // write_display_memory_on();

    strokeLineAA(ctx, 50, 50, 100, 150);
    write_display_memory_on();

    strokeLine(ctx, 80, 50, 130, 150);
    write_display_memory_on();

    Array_ptr arr = newArray(5);

    int val0 = 0;
    int val1 = 1;
    int val2 = 2;
    int val3 = 3;
    int val4 = 4;
    int val5 = 5;

    arrWrite(arr, 0, &val0);
    arrWrite(arr, 1, &val1);
    arrWrite(arr, 2, &val2);
    arrWrite(arr, 3, &val3);
    arrWrite(arr, 4, &val4);
    arrWrite(arr, 5, &val5);

    for (size_t i = 0; i < 6; i++)
    {
        int *val = (int *)arrData(arr, i);
        printf("=>%d\n", *val);
    }
    printf("Size:%d\n", arr->size);

    return 0;
}