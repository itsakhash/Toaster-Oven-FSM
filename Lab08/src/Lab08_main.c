/**
 * @file    Lab08_main.c
 *
 * @author  Akhash Arjundayal (aarjunda)
 *
 * @date    August 15 2026
 */
// **** Include libraries here ****
// Standard libraries.
#include <stdio.h>
#include <stdint.h>

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
    RESET_PENDING,
    ALERT // Extra credit: blinks the display after cooking finishes naturally, until dismissed.
} OvenState;

// The three cooking modes, cycled via a long-press of BTN3.
typedef enum
{
    MODE_BAKE,
    MODE_TOAST,
    MODE_BROIL
} CookMode;

// Which value the potentiometer currently controls (only relevant in Bake mode).
typedef enum
{
    SELECT_TIME,
    SELECT_TEMP
} SelectorState;

typedef struct
{
    OvenState state;             // Current FSM state.
    CookMode mode;               // Current cooking mode (bake/toast/broil).
    SelectorState selector;      // Whether the pot currently adjusts time or temp.
    uint16_t cookTimeSec;        // Current time, in seconds: set time in SETUP, remaining time while COOKING.
    uint16_t initialCookTimeSec; // Cook time as set before cooking started -- for the LED bar and cancel-restore.
    uint16_t tempF;              // Cook temperature, in degrees F.
    uint16_t buttonPressTime;    // freeRunningCounter snapshot when a button went down, in timer ticks.
    uint16_t cookStartTime;      // freeRunningCounter snapshot when cooking started, in timer ticks.
} OvenData;

// Events that can drive the oven's state machine. Set by ISRs, consumed
// (and cleared) by the main loop before calling runOvenSM().
typedef enum
{
    EVENT_ADC_CHANGED,
    EVENT_TIMER_TICK,
    EVENT_BTN3_DOWN,
    EVENT_BTN3_UP,
    EVENT_BTN4_DOWN,
    EVENT_BTN4_UP
} OvenEvent;

// **** Declare any datatypes here ****

// **** Define any module-level, global, or external variables here ****
static volatile OvenData oven;

// Most recent raw (12-bit) ADC reading, captured by the ADC ISR. Only
// meaningful when adcChangedEvent is set.
static volatile uint16_t rawAdcValue;

// Event flags, set by ISRs and cleared by the main loop once handled.
static volatile uint8_t adcChangedEvent = FALSE;
static volatile uint8_t timerTickEvent = FALSE;

// Raw button event bits (BUTTON_EVENT_3UP/3DOWN/4UP/4DOWN), OR'd in from
// TIM3's 100Hz poll and cleared one bit at a time by the main loop -- this
// lets simultaneous button events (rare, but possible per Buttons.h) all
// still get handled instead of clobbering each other.
static volatile uint8_t buttonEventFlags = BUTTON_EVENT_NONE;

// Free-running counter, incremented every TIM2 tick (5Hz). Used to time
// button holds (see LONG_PRESS_TICKS below) without dedicating a hardware
// timer to each individual press.
static volatile uint16_t freeRunningCounter = 0;

// A button held for at least this many freeRunningCounter ticks counts as a
// LONG_PRESS. At 5Hz, 5 ticks = 1 second.
#define LONG_PRESS_TICKS TIM2_DEFAULT_FREQ_HZ

// Top 8 bits of the last ADC reading we actually acted on. Used to detect a
// meaningful change in HAL_ADC_ConvCpltCallback(), since that callback fires
// on every single conversion (much faster than our value actually matters).
static volatile uint8_t lastAdcTop8 = 0;

// Tracks whether the display is currently inverted, while blinking in ALERT.
static volatile uint8_t alertInverted = FALSE;

// **** Put any helper functions here ****

// Draws a string starting at pixel (x0, y0), advancing right per character and
// dropping to a new line (back to x0) on '\n'. Lets us place text anywhere on
// screen, unlike OLED_DrawString() which always starts each line at x=0 --
// necessary here since we want text to the right of the oven icon graphic.
static void drawTextAt(int x0, int y0, const char *str)
{
    int x = x0;
    int y = y0;
    for (const char *c = str; *c != '\0'; c++)
    {
        if (*c == '\n')
        {
            y += ASCII_FONT_HEIGHT;
            x = x0;
        }
        else
        {
            OLED_DrawChar(x, y, *c);
            x += ASCII_FONT_WIDTH;
        }
    }
}

