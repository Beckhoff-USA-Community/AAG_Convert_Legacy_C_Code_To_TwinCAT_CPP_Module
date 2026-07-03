# Heater Controller Firmware — the "Before" Project

This folder plays the role of **the customer's original embedded project**: a
bare-metal C heater controller as it existed before any TwinCAT involvement.
It is the starting point of the retrofit story told in [../DEMO.md](../DEMO.md).

```
src\
├── main.c              1 kHz super-loop: read HAL → state machine → PID → write HAL
├── hal.h               the hardware boundary (9 functions)
├── hal_target.c        register-level HAL for the real board (reference only)
├── legacy_pid.c/.h     PID controller          ─┐ the portable control core:
└── legacy_temp_seq.c/.h supervision state machine ─┘ no OS, no HAL, no TwinCAT
sim\
└── hal_sim.c           PC build of the HAL: simulated plant + scripted operator
```

## Run it on a PC

```powershell
.\build_sim.ps1 -Run
```

Compiles the firmware **unmodified** against the simulation HAL and runs a
140-second scripted scenario (enable → ramp to 80 °C → setpoint 120 °C forces
an overtemp trip at 110 °C → cool-down → operator fault reset → recovery),
printing a CSV trace plus a line per state change.

## The point of the exercise

The architecture already separates the portable core from the hardware:

- `legacy_pid.c` / `legacy_temp_seq.c` know nothing about the board. **These
  move to TwinCAT byte-for-byte identical** — compare with
  `..\TwinCAT_CPP_Retrofit_Classic_C\MyCPP Versioned\legacy\`:

  ```powershell
  fc.exe /b src\legacy_pid.c "..\TwinCAT_CPP_Retrofit_Classic_C\MyCPP Versioned\legacy\legacy_pid.c"
  ```

- `main.c` + `hal_target.c` are the hardware-specific ~150 lines. In the
  TwinCAT retrofit their jobs are taken over by the TcCOM module
  (`CallClassicC.cpp`): the super-loop tick becomes `CycleUpdate()`, the
  ADC/PWM registers become mapped data areas, EEPROM tuning constants become
  module parameters, and the watchdog belongs to the TwinCAT runtime.

That replacement — not a rewrite of the control logic — is the entire port.
