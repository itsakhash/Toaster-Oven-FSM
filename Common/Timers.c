/**
 * @file    Timers.c
 *
 * @author  Adam Korycki
 * @author  HARE Lab
 *
 * @date 29 Sep 2023
 *
 */
#include "Timers.h"

#include <stdint.h>
#include <stdlib.h>

/*  MODULE-LEVEL DEFINITIONS, MACROS    */
// Boolean defines for TRUE, FALSE, SUCCESS and ERROR
#ifndef FALSE
#define FALSE ((int8_t)0)
#endif /*  FALSE   */
#ifndef TRUE
#define TRUE ((int8_t)1)
#endif /*  TRUE    */
#ifndef FALSE
#define FALSE ((int8_t)1)
#endif /*  FALSE    */
#ifndef ERROR
#define ERROR ((int8_t)-1)
#endif /*  ERROR   */
#ifndef SUCCESS
#define SUCCESS ((int8_t)1)
#endif /*  SUCCESS */

/*  PROTOTYPES  */

/*  MODULE-LEVEL DEFINITIONS, MACROS    */
static uint8_t initStatus = FALSE;

/*  FUNCTIONS   */
uint8_t MX_TIM1_Init(void);
uint8_t MX_TIM2_Init(void);
uint8_t MX_TIM3_Init(void);
uint8_t MX_TIM4_Init(void);
uint8_t MX_TIM5_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle);

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
uint8_t Timers_ConfigTimer(TIM_HandleTypeDef *htim, uint16_t prescaler, uint32_t period)
{
  // uint32_t sysClkFreq = Timers_GetSystemClockFreq(); // Get the timer clock freq in Hz.

  htim->Init.Prescaler = prescaler;
  htim->Init.Period = period;
  if (HAL_TIM_Base_Init(htim) != HAL_OK)
  {
    return ERROR;
  }

  return SUCCESS;
}

/** Timers_Start(htim*, channel)
 *
 * [Re]Starts specified timer channel interrupt service.
 *
 * @param   *htim   (TIM_HandleTypeDef *)   [htim2]
 * @return  status  (uint8_t)               [SUCCESS, ERROR]
 */
// uint8_t Timers_Start(TIM_HandleTypeDef *htim, uint32_t channel)
uint8_t Timers_Start(TIM_HandleTypeDef *htim)
{
  HAL_TIM_Base_Start_IT(htim);

  return SUCCESS;
}

/** Timers_Stop(htim*, channel)
 *
 * Stops specified timer channel interrupt service.
 *
 * @param   *htim   (TIM_HandleTypeDef *)   [htim2]
 * @return  status  (uint8_t)               [SUCCESS, ERROR]
 */
// uint8_t Timers_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
uint8_t Timers_Stop(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM5)
  {
    HAL_TIM_Base_Stop(htim);
  }
  else
  {
    HAL_TIM_Base_Stop_IT(htim);
  }
  return SUCCESS;
}

/** Timers_End(void)
 *
 * Stops all timers and their associated interrupts.
 *
 * @return  status  (uint8_t)   [SUCCESS, ERROR]
 */
uint8_t Timers_End(void)
{
  Timers_Stop(&htim2);
  Timers_Stop(&htim3);
  Timers_Stop(&htim4);
  Timers_Stop(&htim5);

  // TODO(nubby): Shut down the htim2 interrupt.

  return SUCCESS;
}

/** Timers_Init()
 *
 * Initializes and starts all peripheral timers with a default interrupt
 * frequency.
 *
 * @return  status  (uint8_t)       [SUCCESS, ERROR]
 */
uint8_t Timers_Init(void)
{
  if (initStatus == FALSE)
  {

    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM5_Init();

    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_Base_Start_IT(&htim3);
    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_Base_Start(&htim5);

    initStatus = TRUE;
  }
  return SUCCESS;
}

/* TIM1 init function */
uint8_t MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    return ERROR;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    return ERROR;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    return ERROR;
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    return ERROR;
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    return ERROR;
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);
  return SUCCESS;
}

/* TIM2 init function */
uint8_t MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 20000000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    return ERROR;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    return ERROR;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    return ERROR;
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  return SUCCESS;
}

/* TIM3 init function */
uint8_t MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 62500;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    return ERROR;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    return ERROR;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    return ERROR;
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    return ERROR;
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  return SUCCESS;
}

/* TIM4 init function */
uint8_t MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 50000;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    return ERROR;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    return ERROR;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    return ERROR;
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    return ERROR;
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    return ERROR;
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);
  return SUCCESS;
}

/* TIM5 init function */
uint8_t MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 99;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    return ERROR;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    return ERROR;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    return ERROR;
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  return SUCCESS;
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (timHandle->Instance == TIM1)
  {
    /* USER CODE BEGIN TIM1_MspPostInit 0 */

    /* USER CODE END TIM1_MspPostInit 0 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PA8     ------> TIM1_CH1
    PA9     ------> TIM1_CH2
    PA10     ------> TIM1_CH3
    PA11     ------> TIM1_CH4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM1_MspPostInit 1 */

    /* USER CODE END TIM1_MspPostInit 1 */
  }
  else if (timHandle->Instance == TIM4)
  {
    /* USER CODE BEGIN TIM4_MspPostInit 0 */

    /* USER CODE END TIM4_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    PB8     ------> TIM4_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM4_MspPostInit 1 */

    /* USER CODE END TIM4_MspPostInit 1 */
  }
}

