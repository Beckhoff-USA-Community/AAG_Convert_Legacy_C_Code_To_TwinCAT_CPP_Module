# About This Repository

This repository demonstrates how **classic embedded C code can be reused
unchanged inside a TwinCAT 3 C++ (TcCOM) real-time module** — the typical
retrofit question: *"Can we keep our proven C firmware logic when we move to
TwinCAT?"* The answer here is a complete, working before-and-after:

| Folder / file | What it is |
|---|---|
| [`LegacyFirmware_Original\`](LegacyFirmware_Original/README.md) | The "before": a bare-metal C heater controller (1 kHz super-loop, register HAL, portable PID + state-machine core). Runs on a PC via `build_sim.ps1 -Run`. |
| [`TwinCAT_CPP_Retrofit_Classic_C\`](TwinCAT_CPP_Retrofit_Classic_C) | The "after": a versioned (online-change capable) TwinCAT C++ solution whose TcCOM module compiles the **byte-identical** C core and closes the loop against a simulated plant. |
| [`PORTING_TUTORIAL.md`](PORTING_TUTORIAL.md) | The generic 7-step guide for porting your own C code. |
| [`DEMO.md`](DEMO.md) | The step-by-step demo script (build, activate, live values, talking points). |
| [`docs\RetrofitDiagrams.drawio`](docs/RetrofitDiagrams.drawio) | Diagrams: before/after architecture, porting flowchart, state machine, cyclic data flow (open with [draw.io](https://www.drawio.com/)). |

The key takeaways shown by the sample: portable C99 compiles as-is against the
TwinCAT real-time C runtime; per `.c` file only one project setting is needed
(*Precompiled Headers → Not Using*); the embedded main loop maps to
`CycleUpdate()` and the HAL maps to TMC data areas; and anything unsupported
fails at **link time**, never silently at runtime.

All sample code provided by [Beckhoff Automation LLC](https://www.beckhoff.com/en-us/)
are for illustrative purposes only and are provided "as is" and without any
warranties, express or implied. Actual implementations in applications will
vary significantly. Beckhoff Automation LLC shall have no liability for, and
does not waive any rights in relation to, any code samples that it provides or
the use of such code samples for any purpose.

The sample code is provided as-is under the [Zero-Clause BSD license](LICENSE).

# How to get support

Should you have any questions regarding the provided sample code, please
contact your local Beckhoff support team. Contact information can be found on
the official Beckhoff website at https://www.beckhoff.com/en-us/support/.

# Further Information

1. [TwinCAT 3 C++ documentation (Infosys)](https://infosys.beckhoff.com/content/1033/tc3_c/index.html)
2. [TwinCAT 3 C++ — Limitations of the real-time context](https://infosys.beckhoff.com/content/1033/tc3_c/674760331.html)
3. [Versioned C++ projects and Online Change](https://infosys.beckhoff.com/content/1033/tc3_c/6777687691.html)

## Requirements

The following components must be installed to build and run the sample code:

- [TE1000 TwinCAT 3 Engineering](https://www.beckhoff.com/en-us/products/automation/twincat/te1xxx-twincat-3-engineering/te1000.html)
  version 3.1.4026 or higher
- Microsoft Visual Studio with the *Desktop development with C++* workload
  (the project targets the v145 toolset; retarget under project properties for
  older toolsets)
- TC1300 TwinCAT 3 C++ license on the runtime target
- A TwinCAT user certificate for signing the versioned module
  ([Infosys: Module signing](https://infosys.beckhoff.com/content/1033/tc3_c/110691083.html))
- Optional, for the PC simulation of the original firmware: any MSVC
  toolchain — run `LegacyFirmware_Original\build_sim.ps1 -Run`
