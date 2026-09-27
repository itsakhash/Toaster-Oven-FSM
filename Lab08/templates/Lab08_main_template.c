/**
 * @file    Lab08_main_template.c
 *
 * @author
 *
 * @date
 */
// **** Include libraries here ****
// Standard libraries.
#include <stdio.h>

// Course library.
#include <Adc.h>
#include <BOARD.h>
#include <Buttons.h>
#include <Leds.h>
#include <Oled.h>
#include <Timers.h>

// **** Set any macros or preprocessor directives here ****

// **** Set any local typedefs here ****
typedef enum
{
    SETUP,
    SELECTOR_CHANGE_PENDING,
    COOKING,
    RESET_PENDING
} OvenState;

typedef struct
{
    OvenState state;
    // add more members to this struct
} OvenData;

// **** Declare any datatypes here ****

// **** Define any module-level, global, or external variables here ****
static volatile OvenData oven;

// **** Put any helper functions here ****

/* This function will update your OLED to reflect the state. */
void updateOvenOLED(void)
{
    // Update OLED here.
}

/* This function will execute your state machine.
 * It should ONLY run if an event flag has been set.
 */
void runOvenSM(void)
{
    // Write your SM logic here.
}

int main()
{
    BOARD_Init();
    Buttons_Init();
    LEDs_Init();
    Timers_Init();
    ADC_Init(ADC_CONTINUOUS_WATCHDOG); // See the typedef of Adc_Modes_t in Adc.h for the ADC configuration options.
    OLED_Init();

    printf(
        "Welcome to CRUZID's Lab08 (Toaster Oven)."
        "Compiled on %s %s.\n\r",
        __TIME__,
        __DATE__);

    // Initialize state machine (and anything else you need to init) here.

    while (1)
    {
        // Add main loop code here:
        // check for events
        // on event, run runOvenSM()
        // clear event flags
    };

    BOARD_End();
    while(1);
}


/**
 * This is the interrupt for the TIM peripheral. It will trigger whenever a timer
 * ticks over from its period to 0.
 *
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2) // This will be triggered every TIM2_DEFAULT_FREQ_HZ
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
    else if (htim == &htim3) // This will be triggered every TIM3_DEFAULT_FREQ_HZ
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
    else if (htim == &htim4) // This will be triggered every TIM4_DEFAULT_FREQ_HZ
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
 * This is the interrupt for the ADC1 peripheral. It will trigger whenever a new
 * ADC reading is available in the ADC when you are configured as
 * ADC_CONTINUOUS_INTERRUPT or ADC_SINGLE_SHOT_INTERRUPT.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
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
 * It should not be called, and should communicate with main code only by using
 * module-level variables.
 */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc)
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
