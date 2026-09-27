/**
 * @file    Adc.c
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Adc.h"

/*  PROTOTYPES  */
static int8_t ADC_ConfigPins(void);
static int8_t ADC_ConfigClks(void);

/*  MODULE-LEVEL DEFINITIONS, MACROS    */
#define ADC_WATCHDOG_HIGH_DEFAULT 3000
#define ADC_WATCHDOG_LOW_DEFAULT 1000
static int8_t initStatus = FALSE;
static Adc_Modes_t adcMode = ADC_CONTINUOUS_INTERRUPT; // Set to ADC_CONTINUOUS_INTERRUPT by default.

/*  FUNCTIONS   */
static int8_t ADC_ConfigPins(void)
{
    /**ADC1 GPIO Configuration
    PC0     ------> ADC1_IN10
    PC1     ------> ADC1_IN11
    PC2     ------> ADC1_IN12
    PC3     ------> ADC1_IN13
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA4     ------> ADC1_IN4
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    switch (adcMode)
    {
    case ADC_SINGLE_SHOT_INTERRUPT:
    case ADC_SINGLE_SHOT_WATCHDOG:
    case ADC_CONTINUOUS_INTERRUPT:
    case ADC_CONTINUOUS_WATCHDOG:
        HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC_IRQn);
        break;
    default:
        break;
    }

    return SUCCESS;
}

static int8_t ADC_ConfigClks(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    return SUCCESS;
}

int8_t ADC_Start(void)
{
    switch (adcMode)
    {
    case ADC_SINGLE_SHOT_POLLING:
    case ADC_CONTINUOUS_POLLING:
    case ADC_SINGLE_SHOT_WATCHDOG:
    case ADC_CONTINUOUS_WATCHDOG:
        HAL_ADC_Start(&hadc1);
        break;
    case ADC_SINGLE_SHOT_INTERRUPT:
    case ADC_CONTINUOUS_INTERRUPT:
        HAL_ADC_Start_IT(&hadc1);
        break;
    default:
        break;
    }
    return SUCCESS;
}

int8_t ADC_End(void)
{
    switch (adcMode)
    {
    case ADC_SINGLE_SHOT_POLLING:
    case ADC_CONTINUOUS_POLLING:
    case ADC_SINGLE_SHOT_WATCHDOG:
    case ADC_CONTINUOUS_WATCHDOG:
        HAL_ADC_Stop(&hadc1);
        break;
    case ADC_SINGLE_SHOT_INTERRUPT:
    case ADC_CONTINUOUS_INTERRUPT:
        HAL_ADC_Stop_IT(&hadc1);
        break;
    default:
        break;
    }

    HAL_ADC_DeInit(&hadc1);

    return SUCCESS;
}

uint16_t ADC_Read(uint32_t channel)
{
    // // Select channel and sampling time.
    // ADC_ChannelConfTypeDef sConfig = {0};
    // sConfig.Channel = channel;
    // sConfig.Rank = 1;
    // sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    // HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // Start ADC Conversion, wait for conversion and return value.
    switch (adcMode)
    {
    case ADC_SINGLE_SHOT_POLLING:
        ADC_Start();
        HAL_ADC_PollForConversion(&hadc1, 1);
        break;
    case ADC_SINGLE_SHOT_INTERRUPT:
    case ADC_SINGLE_SHOT_WATCHDOG:
    case ADC_CONTINUOUS_POLLING:
    case ADC_CONTINUOUS_INTERRUPT:
    case ADC_CONTINUOUS_WATCHDOG:
        HAL_ADC_GetValue(&hadc1);
        break;
    default:
        break;
    }

    return HAL_ADC_GetValue(&hadc1);
}

int8_t ADC_Init(Adc_Modes_t mode)
{
    adcMode = mode;
    if (initStatus == FALSE)
    {
        ADC_ConfigClks();
        ADC_ConfigPins();

        /** Configure the global features of the ADC (clock, resolution, data
         *  alignment and number of conversions).
         */
        hadc1.Instance = ADC1;
        hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
        hadc1.Init.Resolution = ADC_RESOLUTION_12B;
        hadc1.Init.ScanConvMode = DISABLE;
        hadc1.Init.DiscontinuousConvMode = DISABLE;
        hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
        hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
        hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
        hadc1.Init.NbrOfConversion = 1;
        hadc1.Init.DMAContinuousRequests = DISABLE;
        hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV; // ADC_EOC_SINGLE_CONV or ADC_EOC_SEQ_CONV. If overrrun errors, then set to ADC_EOC_SEQ_CONV and increase sampling time.

        switch (adcMode)
        {
        case ADC_SINGLE_SHOT_POLLING:
        case ADC_SINGLE_SHOT_INTERRUPT:
        case ADC_SINGLE_SHOT_WATCHDOG:
            hadc1.Init.ContinuousConvMode = DISABLE;
            break;
        case ADC_CONTINUOUS_POLLING:
        case ADC_CONTINUOUS_INTERRUPT:
        case ADC_CONTINUOUS_WATCHDOG:
            hadc1.Init.ContinuousConvMode = ENABLE;
            break;
        default:
            break;
        }

        if (HAL_ADC_Init(&hadc1) != HAL_OK)
        {
            return ERROR;
        }

        switch (adcMode)
        {
        case ADC_SINGLE_SHOT_WATCHDOG:
        case ADC_CONTINUOUS_WATCHDOG:
            ADC_Watchdog_Config(ADC_WATCHDOG_HIGH_DEFAULT, ADC_WATCHDOG_LOW_DEFAULT);
            break;
        default:
            break;
        }

        ADC_ChannelConfTypeDef sConfig = {0};
        sConfig.Channel = POT;
        sConfig.Rank = 1;
        sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 3 15 28 56 84 112 144 480
        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
        {
            return ERROR;
        }

        // Start ADC.
        // ADC_Start();

        // Complete initialization.
        initStatus = TRUE;
    }

    return SUCCESS;
}