// Human-readable names for each cook mode, indexed by CookMode.
static const char *const modeNames[] = {"BAKE", "TOAST", "BROIL"};

/* This function will update your OLED to reflect the state. */
void updateOvenOLED(void)
{
    char buffer[80];
    int offset = 0;

    OLED_Clear(OLED_COLOR_BLACK);

    // Heating elements are only actually on while cooking (including while
    // RESET_PENDING -- you're still actively cooking while we're deciding
    // whether the hold is a real cancel), and only the elements relevant to
    // the current mode: bake uses both, toast uses only the bottom element,
    // broil uses only the top element.
    uint8_t isCooking = (oven.state == COOKING) || (oven.state == RESET_PENDING);
    uint8_t topOn = isCooking && (oven.mode == MODE_BAKE || oven.mode == MODE_BROIL);
    uint8_t bottomOn = isCooking && (oven.mode == MODE_BAKE || oven.mode == MODE_TOAST);
    char topChar = topOn ? OVEN_TOP_ON[0] : OVEN_TOP_OFF[0];
    char bottomChar = bottomOn ? OVEN_BOTTOM_ON[0] : OVEN_BOTTOM_OFF[0];

    // --- Oven icon: a box where the top and bottom borders themselves are
    // the hatched heating elements, with a tray/rack graphic drawn in the
    // interior.
    const int iconX0 = 0;
    const int iconY0 = 0;
    const int iconCols = 5; // icon width, in characters
    const int iconRows = 4; // icon height, in characters: top elem / interior x2 / bottom elem
    const int iconW = iconCols * ASCII_FONT_WIDTH;
    const int iconH = iconRows * ASCII_FONT_HEIGHT;

    // Top border = top heating element, tiled across the full width.
    for (int col = 0; col < iconCols; col++)
    {
        OLED_DrawChar(iconX0 + col * ASCII_FONT_WIDTH, iconY0, topChar);
    }
    // Bottom border = bottom heating element, tiled across the full width.
    for (int col = 0; col < iconCols; col++)
    {
        OLED_DrawChar(iconX0 + col * ASCII_FONT_WIDTH, iconY0 + iconH - ASCII_FONT_HEIGHT, bottomChar);
    }
    // Left/right side borders, spanning just the interior rows (the top/bottom
    // hatch rows above already cover the corners).
    for (int y = iconY0 + ASCII_FONT_HEIGHT; y < iconY0 + iconH - ASCII_FONT_HEIGHT; y++)
    {
        OLED_SetPixel(iconX0, y, OLED_COLOR_WHITE);
        OLED_SetPixel(iconX0 + iconW - 1, y, OLED_COLOR_WHITE);
    }

    // Oven tray/rack graphic in the interior: a horizontal shelf line with a
    // few short tick marks hanging below it, like rack wires.
    int trayY = iconY0 + iconH / 2;
    for (int x = iconX0 + 3; x < iconX0 + iconW - 3; x++)
    {
        OLED_SetPixel(x, trayY, OLED_COLOR_WHITE);
    }
    for (int x = iconX0 + 4; x < iconX0 + iconW - 3; x += 3)
    {
        OLED_SetPixel(x, trayY + 1, OLED_COLOR_WHITE);
        OLED_SetPixel(x, trayY + 2, OLED_COLOR_WHITE);
    }

    // --- Text block, placed to the right of the icon, built from the real
    // `oven` struct.
    offset += sprintf(buffer + offset, "MODE: %s\n", modeNames[oven.mode]);

    // Cook time as MM:SS. Only bake mode shows the ">" selector indicator,
    // and only next to whichever setting (time/temp) is currently selected.
    uint16_t minutes = oven.cookTimeSec / 60;
    uint16_t seconds = oven.cookTimeSec % 60;
    const char *timeMarker = (oven.mode == MODE_BAKE && oven.selector == SELECT_TIME) ? ">" : "";
    offset += sprintf(buffer + offset, "%sTIME: %02u:%02u\n", timeMarker, minutes, seconds);

    // Temperature is not shown at all in toast mode.
    if (oven.mode != MODE_TOAST)
    {
        const char *tempMarker = (oven.mode == MODE_BAKE && oven.selector == SELECT_TEMP) ? ">" : "";
        offset += sprintf(buffer + offset, "%sTEMP: %u%s", tempMarker, oven.tempF, DEGREE_SYMBOL);
    }

    drawTextAt(iconX0 + iconW + 4, iconY0, buffer);

    OLED_Update();
}

