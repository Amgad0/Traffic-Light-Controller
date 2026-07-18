# Traffic Light Controller — TM4C123 (Tiva C)

An interrupt-driven traffic light and pedestrian crossing controller for the **TI TM4C123GH6PM (Tiva C) LaunchPad**, written in bare-metal C against **TivaWare™ Peripheral Driver Library**. The system models a two-way road intersection: one set of car lights for the North–South approach, one for East–West, each paired with a pedestrian signal and a push-button "beg button" that interrupts the normal cycle to let pedestrians cross.

Originally built as part of a university embedded-systems course group project; the toolchain migration to TI Arm Clang, the bug fixes, and the refactoring documented below are my own subsequent work.

## Table of Contents

- [Features](#features)
- [Hardware](#hardware)
- [System Architecture](#system-architecture)
- [Module Reference](#module-reference)
- [Building & Flashing](#building--flashing)
- [Bug Fixes & Further Improvements](#bug-fixes--further-improvements)

## Features

- **Two independent vehicle traffic lights** cycling Green (5 s) → Yellow (2 s) → Red, with the opposite light going green exactly 1 second after the active one turns red.
- **Two pedestrian crossings**, each with its own push button and Red/Green LED pair. Pressing a button interrupts the active vehicle cycle, turns the corresponding pedestrian light green for 2 seconds, then returns control to the interrupted sequence.
- **Fully interrupt-driven**: `main()` only performs peripheral setup — there is no polling super-loop. All runtime behavior is driven by a periodic timer interrupt and two GPIO edge interrupts.
- Built entirely on **TivaWare driverlib**, aside from a couple of low-level register accesses documented in [Bug Fixes & Further Improvements](#bug-fixes--further-improvements).

Not implemented (called out as bonus/stretch goals in the original spec): simultaneous two-pedestrian arbitration, debounced repeated-press handling, request queuing after the crossing window, and UART state broadcasting.

## Hardware

| Target | TM4C123GH6PM (ARM Cortex-M4F), EK-TM4C123GXL LaunchPad |
|---|---|
| Toolchain | Code Composer Studio, TI Arm Clang (`ti-cgt-armllvm` 5.1.1 LTS) |
| Driver library | TivaWare_C_Series-2.2.0.295 |
| Clock source | Timer0A / Timer1A on the system clock |

### Pin map (as configured in `Port_Init`)

| Port | Pins | Function |
|---|---|---|
| `PORTA` | PA2, PA3, PA4 | Traffic 1 red LED + Pedestrian 1 red/green LEDs |
| `PORTB` | PB2, PB3 | Pedestrian LEDs (red/green) |
| `PORTC` | PC4–PC7 | Traffic 1 green LED + Traffic 2 red/yellow/green LEDs |
| `PORTD` | PD6 | Traffic 1 yellow LED |
| `PORTF` | PF0, PF4 | Onboard push buttons (SW2 / SW1), configured as inputs with weak pull-ups, falling-edge interrupt |

## System Architecture

There is no `while(1)` loop in application code. `main()` (`main.c`) performs three initialization calls and returns; from that point on, the whole system is a pair of cooperating ISRs:

```
main()
 ├─ Port_Init()       // clock-enable + pin-direction setup for GPIOA/B/C/D/F
 ├─ Init_TimerInt()   // configure Timer0A as a 1 Hz periodic interrupt source
 └─ GPIO_Int()        // arm falling-edge interrupts on PF0 / PF4

Timer0A ISR (1 Hz tick)                  GPIOF ISR (button press)
  Timer0_Handler()                         Pedestrian_Crossing()
  ├─ drives a counter/switch_traffic       ├─ reads which button fired
  │  state machine that steps each          ├─ forces the relevant pedestrian
  │  traffic light through                    light green / car light red
  │  Green(5s) → Yellow(2s) → Red(+1s)      ├─ busy-waits ~2s via Timer_Delay()
  └─ alternates between traffic 1 and 2     └─ restores the pedestrian light to red
```

The normal-cycle timing (5 s / 2 s / 1 s) is implemented entirely by counting ticks of a single 1-second periodic timer (`Timer0A`) inside `Timer0_Handler`, using two file-scope state variables:

- `counter` — ticks elapsed in the current phase (0–7 per half-cycle: 5 s green + 2 s yellow + 1 s all-red gap).
- `switch_traffic` — which of the two traffic lights is currently in its active phase.

Pedestrian requests are handled asynchronously via GPIO interrupts on the LaunchPad's onboard buttons (`PF0`, `PF4`), rather than being polled from the main cycle. The pedestrian ISR uses a second, independent timer (`Timer1A`) purely as a blocking 1-second delay primitive (`Timer_Delay`, called twice for the 2-second crossing window).

## Module Reference

### `main.c`
Composition root. Calls the two drivers' init routines and returns — by design, everything after this point runs from interrupt context.

### `GPIO_INIT.h` / `GPIO_INIT.c` — GPIO driver
Owns all pin configuration and the pedestrian-button interrupt path.

- **`Port_Init(void)`** — enables the clocks for GPIO ports A, B, C, D, F and blocks on `SysCtlPeripheralReady` for each; configures the LED pins listed in the [pin map](#pin-map-as-configured-in-port_init) as outputs and the two onboard buttons as inputs with internal weak pull-ups.
- **`GPIO_Int(void)`** — disables/clears any pending interrupt on `PF0`/`PF4`, sets both to falling-edge triggering, registers `Pedestrian_Crossing` as the ISR, and enables the interrupt.
- **`Pedestrian_Crossing(void)`** — the GPIOF ISR. Determines which button fired, forces the corresponding car light to red and the pedestrian light to green, blocks for ~2 seconds via `Timer_Delay()`, then returns the pedestrian light to red and clears the interrupt flag.

### `GPTM.h` / `GPTM.c` — General-Purpose Timer driver
Owns the periodic tick that drives the main traffic-light state machine, plus a secondary timer used as a blocking delay.

- **`Init_TimerInt(void)`** — enables Timer0, configures Timer0A as a 32-bit periodic up-counter with a load value tuned to fire once per second, registers `Timer0_Handler` as the ISR, and starts the timer.
- **`Timer0_Handler(void)`** — the 1 Hz ISR. On every tick it increments `counter` and, based on `counter` and `switch_traffic`, drives the Green→Yellow→Red sequence for whichever traffic light is currently active, then hands off to the other light once a full 8-second half-cycle has elapsed.
- **`Timer_Delay(void)`** — configures and starts Timer1A as a one-shot ~1-second timer and busy-waits until it fires; used by `Pedestrian_Crossing` as a simple blocking delay primitive (called twice for a 2-second pedestrian window).

### `tm4c123gh6pm_startup_ccs.c` / `tm4c123gh6pm.cmd`
TI-generated startup code (vector table, reset handler) and linker command file for the TM4C123GH6PM, provided by the CCS project template. `tm4c123gh6pm.cmd` is unmodified; the startup file needed one change to build correctly under TI Arm Clang — see [Bug Fixes & Further Improvements](#bug-fixes--further-improvements).

## Building & Flashing

This is a **Code Composer Studio** managed-build project (Eclipse CDT), not a standalone Makefile/CMake project. It builds with the **TI Arm Clang** compiler (`ti-cgt-armllvm`), which ships with current CCS releases.

1. Install [Code Composer Studio](https://www.ti.com/tool/CCSTUDIO) (bundles TI Arm Clang) with support for TM4C12x devices, plus **TivaWare_C_Series-2.2.0.295**.
2. `File → Import → Code Composer Studio → CCS Projects`, select this directory.
3. Project settings expect TivaWare at `C:\ti\TivaWare_C_Series-2.2.0.295` — the compiler include path (`-I`), predefined symbols (`PART_TM4C123GH6PM`, `TARGET_IS_TM4C123_RB1`), and the linked driverlib build (`driverlib\gcc\libdriver.a`, the TI Arm Clang–compatible variant — **not** the classic-compiler `driverlib\ccs\Debug\driverlib.lib`) are all set in the project's CCS Build settings; adjust them if TivaWare is installed elsewhere.
4. Build (`Project → Build`), then flash/debug onto an EK-TM4C123GXL LaunchPad via its onboard XDS110 debug probe.

## Bug Fixes & Further Improvements

### Fixed in this repository

1. **Vector table misplaced under TI Arm Clang.** The startup file placed `g_pfnVectors` with the legacy `#pragma DATA_SECTION(g_pfnVectors, ".intvecs")`. TI Arm Clang doesn't support that pragma and silently drops it, so the vector table landed wherever the compiler's default section allocation put it instead of address `0x00000000` — an image that links without error but doesn't boot. Fixed by switching to `__attribute__((section(".intvecs")))`, which the linker now places correctly at `0x0` (verified against the link map).

### Known issues (not yet fixed)

These were found by reading the code, not by running it on hardware.

1. **`PF0` (SW2) button doesn't respond.** `PF0` shares its pin with the TM4C123's NMI function and is **locked by default**; `Port_Init` never unlocks it. This is a well-known Tiva C gotcha (shared by `PD7`) and the likely root cause of the button-1 failure noted in the original team's submission.

2. **`Pedestrian_Crossing`'s second branch reads the wrong pin.** It tests `GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3)` — `PA3` is configured as an **LED output**, not the second push button (`PF4`). As written, the branch reacts to an LED's output state rather than a button press.

3. **`Timer_Delay` polls the wrong timer's registers.** It configures and enables `TIMER1_BASE`, but clears/polls `TIMER0_ICR_R` / `TIMER0_RIS_R` directly — registers belonging to Timer0, the independent 1 Hz system tick. The delay only appears to work because Timer0 happens to also cycle roughly once per second; it isn't actually gated on Timer1 at all.

### Further improvements (not yet applied)

- **Blocking delay inside an ISR.** `Pedestrian_Crossing` busy-waits for ~2 seconds inside GPIO interrupt context, holding off any interrupt of equal or lower priority for that entire window. A cleaner design would fold the pedestrian phase into the same `Timer0_Handler` tick-driven state machine (e.g. a `pedestrian_request` flag consumed on the next 1 Hz tick), removing the second timer and the nested blocking wait entirely.

- **Resume semantics don't match the spec.** The assignment calls for resuming the *remaining* seconds of the interrupted vehicle phase after a pedestrian crossing (e.g., interrupted after 2 of 5 green seconds → resume with 3 seconds left). The current implementation just delays 2 seconds and lets the normal cycle carry on from wherever `counter` happens to be, rather than accounting for elapsed time.

- **Magic numbers throughout.** Pin masks, the `16000000` timer load values (implicitly assuming the default 16 MHz clock), and phase durations (`5`, `2`, `7`, `8`) are inlined rather than named. Naming these would make the state machine in `Timer0_Handler` considerably easier to follow and to retune.

- **Non-`volatile` shared state.** `counter` and `switch_traffic` (`GPTM.c`) are written from `Timer0_Handler` and read across ISR/refactor boundaries without a `volatile` qualifier — harmless today given how the compiler happens to treat them, but worth adding for correctness if the code is optimized or restructured.

- **Bonus features from the spec are unimplemented**: concurrent pedestrian requests on both crossings, debounced multi-press handling within one crossing window, deferred requests made just after a crossing window closes, and UART broadcast of system state on every transition.
