#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// Clock Ploarity. Set 1 when SCL idle at high level.
#define DISPLAY_CPOL 1

// Clock Phase. Set 1 if sample data at second jump edge of SCL.
#define DISPLAY_CPHA 0

// Define display interface here.
#define DISPLAY_MOSI 23
#define DISPLAY_MISO -1
#define DISPLAY_SCL 18
#define DISPLAY_DC 27
#define DISPLAY_CS 5
#define DISPLAY_RES 33

// Define display width and height here.
#define DISPLAY_WIDTH 500
#define DISPLAY_HEIGHT 500

// SPI D/CX MODE
#define SPI_COMMAND_MODE 0
#define SPI_DATA_MODE 1

//
//********************************
//* ST7789V COMMAND CONFIG INFO  *
//********************************

// The display module performs a software reset, registers are written with their SW reset default values.
// It will be necessary to wait 5msec before sending new command following software reset.
#define SOFTWARE_RESET 0x01

// This command causes the LCD module to enter the minimum power consumption mode. */
#define SLEEP_IN 0x10

// This command turn off sleep mode.
#define SLEEP_OUT 0x11

// This command turns the display to normal mode.
#define NORMAL_DISPLAY_MODE 0x13

// This command is used to recover from display inversion mode.
#define DISPLAY_INVERSION_OFF 0x20

// This command is used to recover from display inversion mode.
#define DISPLAY_INVERSION_ON 0x21

// This command is used to enter into DISPLAY OFF mode. In this mode, the output from Frame Memory is disabled and blank page inserted.
#define DISPLAY_OFF 0x28

// This command is used to recover from DISPLAY OFF mode.
#define DISPLAY_ON 0x29

// This command is used to define the format of RGB picture data, which is to be transferred via the MCU interface.
#define INTERFACE_PIXEL_FORMAT 0x3A
#define RGB_65K_INTERFACE 0b0101
#define RGB_262K_INTERFACE 0b0110
#define _12BIT_PER_PIXEL 0b0011
#define _16BIT_PER_PIXEL 0b0101
#define _18BIT_PER_PIXEL 0b0110

// This command defines read/ write scanning direction of frame memory.
#define MEMORY_DATA_ACCESS_CONTROL 0x36

#define COLUMN_ADDRESS_SET 0x2A
#define ROW_ADDRESS_SET 0x2B

#define MEMORY_WRITE 0x2C

// Partial Mode
// Idle Mode
// Sleep Mode
// Normal Mode

#endif