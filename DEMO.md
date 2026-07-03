# Demo: Legacy Embedded C inside TwinCAT 3 C++ — Unchanged

**The claim to the customer:** portable embedded C code compiles *unchanged* into a
TwinCAT 3 C++ real-time module. One project setting per C file, a thin adapter in the
module's cyclic method — that's the whole retrofit.

**What this demo shows:** a heater temperature controller whose entire control logic
(PID + supervision state machine) is classic embedded C, running in hard real time on
the CX, closing a loop against a simulated plant. No hardware needed.

**Companion material:** the generic how-to is in
[PORTING_TUTORIAL.md](PORTING_TUTORIAL.md) (7 steps); diagrams — before/after
architecture, porting flowchart, state machine, cycle data flow — are in
[docs\RetrofitDiagrams.drawio](docs/RetrofitDiagrams.drawio) (open with
draw.io / diagrams.net, 4 pages).

---

## Act 0 — the "before": the customer's original firmware

[`LegacyFirmware_Original\`](LegacyFirmware_Original/README.md) is the customer's
project as it existed before TwinCAT: a bare-metal 1 kHz super-loop (`main.c`), a
register-level HAL (`hal_target.c`), and the portable control core
(`legacy_pid.c`, `legacy_temp_seq.c`). It even runs on a laptop:

```powershell
cd LegacyFirmware_Original; .\build_sim.ps1 -Run
```

...which prints the full scripted scenario (ramp to 80 °C → overtemp trip at 110 °C
→ fault reset → recovery) from the unmodified firmware logic.

**The key exhibit:** the control-core files in the TwinCAT project are
*byte-for-byte identical* to the firmware originals — prove it live:

```powershell
fc.exe /b "LegacyFirmware_Original\src\legacy_pid.c" "TwinCAT_CPP_Retrofit_Classic_C\MyCPP Versioned\legacy\legacy_pid.c"
```

Only `main.c` and the HAL — the parts that were always hardware-specific — get
replaced by the TcCOM module.

## What's in the TwinCAT project

```
MyCPP Versioned\
├── legacy\                      <-- the customer's code, copied UNCHANGED from
│   ├── legacy_pid.c/.h              LegacyFirmware_Original\src\ (PID with
│   └── legacy_temp_seq.c/.h         anti-windup; state machine w/ latched FAULT)
├── CallClassicC.cpp/.h          TcCOM module = replaces main.c + HAL: CycleUpdate()
│                                calls the C code and simulates the heater plant
└── MyCPP_Versioned.tmc          Module description: I/O data areas + PID gain parameters
```

The mapping from embedded-land to TwinCAT-land (compare the actual files —
`main.c`/`hal_target.c` vs `CallClassicC.cpp`):

| Original firmware                          | TwinCAT equivalent                             |
|--------------------------------------------|------------------------------------------------|
| `hal_wait_tick()` — 1 kHz super-loop tick  | `CycleUpdate()` (task cycle, e.g. 1 ms)        |
| `hal_read_temp()` etc. — ADC/GPIO registers| Mapped input data area (`m_pInputs`)           |
| `hal_set_heater()` — PWM register          | Mapped output data area (`m_pOutputs`)         |
| `CFG_KP` … from EEPROM                     | TcCOM module parameters (Kp, Ki, Kd, limit)    |
| `hal_kick_watchdog()`                      | TwinCAT runtime supervision                    |
| `printf` debugging                         | `m_Trace.Log(...)` → TwinCAT Engineering       |

## The two (and only two) integration changes

1. **Per C file:** *Properties → C/C++ → Precompiled Headers → Not Using Precompiled
   Headers*. (A C translation unit can't consume the C++ `TcPch.h` precompiled header.
   In the `.vcxproj` this is just `<PrecompiledHeader>NotUsing</PrecompiledHeader>`.)
2. **In the module:** include the C headers with C linkage —
   `extern "C" { #include "legacy/legacy_pid.h" }` — and call the functions from
   `CycleUpdate()`. The legacy files themselves are untouched.

MSVC automatically compiles `.c` files as C (`/TC`), against Beckhoff's own real-time
C runtime (`string.h`, `stdlib.h`, `math.h`, `stdint.h`, `snprintf`, even `malloc`
backed by router memory). Anything genuinely unsupported (`printf`, `fopen`, threads,
`exit`) fails **at link time** — nothing fails silently at runtime, so a porting audit
is literally "build and read the linker output."

## Running the demo

Prereqs: the module builds and signs already (`Release | TwinCAT RT (x64)` →
`_products\...\MyCPP_Versioned.tmx`). Target: CX-3AB73C (192.168.40.111.1.1).

1. **Wire it into the TwinCAT project** (once): in the TwinCAT solution choose the
   target, then under `SYSTEM → TcCOM Objects` right-click → *Add New Item* →
   `C++ Modules → CallClassicC`. Create a 1 ms task (`SYSTEM → Tasks`) and set the
   module instance's *Context* to it. No I/O mapping needed — the plant is simulated.
2. **Activate** the configuration on the CX and set the runtime to Run.
3. **Show the live loop:** open the module instance's *Data Area* tab (or add the
   symbols to a Watch / TwinCAT Scope):
   - Set `Inputs.SetpointTemp` = `80`, `Inputs.Enable` = `1`.
   - Watch `Outputs.ActualTemp` ramp from 25 °C to 80 °C, `Outputs.HeaterPower`
     modulate, and `Outputs.CtrlState` go `0 (IDLE) → 1 (RAMP) → 2 (TRACK)`.
4. **Show the supervision (the state machine is also legacy C):**
   - Set `SetpointTemp` = `120`. The trip limit defaults to 110 °C: watch
     `AlarmFlags` bit 1 (near-limit warning) come on, then the latched trip —
     `CtrlState` = `3 (FAULT)`, heater forced off, temperature decays.
   - Recovery: set setpoint back to `80`, wait until the temperature is below
     100 °C, then pulse `Inputs.FaultReset` (write `1`, then back to `0`).
     The reset is **edge-triggered** — a held or stuck-high reset input cannot
     defeat the latch, which is itself a nice talking point about the quality
     of the legacy code.
5. **Show the tunables:** Kp/Ki/Kd/TempHighLimit are instance parameters
   (*Parameter (Init)* tab). Zeros mean "use built-in defaults" (Kp 8, Ki 0.5,
   Kd 0, limit 110 °C).

Cheat sheet — `CtrlState`: 0 IDLE, 1 RAMP, 2 TRACK, 3 FAULT.
`AlarmFlags`: bit 0 = latched overtemp trip, bit 1 = within 5 °C of the limit.

## Talking points for the customer

- **The C files have no idea they're in TwinCAT.** No Beckhoff include, no compiler
  pragma, no renamed function. They'd still compile on the old micro tomorrow.
- **State lives in context structs, not statics** — the module owns `pid_ctx_t` /
  `tseq_ctx_t`. Most well-written embedded C already works this way; code that
  hides state in file-scope statics still works, but context structs are what make
  the *next* step (online change with state carry-over) clean.
- **Real-time rules are enforced by the toolchain**: kernel context, no OS calls, a
  vetted C runtime. The linker is the gatekeeper.
- **This project is already online-change capable** (versioned TMX), and the state
  handover is fully wired: `PrepareOnlineChange` carries the parameters,
  `PerformOnlineChange` pulls the cycle counter **and the entire live controller
  state** (PID integrator, state machine incl. latched faults, plant state) out of
  the old instance via a dedicated transfer block (`PID_CallClassicCXferState`).
  The natural follow-up demo: change the legacy C, bump the version, and swap it in
  while the machine keeps running — temperature and state continue seamlessly.

## Honest caveats (worth stating before the customer finds them)

- Beckhoff's docs never explicitly document adding plain `.c` files — everything in
  their samples is C++. It works cleanly (their CRT headers are deliberately
  C-compatible), but if vendor supportability is a requirement, get it in writing
  from Beckhoff support.
- Code that calls `printf`/`fopen`/threads/OS APIs needs stubs or a thin port layer —
  budget for it in the audit.
- On the x86 (32-bit) platform only: TwinCAT compiles with `/Gz` (stdcall default);
  C code with function-pointer tables may need calling-convention annotations.
  Irrelevant on x64 targets like the CX.
- Deep call chains / big stack arrays: the real-time task stack defaults to 64 KB
  (configurable under Real-Time → Settings).
