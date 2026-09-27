/**
 * @file    Leds.h
 *
 * This library provides an interface for controlling the LEDs on the UCSC
 * Nucleo I/O Shield development board.
 *
 * @author  HARE Lab
 *
 * @date    15 Nov 2023
 */
#ifndef LEDS_H
#define LEDS_H

#include "BOARD.h"


/*  PROTOTYPES  */
/** LEDs_Init()
 *
 * Initializes the LED bar:
 *      1) Set each LED pin to "output" mode.
 *      2) Set each LED pin's output value to 0 (low voltage).
 * After calling LEDs_Init(), the other functions in this file can be used to 
 * manipulate the LED bar.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
__attribute__((weak)) int8_t LEDs_Init(void);

/** LEDs_Set()
 *
 * Controls the output on the LED bar.  Input values are 8-bit patterns, where
 * each bit describes the desired state of the corresponding LED. So, for
 * example, LEDs_Set(0x80) should  leave the first LED on, and the last 7 LEDs
 * off. LEDs_Set(0x1F) should turn off the first three LEDs and turn on the
 * remaining five LEDs. 
 *
 * LEDs_Set should not change any LED pins to inputs.
 *
 * @param   newPattern  (uint8_t)   An 8-bit value describing the desired output
 *                                  on the LED bar. 
 */
__attribute__((weak)) void LEDs_Set(uint8_t newPattern);

/** LEDs_Get()
 *
 * Returns the current state of the LED bar. Return values are 8-bit patterns,
 * where each bit describes the current state of the corresponding LED. So, for
 * example, if the LED bar's LED's are
 *
 * [ON OFF ON OFF   OFF ON OFF ON],
 *
 * LEDs_Get() should return 0xA5.
 * LEDs_Get() should not change the state of the LEDs, or any SFRs.
 * 
 * @return  ledsState   (uint8_t)   An 8-bit value describing the current output
 *                                  on the LED bar.  
 */
__attribute__((weak)) uint8_t LEDs_Get(void);


#endif // LEDS_H
