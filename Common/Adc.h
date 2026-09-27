#ifndef ADC_H
#define	ADC_H
/**
 * @file    Adc.h
 *
 * Configure and read from ADC channels using the UCSC Nucleo I/O shield.
 *
 * @author  HARE Lab
 * @author  jLab
 * @author  Adam Korycki
 *
 * @date    16 Sep 2023
 *
 * @TODO    Switch to an interrupt-driven ADC framework that uses DMA to read/
 *          store data from each of the 7 channels when triggered. This special
 *          approach is required since the ADC SFRs are not labeled by channel,
 *          i.e. otherwise we cannot tell which ADC value was read by which
 *          ADC channel (values are mixed up).
 */
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"


/*  MODULE-LEVEL VARIABLES, MACROS  */
#define ADC_0               ADC_CHANNEL_0
#define ADC_1               ADC_CHANNEL_1
#define POT                 ADC_CHANNEL_4   // Onboard potentiometer.
#define ADC_2               ADC_CHANNEL_10
#define ADC_3               ADC_CHANNEL_11
#define ADC_4               ADC_CHANNEL_12
#define ADC_5               ADC_CHANNEL_13
#define ADC_NUM_CHANNELS    7
#define ADC_MIN             0
#define ADC_MAX             4095

// ADC operating modes.
typedef enum {
  ADC_SINGLE_SHOT_POLLING,
  ADC_SINGLE_SHOT_INTERRUPT,
  ADC_SINGLE_SHOT_WATCHDOG,
  ADC_CONTINUOUS_POLLING,
  ADC_CONTINUOUS_INTERRUPT,
  ADC_CONTINUOUS_WATCHDOG
} Adc_Modes_t;

#ifndef FALSE
#define FALSE ((int8_t) 0)
#endif  /*  FALSE   */
#ifndef TRUE
#define TRUE ((int8_t) 1)
#endif  /*  TRUE    */
#ifndef ERROR
#define ERROR ((int8_t) -1)
#endif  /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t) 1)
#endif  /*  SUCCESS */

ADC_HandleTypeDef hadc1;
ADC_AnalogWDGConfTypeDef AnalogWDGConfig;


/*  PROTOTYPES  */
/** ADC_Start()
 *
 * Start ADC conversions for polling.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_Start(void);

/** ADC_End()
 *
 * Disables the ADC subsystem and releases the pins used.
 *
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_End(void);

/** ADC_Read(channel)
 *
 * Selects ADC channel and returns 12-bit reading.
 *
 * @param   channel (uint32_t)  Select ADC channel
 *                                  (ADC_0, ADC_1, ..., ADC_5, POT)
 * @return          (uint16_t)  12-bit ADC reading.
 */
uint16_t ADC_Read(uint32_t channel);

/** ADC_Init(mode)
 *
 * Initializes the ADC subsystem as polling, interrupt, or watchdog-driven;
 * selects ADC Pin4 by default.
 * 
 * @param   (Adc_Modes_t)       [ADC_POLLING, ADC_INTERRUPT, ADC_WATCHDOG]
 * @return  (int8_t)    [SUCCESS, ERROR]
 */
int8_t ADC_Init(Adc_Modes_t mode);

/** ADC_Watchdog_Config(high, low)
 *
 * Adjusts the analog watchdog hysteresis thresholds.
 * 
 * @param   (uint16_t)    Analog watchdog high threshold (12-bit)
 * @return  (uint16_t)    Analog watchdog low threshold (12-bit)
 */
int8_t ADC_Watchdog_Config(uint16_t high, uint16_t low);

#endif
