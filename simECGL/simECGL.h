#ifndef SIM_ECGL_H
#define SIM_ECGL_H

#include <GLFW/glfw3.h>
#include "../canvas/canvas_driver.h"
#include "../display/display_config.h"

const GLsizei DP_HEIGHT = DISPLAY_HEIGHT;
const GLsizei DP_WIDTH = DISPLAY_WIDTH;
extern GLubyte glImage[DP_HEIGHT][DP_WIDTH][3];

extern bool LoadCurrentFrame;
extern GLFWwindow* mainECGLWindow;

void IDM_write_to_GLImage();
void simECGL_glWindowDisplay();
GLFWwindow* createGLFWindow();

#endif

