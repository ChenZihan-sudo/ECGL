#ifndef SIMECGL_CPP
#define SIMECGL_CPP     v  

#include "GLFW/glfw3.h"
#include "../canvas/canvas_driver.h"
#include "../display/display_config.h"
#include "../simECGL/simECGL.h"
#include <stdio.h>

GLubyte glImage[DP_HEIGHT][DP_WIDTH][3];
GLFWwindow* mainECGLWindow = createGLFWindow();

void IDM_write_to_GLImage() {
	for (size_t i = 0; i < DP_HEIGHT; i++)
	{
		for (size_t j = 0; j < DP_WIDTH; j++)
		{
			size_t addr = 2 * ((IDM_yBegin + i) * DISPLAY_WIDTH + (IDM_xBegin + j));
			RGB565 rgb565 = ((uint16_t)displayMemory[addr]) << 8 | ((uint16_t)displayMemory[addr + 1]) << 0;
			RGB888 rgb888 = RGB565_to_RGB888(rgb565);
			
			uint8_t red = ((rgb888 & RGB888_RED) >> 16);
			uint8_t green = ((rgb888 & RGB888_GREEN) >> 8);
			uint8_t blue = ((rgb888 & RGB888_BLUE) >> 0);
			
			glImage[i][j][0] = red;
			glImage[i][j][1] = green;
			glImage[i][j][2] = blue;
		}
	}
}

bool LoadCurrentFrame = true;
void simECGL_glWindowDisplay() {
	IDM_write_to_GLImage();
	glDrawPixels(DP_WIDTH, DP_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, glImage);
	glfwSwapBuffers(mainECGLWindow); // Swap front and back buffers
	glfwPollEvents();
	LoadCurrentFrame = true;
}

GLFWwindow* createGLFWindow() {
	GLFWwindow* window = NULL;
	if (window == NULL) {
		/* Initialize the library */
		if (!glfwInit())
			return window;

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(DP_WIDTH, DP_HEIGHT, "ECGL Simluator (beta)", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return window;
		}

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		//! Custom one.
		glfwSetWindowPos(window, 500, 500);
	}

	return window;
}

#endif