// TODO: Refactor these to accept a Timer # param.
/**
 * @function Timers_GetMilliSeconds(void)
 * @param None
 * @return current millisecond count
 * @brief ^
 * @note Based on TIM5->CNT, which is a 32-bit timer at 1 MHz. This count overflows/resets after 4294 seconds (71 minutes).
 * @author Adam Korycki, 2023.09.29 */
uint32_t Timers_GetMilliSeconds(void)
{
  return (Timers_GetMicroSeconds() / 1000);
}

/**
 * @function Timers_GetMicroSeconds(void)
 * @param None
 * @return current microsecond count
 * @brief ^
 * @note Based on TIM5->CNT, which is a 32-bit timer at 1 MHz. This count overflows/resets after 4294 seconds (71 minues).
 * @author Adam Korycki, 2023.09.29 */
uint32_t Timers_GetMicroSeconds(void)
{
  return (TIM5->CNT); // (ms*1000) + (current value of 1Mhz counter)
}

/**
 * @function Timers_GetSystemClockFreq(void)
 * @param None
 * @return Frequnecy of the timer clock (PCLK1 multiplied by the APB1 timer multiplier) in Hz.
 * @brief ^
 * @author Adam Korycki, 2023.09.29 */
uint32_t Timers_GetSystemClockFreq(void)
{
#ifdef STM32F4
  // Assume that TIM5 is using the internal clock source TIM_CLOCKSOURCE_INTERNAL.
  // This is PCLK1 (maximum 50 MHz) multiplied by the APB1 timer multiplier (which depends on the APB1 divider and TIMPRE)
  RCC_ClkInitTypeDef rccClocksConfig = {0};
  uint32_t flashLatency = {0};
  HAL_RCC_GetClockConfig(&rccClocksConfig, &flashLatency);

  if (RCC->DCKCFGR & RCC_DCKCFGR_TIMPRE)
  {
    if (rccClocksConfig.APB1CLKDivider & (RCC_HCLK_DIV1 | RCC_HCLK_DIV2))
    {
      return HAL_RCC_GetPCLK1Freq();
    }
    else
    {
      return HAL_RCC_GetPCLK1Freq() * 4;
    }
  }
  else
  {
    if (rccClocksConfig.APB1CLKDivider & RCC_HCLK_DIV1) // RCC->RCC_CFGR
    {
      return HAL_RCC_GetPCLK1Freq();
    }
    else
    {
      return HAL_RCC_GetPCLK1Freq() * 2;
    }
  }

#else
  return 0;
#endif /*  STM32F4 */
}

/**
 * This is the interrupt for the TIM peripheral. It will trigger whenever a timer
 * ticks over from its period to 0.
 *
 * If you want to use this interrupt, you can overwrite this __weak function by
 * copy/pasting this entire function to your main file and removing _weak.
 *
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
__weak void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2) // This will be triggered every TIM2_DEFAULT_FREQ_HZ
  {
    // TIM2
  }
  else if (htim == &htim3) // This will be triggered every TIM3_DEFAULT_FREQ_HZ
  {
    // TIM3
  }
  else if (htim == &htim4) // This will be triggered every TIM4_DEFAULT_FREQ_HZ
  {
    // TIM4
  }
}

/** TIMERS_TEST
 *
 * Uncomment the below "#define" to run this test.
 * Verifies the following:
 *  1)  Each timer tick == 1ms.
 *  2)  The "GetMilliseconds" function is accurate.
 */
// #define TIMERS_TEST
#ifdef TIMERS_TEST
// SUCCESS - printed microsecond and millisecond values are are around 100 ms apart

#include <stdio.h>
#include <stdlib.h>

#include "BOARD.h"
#include "Timers.h"

volatile int t = 0;
volatile int8_t timerNewEvent = FALSE;

int main(void)
{
  BOARD_Init();
  Timers_Init();

  int secs = 0;

  while (TRUE)
  {
    if (timerNewEvent == TRUE)
    {
      secs += 1;
      printf("Beep boop: %d\r\n", secs);
      timerNewEvent = FALSE;
    }
  }
}

/** HAL_TIM_OC_DelayElapsedCallback(htim)
 * Interrupt handling for timer IRQ handling.
 *
 * TIM2 is set up with the following base and channel frequencies:
 *  + f_base:   96MHz
 *  + htim2:    1kHz
 *  + htim3:    5Hz
 *  + htim4:    100Hz
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /***************************************************************************
   * Your code goes in between this comment and the following one with
   * asterisks.
   **************************************************************************/
  if (htim == &htim2)
  {
    timerNewEvent = TRUE;
  }
  Timers_Start(htim);
}
#endif /*  TIMERS_TEST */