int8_t ADC_Watchdog_Config(uint16_t high, uint16_t low)
{
    AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_SINGLE_REG;
    AnalogWDGConfig.HighThreshold = high;
    AnalogWDGConfig.LowThreshold = low;
    AnalogWDGConfig.Channel = POT;
    AnalogWDGConfig.ITMode = ENABLE;
    if (HAL_ADC_AnalogWDGConfig(&hadc1, &AnalogWDGConfig) != HAL_OK)
    {
        return ERROR;
    }
    return SUCCESS;
}

/**
 * This is the interrupt for the ADC1 peripheral. It will trigger whenever a new
 * ADC reading is available in the ADC when you are configured as
 * ADC_CONTINUOUS_INTERRUPT or ADC_SINGLE_SHOT_INTERRUPT.
 *
 * If you want to use this interrupt, you can overwrite this __weak function by
 * copy/pasting this entire function to your main file and removing _weak.
 *
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
__weak void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        /***************************************************************************
         * Your code goes in between this comment and the following one with
         * asterisks.
         **************************************************************************/

        /***************************************************************************
         * Your code goes in between this comment and the preceding one with
         * asterisks.
         **************************************************************************/
    }
}

/**
 * This is the interrupt for the ADC1 peripheral's analog watchdog. It will trigger
 * whenever a new ADC reading is available that is outside of the high/low thresholds
 * that you set in ADC_Watchdog_Config(). This interrupt is only active when the ADC
 * is configured as ADC_CONTINUOUS_WATCHDOG or ADC_SINGLE_SHOT_WATCHDOG.
 *
 * If you want to use this interrupt, you can overwrite this __weak function by
 * copy/pasting this entire function to your main file and removing _weak.
 *
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
__weak void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        /***************************************************************************
         * Your code goes in between this comment and the following one with
         * asterisks.
         **************************************************************************/

        /***************************************************************************
         * Your code goes in between this comment and the preceding one with
         * asterisks.
         **************************************************************************/
    }
}

/**
 * This interrupt is called when the ADC1 peripheral encounters an error.
 *
 * If you want to use this interrupt, you can overwrite this __weak function by
 * copy/pasting this entire function to your main file and removing _weak.
 *
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
__weak void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    uint32_t errorCode = HAL_ADC_GetError(&hadc1);
    printf("\r\n\r\nADC CALLBACK ERROR: %ld\r\n", errorCode);
    switch (errorCode)
    {
    case HAL_ADC_ERROR_NONE:
        printf("No error.\r\n");
        break;
    case HAL_ADC_ERROR_INTERNAL:
        printf("ADC IP internal error: if problem of clocking, enable/disable, erroneous state.\r\n");
        break;
    case HAL_ADC_ERROR_OVR:
        printf("Overrun error. Set EOCSelection to ADC_EOC_SEQ_CONV or increase SamplingTime.\r\n");
        break;
    case HAL_ADC_ERROR_DMA:
        printf("DMA transfer error.\r\n");
        break;
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    case HAL_ADC_ERROR_INVALID_CALLBACK:
        printf("Invalid Callback error.\r\n");
        break;
#endif
    default:
        printf("Unknown error.\r\n");
        break;
    }
}

/** ADC_TEST
 *
 * Uncomment the below "#define" to run the ADC_TEST.
 *
 * SUCCESS - Printed analog values for each channel are correct.
 */
// #define ADC_TEST
#ifdef ADC_TEST

#include <stdio.h>
#include <Board.h>
#include <ADC.h>

int main(void)
{
    BOARD_Init();
    char ret = ADC_Init();

    if (ret != ERROR)
    {
        while (TRUE)
        {
            printf("POT   = %d\r\n", ADC_Read(POT));
            printf("ADC_0 = %d\r\n", ADC_Read(ADC_0));
            printf("ADC_1 = %d\r\n", ADC_Read(ADC_1));
            printf("ADC_2 = %d\r\n", ADC_Read(ADC_2));
            printf("ADC_3 = %d\r\n", ADC_Read(ADC_3));
            printf("ADC_4 = %d\r\n", ADC_Read(ADC_4));
            printf("ADC_5 = %d\r\n\r\n", ADC_Read(ADC_5));
            HAL_Delay(500);
        }
    }
    else
    {
        printf("ADC init error");
    }
}

#endif /*  ADC_TEST    */
