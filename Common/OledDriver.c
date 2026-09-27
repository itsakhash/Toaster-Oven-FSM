/*
 * File:   OledDriver.c
 * Author: Adam Korycki
 *
 * Support library for main Oled driver (Oled.c/h). 
 * I2C level interface for SSD1306 Oled module.
 * 
 * Created on November 6, 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BOARD.h"
#include "I2c.h"
#include "OledDriver.h"
#include "Timers.h"

#define OLED_ADDRESS 0x3C // I2C address for Oled device

/**
 * Constants for the various transmission types
 */
  #define COMMAND           0x80  // Continuation bit=1, D/C=0; 1000 0000
  #define COMMAND_STREAM    0x00  // Continuation bit=0, D/C=0; 0000 0000
  #define DATA              0xC0  // Continuation bit=1, D/C=1; 1100 0000
  #define DATA_STREAM       0x40  // Continuation bit=0, D/C=1; 0100 0000

/**
 * Constants for the various command values that can be sent to the OLED driver.
 */
typedef enum {
    OLED_COMMAND_SET_DISPLAY_LOWER_COLUMN_0 = 0x00,
    OLED_COMMAND_SET_DISPLAY_UPPER_COLUMN_0 = 0x10,
    OLED_COMMAND_SET_PAGE = 0x22,
    OLED_COMMAND_SET_CHARGE_PUMP = 0x8D,
    OLED_COMMAND_SET_SEGMENT_REMAP = 0xA1,
    OLED_COMMAND_DISPLAY_NORMAL = 0xA6,
    OLED_COMMAND_DISPLAY_INVERTED = 0xA7,
    OLED_COMMAND_DISPLAY_OFF = 0xAE,
    OLED_COMMAND_DISPLAY_ON = 0xAF,
    OLED_COMMAND_SET_PRECHARGE_PERIOD = 0xD9,
    OLED_COMMAND_SET_COM_PINS_CONFIG = 0xDA
} OledCommand;

/**
 * Store constants for all settings used with the OLED driver.
 */
typedef enum {
    OLED_SETTING_ENABLE_CHARGE_PUMP = 0x14,
    OLED_SETTING_MAXIMUM_PRECHARGE = 0xF1,
    OLED_SETTING_SEQUENTIAL_COM_NON_INTERLEAVED = 0x20,
    OLED_SETTING_REVERSE_ROW_ORDERING = 0xC8
} OledSetting;

#define OLED_DRIVER_PAGES 4

/**
 * This array is the off-screen frame buffer used for rendering.
 * It isn't possible to read back from the OLED display device,
 * so display data is rendered into this off-screen buffer and then
 * copied to the display.
 * @note  Any time this is updated, An `OledDriverUpdateDisplay()` call must be
 *        performed.
 */
uint8_t rgbOledBmp[OLED_DRIVER_BUFFER_SIZE];

// Function prototypes for private functions.
void DelayMs(uint32_t ms);

/**
 * Initialize the STM32 to communicate with the OLED display through the SSD1306
 * display controller using an instance of I2C.
 */
void OledHostInit(void)
{
    BOARD_Init(); // init board and printf functionality
    Timers_Init(); // init timer module for delay functions
    I2C_Init();   // init I2C module
}

/**
 * Initialize the OLED display and send init/config sequence
 */
void OledDriverInitDisplay(void)
{
    // Turn off the display.
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_DISPLAY_OFF);

    // Enable the charge pump and
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_CHARGE_PUMP);
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_SETTING_ENABLE_CHARGE_PUMP);
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_PRECHARGE_PERIOD);
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_SETTING_MAXIMUM_PRECHARGE);

    // Invert row numbering so that (0,0) is upper-right.
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_SEGMENT_REMAP);
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_SETTING_REVERSE_ROW_ORDERING);

    // Set sequential COM configuration with non-interleaved memory.
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_COM_PINS_CONFIG);
    I2C_WriteReg(
      OLED_ADDRESS, COMMAND, OLED_SETTING_SEQUENTIAL_COM_NON_INTERLEAVED
    );

    // And turn on the display.
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_DISPLAY_ON);
}

/**
 * Set the LCD to display pixel values as the opposite of how they are actually
 * stored in NVRAM. So pixels set to black (0) will display as white, and pixels
 * set to white (1) will display as black.
 */
void OledDriverSetDisplayInverted(void)
{

    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_DISPLAY_INVERTED);
}

/**
 * Set the LCD to display pixel values normally, where a 1 indicates white and a
 * 0 indicates black. This is the default operating mode of the LCD and the mode
 * it starts up in.
 */
void OledDriverSetDisplayNormal(void)
{
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_DISPLAY_NORMAL);
}

/**
 * Disable the Oled display before power-off.
 */
void OledDriverDisableDisplay(void)
{
    // Send the display off command.
    I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_DISPLAY_OFF);
}

/**
 * Update the display with the contents of rgb0ledBmp.
 */
void OledDriverUpdateDisplay(void)
{
    uint8_t *pb = rgbOledBmp;
    int page;
    for (page = 0; page < OLED_DRIVER_PAGES; page++) {

        // Set the desired page.
        I2C_WriteReg(OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_PAGE);
        I2C_WriteReg(OLED_ADDRESS, COMMAND, page);

        // Set the starting column back to the origin.
        I2C_WriteReg(
          OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_DISPLAY_LOWER_COLUMN_0
        );
        I2C_WriteReg(
          OLED_ADDRESS, COMMAND, OLED_COMMAND_SET_DISPLAY_UPPER_COLUMN_0
        );

        // Write this entire column to the OLED.
        for (int i = 0; i < OLED_DRIVER_PIXEL_COLUMNS; i++) {
            I2C_WriteReg(OLED_ADDRESS, DATA, *pb);
            pb++;
        }
    }
}

/**
 * Block the processor for the desired number of milliseconds.
 * @param ms The number of milliseconds to block for.
 */
void DelayMs(uint32_t ms) {
    uint32_t curr_ms = Timers_GetMilliSeconds();
    while ((Timers_GetMilliSeconds() - curr_ms) < ms);
}
