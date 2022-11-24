#ifndef CANVAS_CONFIG_H
#define CANVAS_CONFIG_H

#include "../datamodel/data_model.h"
#include "canvas_driver.h"
#include "canvas_err.h"

#include <math.h>
#define PI (3.14159265358979)
#define flEPS (1e-6)  // float epsilon(极小数) / polar decimal
#define dbEPS (1e-16) // double epsilon(极小数) / polar decimal

// Used for compare float number
#define flEQUAL(A, B) ((fabs((A) - (B))) < (flEPS))
#define flGREAT(A, B) (((A) - (B)) > (flEPS))
#define flGREATE(A, B) (((A) - (B)) > (-flEPS))
#define flLESS(A, B) (((A) - (B)) < (-flEPS))
#define flLESSE(A, B) (((A) - (B)) < (flEPS))

// Used for check whether even or odd number
#define cckEVEN(N) (!((N) % 2))
#define cckODD(N) ((N) % 2)

#define USE_INNER_DISPLAY_MEMORY 1
#define INNER_DISPLAY_MEMORY_FULL_SIZE 1
#define IDM_ROW_TRANSFER_SIZE 1
#define USE_FRAMEWORK_RATE_TESTER 1
#define CANVAS_DEFAULT_BRUSH 1 // Do Not Set to 0

#endif