// Advances the cooking countdown by one TIMER_TICK's worth of elapsed time:
// updates the remaining time and LED progress bar once a whole second has
// passed, and handles natural completion once the set time has fully
// elapsed. Shared by COOKING and RESET_PENDING, since cooking visually
// keeps going while we're deciding whether a BTN4 hold is a real cancel.
static void advanceCookingCountdown(void)
{
    uint16_t elapsedTicks = freeRunningCounter - oven.cookStartTime; // unsigned subtraction is rollover-safe
    uint16_t elapsedSec = elapsedTicks / TIM2_DEFAULT_FREQ_HZ;

    if (elapsedSec >= oven.initialCookTimeSec)
    {
        // Cooking finished naturally. Per the manual, this resyncs
        // whichever value the selector currently points to from the pot's
        // current position (unlike a cancel, which restores the exact
        // original value instead).
        if (oven.mode == MODE_BAKE && oven.selector == SELECT_TEMP)
        {
            oven.tempF = (uint16_t)lastAdcTop8 + 300;
        }
        else
        {
            oven.cookTimeSec = (uint16_t)lastAdcTop8 + 1;
        }
        LEDs_Set(0x00);
        // Extra credit: blink the display until dismissed, instead of
        // going straight back to SETUP.
        oven.state = ALERT;
        alertInverted = FALSE;
        updateOvenOLED();
    }
    else if (elapsedTicks % TIM2_DEFAULT_FREQ_HZ == 0)
    {
        // A new whole second has elapsed -- update the countdown display
        // and LED progress bar. (Gating on the modulus keeps us from
        // calling the slow updateOvenOLED() on every 5Hz tick when only
        // every 5th tick actually changes what's displayed.)
        oven.cookTimeSec = oven.initialCookTimeSec - elapsedSec;

        uint16_t ledsOff = (uint16_t)elapsedSec * 8 / oven.initialCookTimeSec; // multiply before divide
        if (ledsOff > 8)
        {
            ledsOff = 8;
        }
        LEDs_Set((uint8_t)(0xFF >> ledsOff));

        updateOvenOLED();
    }
}

/* This function will execute your state machine.
 * It should ONLY run if an event flag has been set.
 */
