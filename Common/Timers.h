/**
 * @file    Timers.h
 *
 * Configure and manipulate timers for use with the Nucleo F411RE.
 *
 * @author  Adam Korycki
 * @author  HARE Lab
 *
 * @date    29 Sept 2023
 */
#ifndef TIMERS_H
#define	TIMERS_H

#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_tim.h"


/*  MODULE-LEVEL DEFINITIONS, MACROS    */
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

#define TIM2_DEFAULT_FREQ_HZ 5
#define TIM3_DEFAULT_FREQ_HZ 100
#define TIM4_DEFAULT_FREQ_HZ 1000
#define TIM5_DEFAULT_FREQ_HZ 1000000


/*  PROTOTYPES  */
/** Timers_SetupTimer(htim, prescaler, period)
 *
 * Changes the specified timer's interrupt frequency.
 * The timer must already be initialized.
 * The timer's counter only increments every (prescaler + 1) timer clock cycles (default 100 MHz, 10 ns)
 * Depending on the timer, the period is either 16-bit or 32-bit.
 * 16-bit: TIM3 TIM4
 * 32-bit: TIM2 TIM5
 *
 * @param   htim      (TIM_HandleTypeDef *) [htim1, htim2, htim3, htim4, htim5]
 * @param   prescaler (uint16_t)            Desired timer prescaler. Timer CNT register increments every (prescaler + 1) ticks at the timer clock frequency (default 100 MHz, 10 ns).
 * @param   period    (uint32_t)            Desired timer peiod of the CNT register in pre-scaled timer clock cycles. On TIM3 and TIM4, this is a 16-bit value.
 * @return  status    (uint8_t)             [SUCCESS, ERROR]
 */
uint8_t Timers_ConfigTimer(TIM_HandleTypeDef *htim, uint16_t prescaler, uint32_t period);

/** Timers_Start(htim*, channel)
 *
 * [Re]Starts specified timer channel interrupt service.
 *
 * @param   *htim   (TIM_HandleTypeDef *)   [htim2]
 * @param   channel (uint32_t)              [TIM_1,..., TIM_4]
 * @return  status  (uint8_t)               [SUCCESS, ERROR]
 * TODO(nubby): Add channel setup confirmation.
 */
//uint8_t Timers_Start(TIM_HandleTypeDef *htim, uint32_t channel);
uint8_t Timers_Start(TIM_HandleTypeDef *htim);

/** Timers_Stop(htim*, channel)
 *
 * Stops specified timer channel interrupt service.
 *
 * @param   *htim   (TIM_HandleTypeDef *)   [htim2]
 * @param   channel (uint32_t)              [TIM_1,..., TIM_4]
 * @return  status  (uint8_t)               [SUCCESS, ERROR]
 * TODO(nubby): Add channel selection confirmation.
 */
//uint8_t Timers_Stop(TIM_HandleTypeDef *htim, uint32_t channel);
uint8_t Timers_Stop(TIM_HandleTypeDef *htim);

/** Timers_End(void)
 *
 * Stops all timers and their associated interrupts.
 *
 * @return  status  (uint8_t)   [SUCCESS, ERROR]
 */
uint8_t Timers_End(void);

/** Timer_Init()
 *
 * Initializes the timer peripheral and starts timers.
 * It is not necessary to call Timers_Start() after Timers_Init().
 *
 * @return  status  (uint8_t)   [SUCCESS, ERROR]
 */
uint8_t Timers_Init(void);

/** Timers_GetMilliSeconds()
 *
 * @return  (uint32_t)  Current millisecond count. Overflow / rollover occurs after 71 minutes.
 */
uint32_t Timers_GetMilliSeconds(void);

/** Timers_GetMicroSeconds()
 *
 * @return  (uint32_t)  Current microsecond count. Overflow / rollover occurs after 71 minutes.
 */
uint32_t Timers_GetMicroSeconds(void);

/** Timers_GetSystemClockFreq()
 *
 * @return  (uint32_t)  Frequnecy of the timer clock (PCLK1 multiplied by the APB1 timer multiplier) in Hz.
 */
uint32_t Timers_GetSystemClockFreq(void);


#endif  /*  TIMERS_H    */
