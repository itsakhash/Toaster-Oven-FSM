/*
 * File:   OledDriver.h
 * Author: Adam Korycki
 *
 * Support library for main Oled driver (Oled.c/h). 
 * I2C level interface for SSD1306 Oled module.
 * 
 * Created on November 6, 2023
 */

#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#include <stdint.h>

// The number of pixel columns in the OLED display.
#define OLED_DRIVER_PIXEL_COLUMNS       128

// The number of pixel rows in the OLED display.
#define OLED_DRIVER_PIXEL_ROWS          32

// Store how high each column is for the OLED in bits in terms of data structure
// storage.
#define OLED_DRIVER_BUFFER_LINE_HEIGHT  8

// The number of bytes required to store all the data for the whole display.
// 1 bit / pixel.
#define OLED_DRIVER_BUFFER_SIZE ( \
    (OLED_DRIVER_PIXEL_COLUMNS * OLED_DRIVER_PIXEL_ROWS) / 8 \
)

/**
 * This array is the off-screen frame buffer used for rendering. It isn't
 * possible to read back from the OLED display device, so display data is
 * rendered into this off-screen buffer and then copied to the display. The
 * high-order bits equate to the lower pixel rows.
 * @note  Any time this is updated, An `OledDriverUpdateDisplay()` call must be
 *        performed.
 */
extern uint8_t rgbOledBmp[OLED_DRIVER_BUFFER_SIZE];

/**
 * Initialize the STM32 to communicate with the OLED display through the SSD1306
 * (I2C) display controller.
 */
void OledHostInit(void);

/**
 * Initialize the OLED display and driver circuitry.
 */
void OledDriverInitDisplay(void);

/**
 * Disable the Oled display before power-off. This means powering it up, sending
 * the display off command, and finally disabling Vbat.
 */
void OledDriverDisableDisplay(void);

/**
 * Update the display with the contents of rgb0ledBmp.
 */
void OledDriverUpdateDisplay(void);

/**
 * Set the LCD to display pixel values as the opposite of how they are actually
 * stored in NVRAM. So pixels set to black (0) will display as white, and pixels
 * set to white (1) will display as black.
 */
void OledDriverSetDisplayInverted(void);

/**
 * Set the LCD to display pixel values normally, where a 1 indicates white and a
 * 0 indicates black. This is the default operating mode of the LCD and the mode
 * it starts up in.
 */
void OledDriverSetDisplayNormal(void);

#endif // OLED_DRIVER_H