void runOvenSM(OvenEvent event)
{
    switch (oven.state)
    {
    case SETUP:
        if (event == EVENT_ADC_CHANGED)
        {
            // Top 8 bits of the 12-bit ADC reading.
            uint8_t top8 = (uint8_t)(rawAdcValue >> 4);

            // Only bake mode lets the selector pick between time and
            // temp; toast and broil always adjust time (toast has no
            // configurable temp, broil's temp is fixed).
            if (oven.mode == MODE_BAKE && oven.selector == SELECT_TEMP)
            {
                oven.tempF = (uint16_t)top8 + 300; // 300-555 F
            }
            else
            {
                oven.cookTimeSec = (uint16_t)top8 + 1; // 1-256 sec (0:01-4:16)
            }
            updateOvenOLED();
        }
        else if (event == EVENT_BTN3_DOWN)
        {
            // Snapshot when the press started, so SELECTOR_CHANGE_PENDING
            // can tell a short press from a long press on release.
            oven.buttonPressTime = freeRunningCounter;
            oven.state = SELECTOR_CHANGE_PENDING;
        }
        else if (event == EVENT_BTN4_DOWN)
        {
            // Start cooking: snapshot the set time as the total (for the
            // LED bar and for restoring on cancel) and the start tick
            // (for the countdown).
            oven.initialCookTimeSec = oven.cookTimeSec;
            oven.cookStartTime = freeRunningCounter;
            oven.state = COOKING;
            LEDs_Set(0xFF); // all LEDs on at the start of cooking
            updateOvenOLED();
        }
        break;

    case SELECTOR_CHANGE_PENDING:
        if (event == EVENT_BTN3_UP)
        {
            uint16_t elapsed = freeRunningCounter - oven.buttonPressTime; // unsigned subtraction is rollover-safe

            if (elapsed >= LONG_PRESS_TICKS)
            {
                // Long press: cycle to the next cook mode (bake -> toast -> broil -> bake).
                oven.mode = (CookMode)((oven.mode + 1) % 3);

                if (oven.mode == MODE_BAKE)
                {
                    oven.selector = SELECT_TIME; // selector always defaults to time entering bake
                }
                else if (oven.mode == MODE_BROIL)
                {
                    oven.tempF = 500; // broil's temp is fixed, not adjustable
                }
            }
            else
            {
                // Short press: toggle the selector between time and temp.
                oven.selector = (oven.selector == SELECT_TIME) ? SELECT_TEMP : SELECT_TIME;
            }

            oven.state = SETUP;
            updateOvenOLED();
        }
        break;

    case COOKING:
        if (event == EVENT_TIMER_TICK)
        {
            advanceCookingCountdown();
        }
        else if (event == EVENT_BTN4_DOWN)
        {
            // Snapshot the press so RESET_PENDING can tell a short tap
            // from a genuine long-press cancel.
            oven.buttonPressTime = freeRunningCounter;
            oven.state = RESET_PENDING;
        }
        break;

    case RESET_PENDING:
        if (event == EVENT_TIMER_TICK)
        {
            uint16_t held = freeRunningCounter - oven.buttonPressTime; // unsigned subtraction is rollover-safe

            if (held >= LONG_PRESS_TICKS)
            {
                // Cancel: restore the exact original set time (not the
                // pot's current position -- that's only for natural
                // completion), turn off the elements, and return to SETUP.
                oven.cookTimeSec = oven.initialCookTimeSec;
                LEDs_Set(0x00);
                oven.state = SETUP;
                updateOvenOLED();
            }
            else
            {
                // Not held long enough to count as a cancel yet --
                // cooking keeps going normally while we wait and see.
                advanceCookingCountdown();
            }
        }
        else if (event == EVENT_BTN4_UP)
        {
            // Released before the long-press threshold: not a cancel,
            // just resume cooking.
            oven.state = COOKING;
        }
        break;

    case ALERT:
        if (event == EVENT_TIMER_TICK)
        {
            // Blink by toggling the display's invert mode each tick.
            // TIM2 only ticks at 5Hz, so this gives ~2.5Hz rather than
            // an exact 2Hz -- the closest clean approximation available
            // from our only tick source.
            alertInverted = !alertInverted;
            if (alertInverted)
            {
                OLED_SetDisplayInverted();
            }
            else
            {
                OLED_SetDisplayNormal();
            }
        }
        else if (event == EVENT_BTN3_DOWN || event == EVENT_BTN4_DOWN)
        {
            // Any button press acknowledges the alert and dismisses it.
            OLED_SetDisplayNormal();
            alertInverted = FALSE;
            oven.state = SETUP;
            updateOvenOLED();
        }
        break;

    // All four states are handled above. This default only exists as a
    // safety net and should be unreachable in practice.
    default:
        break;
    }
}

