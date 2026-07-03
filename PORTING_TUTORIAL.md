# From Classic C to TwinCAT C++ — the Short Guide

Seven steps. For a cleanly written C codebase this is under an hour; the linker
does most of the audit for you. Worked example throughout this repo:
[`LegacyFirmware_Original\`](LegacyFirmware_Original/README.md) (before) →
[`MyCPP Versioned\`](TwinCAT_CPP_Retrofit_Classic_C/MyCPP%20Versioned) (after).
Diagrams: [`docs\RetrofitDiagrams.drawio`](docs/RetrofitDiagrams.drawio)
(open with draw.io / diagrams.net — 4 pages).

**Prerequisites:** a TwinCAT C++ project (versioned recommended), Visual Studio
with the TC1300 C++ workload, a TwinCAT user certificate for signing.

---

### Step 1 — Audit: list what isn't portable C

Portable C99 compiles **as-is**: `string.h`, `stdlib.h`, `math.h`, `stdint.h`
all exist in TwinCAT's real-time C runtime — even `malloc`. Only list the rest;
that list is your entire port:

| Found in the C code             | Becomes in TwinCAT                          |
|---------------------------------|---------------------------------------------|
| `main()` loop / timer ISR       | `CycleUpdate()` (called every task cycle)   |
| HAL / register reads & writes   | Data areas (`m_pInputs` / `m_pOutputs`)     |
| `printf` / UART logging         | `m_Trace.Log(tlInfo, FLEAVEA "...")`        |
| `fopen`/`fread`…                | `ITcFileAccess`, or drop it                 |
| Threads, `sleep`, RTOS calls    | Don't exist — everything is cyclic          |
| Config from EEPROM / `#define`  | TcCOM module parameters (TMC)               |

Don't audit by reading every line — step 7's linker errors are the complete,
machine-generated version of this list.

### Step 2 — Copy the files in, unchanged

Copy the `.c`/`.h` files into a subfolder of the C++ project (here: `legacy\`),
then in Visual Studio: right-click the project → *Add → Existing Item*.
MSVC compiles `.c` files as C automatically — no renaming to `.cpp`.

### Step 3 — The one required setting

For **each `.c` file**: *Properties → C/C++ → Precompiled Headers →*
**Not Using Precompiled Headers**. (A C file can't consume the project's C++
`TcPch.h` precompiled header; without this you get C1010/C1853.) In the
`.vcxproj` that's simply:

```xml
<ClCompile Include="legacy\legacy_pid.c">
  <PrecompiledHeader>NotUsing</PrecompiledHeader>
</ClCompile>
```

### Step 4 — Include with C linkage

In the module header ([CallClassicC.h](TwinCAT_CPP_Retrofit_Classic_C/MyCPP%20Versioned/CallClassicC.h)):

```cpp
extern "C" {
#include "legacy/legacy_pid.h"
#include "legacy/legacy_temp_seq.h"
}
```

The customer's headers stay untouched.

### Step 5 — Give the state a home

The C code's context structs become **module members** (never file-scope
statics — those reset on an online change):

```cpp
pid_ctx_t  m_Pid;      // in the module class
tseq_ctx_t m_Seq;
```

Initialize them in the PREOP→SAFEOP transition (`SetObjStatePS`), feeding
tuning values from TMC module parameters:

```cpp
pid_init(&m_Pid, m_Parameter.Kp, m_Parameter.Ki, m_Parameter.Kd, 0.0, 100.0);
```

### Step 6 — Call it from CycleUpdate()

The task cycle *is* the old timer tick. Data-area reads/writes replace the HAL:

```cpp
ULONG ns = 0;
ipTask->GetCycleTime(&ns);
double dt = ns * 1e-9;                       // the legacy code's "tick"

double power = pid_step(&m_Pid, m_pInputs->SetpointTemp,
                        m_pInputs->ActualTemp, dt);
m_pOutputs->HeaterPower = power;
```

### Step 7 — Build, and let the linker finish the audit

Build `Release | TwinCAT RT (x64)`. Every `unresolved external symbol` is one
line from the Step-1 table — stub it, replace it, or port it, then rebuild
until the signed TMX drops out. Nothing unsupported fails silently at runtime:
**if it links, the calls exist.**

---

### Keep it testable without a PLC

Keep the C core runnable on a desktop, like
[`LegacyFirmware_Original\build_sim.ps1`](LegacyFirmware_Original/build_sim.ps1):
the same `.c` files plus a simulation HAL double as a regression test for the
port — same inputs, same outputs, before and after.

### Rules of thumb for the real-time context

- Allocate in state transitions, not per-cycle (`malloc` in RT = router memory).
- Keep per-cycle work bounded; task stack defaults to 64 KB.
- 32-bit x86 target only: TwinCAT compiles `/Gz` (stdcall default) — check
  function-pointer tables. Irrelevant on x64.
- Beckhoff doesn't officially document plain-`.c` support (it works; all their
  samples are C++). If vendor supportability matters, get it in writing.
