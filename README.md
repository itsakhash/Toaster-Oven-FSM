# Toaster Oven FSM

A toaster oven controller implemented as a software finite-state machine in C on the STM32 NUCLEO-F411RE. The system uses an event-driven pattern where lightweight interrupt handlers only set flags, and the main loop polls those flags to drive the state machine — no logic runs inside the interrupts themselves.

## Features

- **Four core states** — `SETUP`, `SELECTOR_CHANGE_PENDING`, `COOKING`, and `RESET_PENDING`, plus an additional `ALERT` state for extra credit
- **Dual-purpose buttons** — the same short-press/long-press pattern is reused across two different controls: one button cycles between time/temperature mode on a short press but toggles the active selector on a long press, while another starts/cancels cooking on a short press but resumes a paused cook on a long press
- **Timer-driven architecture** — a 100Hz timer polls buttons every cycle, while a separate 5Hz timer drives a free-running counter used both to distinguish short vs. long presses and to time the cook countdown
- **ADC-based input** — a potentiometer is sampled to set either cook time or target temperature, depending on the currently selected mode
- **OLED status display** — shows the oven's current state, selected mode, and countdown in real time
- **Alert state (extra credit)** — automatically triggered when cooking finishes, blinking the display via alternating display-inversion calls until dismissed by any button press

## Architecture

The state machine closely follows the classic event-driven embedded pattern:

1. Interrupt Service Routines (ISRs) only set flags in response to hardware events (timer ticks, button presses)
2. The main loop polls those flags and, when set, feeds the corresponding event into the state machine
3. The state machine's transition logic is implemented as a single `switch` statement mapping (state, event) pairs to the next state and any side effects (starting the countdown, updating the display, etc.)

This keeps all interrupt handlers minimal and fast, while the actual decision-making logic runs safely in the main loop.

## Hardware

- STM32 NUCLEO-F411RE development board
- UCSC ECE13 I/O shield (buttons, OLED display, potentiometer)

## Repository Structure

    .
    ├── Common/          # Shared course libraries (BOARD, Buttons, Timers, Oled, Adc, etc.)
    └── Lab08/
        ├── src/         # Toaster oven FSM implementation
        ├── templates/   # Starter template file provided for the assignment
        └── platformio.ini

The project depends on `Common` via a relative path, so both folders must remain siblings for the build to work correctly.

## Building and Running

This project uses [PlatformIO](https://platformio.org/). From inside `Lab08/`:

```
pio run --target upload
```

Then open a serial monitor at the configured baud rate to view any debug output, or observe the OLED directly on the physical board.

## Notable Implementation Details

- The potentiometer originally used PlatformIO's continuous ADC interrupt/watchdog modes, but neither mode's callback ever fired on this specific board. Switching to a single-shot interrupt mode, with the ISR manually re-triggering each new conversion, resolved the issue.
- The alert state's blink rate is tied to the only available periodic tick source (the 5Hz timer), toggling the display's inversion on every tick — giving an effective blink rate of roughly 2.5Hz rather than an exact round number, since no dedicated timer was reserved for this purpose.