int main()
{
    BOARD_Init();
    Buttons_Init();
    LEDs_Init();
    Timers_Init();
    // Slow TIM4 down from its 1kHz default to ~50Hz. At 1kHz, a TIM4
    // interrupt (which kicks off the next ADC conversion) was landing in the
    // middle of OLED_Update()'s blocking I2C transfer often enough to break
    // it. 50Hz is still far faster than the knob needs and avoids that.
    Timers_ConfigTimer(&htim4, 99, 19999); // 100MHz / (100 * 20000) = 50Hz
    // Using single-shot interrupt mode with manual re-triggering (see
    // HAL_ADC_ConvCpltCallback below), rather than continuous
    // interrupt/watchdog modes -- both continuous modes never fired their
    // interrupt at all on this board/driver, while single-shot interrupt
    // does reliably.
    ADC_Init(ADC_SINGLE_SHOT_INTERRUPT);
    OLED_Init();
    ADC_Start(); // kick off the first conversion; each ISR re-triggers the next one.

    printf(
        "Welcome to CRUZID's Lab08 (Toaster Oven)."
        "Compiled on %s %s.\n\r",
        __TIME__,
        __DATE__);

    // Initialize state machine (and anything else you need to init) here.
    oven.state = SETUP;
    oven.mode = MODE_BAKE;
    oven.selector = SELECT_TIME; // selector always defaults to time when entering bake mode
    oven.cookTimeSec = 1;        // 0:01, per lab defaults
    oven.initialCookTimeSec = 1;
    oven.tempF = 350; // bake mode default temp
    oven.buttonPressTime = 0;
    oven.cookStartTime = 0;

    LEDs_Set(0x00); // all off until cooking starts

    // Draw the initial SETUP screen. Necessary once here at boot since
    // nothing else has happened yet to trigger a redraw via runOvenSM() --
    // all later redraws happen only in response to real events, never
    // called directly from the main loop.
    updateOvenOLED();

    while (1)
    {
        // Add main loop code here:
        // check for events
        // on event, run runOvenSM()
        // clear event flags
        if (adcChangedEvent)
        {
            // Clear the flag BEFORE processing, not after. updateOvenOLED()
            // (called inside runOvenSM()) is slow -- if we cleared the flag
            // afterward and a new ADC interrupt fired mid-update, that clear
            // would wipe out the new event and we'd silently miss a pot
            // change. Clearing first means a same-tick re-trigger just gets
            // picked up on the next loop iteration instead of getting lost.
            adcChangedEvent = FALSE;
            runOvenSM(EVENT_ADC_CHANGED);
        }

        // Button events are accumulated as a bitmask (multiple can arrive in
        // the same TIM3 tick). Handle each bit independently, clearing just
        // that bit before processing it -- same reasoning as adcChangedEvent
        // above, so a new event of a *different* type isn't lost while we're
        // mid-update on this one.
        if (buttonEventFlags & BUTTON_EVENT_3DOWN)
        {
            buttonEventFlags &= ~BUTTON_EVENT_3DOWN;
            runOvenSM(EVENT_BTN3_DOWN);
        }
        if (buttonEventFlags & BUTTON_EVENT_3UP)
        {
            buttonEventFlags &= ~BUTTON_EVENT_3UP;
            runOvenSM(EVENT_BTN3_UP);
        }
        if (buttonEventFlags & BUTTON_EVENT_4DOWN)
        {
            buttonEventFlags &= ~BUTTON_EVENT_4DOWN;
            runOvenSM(EVENT_BTN4_DOWN);
        }
        if (buttonEventFlags & BUTTON_EVENT_4UP)
        {
            buttonEventFlags &= ~BUTTON_EVENT_4UP;
            runOvenSM(EVENT_BTN4_UP);
        }

        if (timerTickEvent)
        {
            timerTickEvent = FALSE;
            runOvenSM(EVENT_TIMER_TICK);
        }
    };

    BOARD_End();
    while (1)
        ;
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
        freeRunningCounter++;
        timerTickEvent = TRUE;
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
        // TIM3 is used exclusively for polling button events, per the manual.
        buttonEventFlags |= Buttons_CheckEvents();
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
        // Paces ADC sampling at a controlled 50Hz, instead of the ADC
        // conversion-complete callback immediately retriggering itself as
        // fast as possible (which was flooding the I2C bus and breaking the
        // OLED).
        ADC_Start();
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
        uint16_t value = HAL_ADC_GetValue(&hadc1);
        uint8_t top8 = (uint8_t)(value >> 4);

        // This callback fires on every completed conversion, far more often
        // than the top-8-bit value we actually care about changes. Only
        // raise the event (and stash the raw value for the state machine to
        // use) when the value we'd actually act on has moved.
        if (top8 != lastAdcTop8)
        {
            lastAdcTop8 = top8;
            rawAdcValue = value;
            adcChangedEvent = TRUE;
        }
        // NOTE: the next conversion is kicked off by TIM4 (50Hz) instead of
        // immediately here -- retriggering back-to-back with no pacing ran
        // the ADC at an extremely high, uncontrolled rate, which was
        // flooding the I2C bus and breaking the OLED's transfers.
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
        // Unused: we use ADC_SINGLE_SHOT_INTERRUPT + HAL_ADC_ConvCpltCallback()
        // instead. The watchdog modes on this board never actually fired
        // their interrupt in testing; see the writeup for details.
        /***************************************************************************
         * Your code goes in between this comment and the preceding one with
         * asterisks.
         **************************************************************************/
    }
}