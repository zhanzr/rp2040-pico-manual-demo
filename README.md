# RP2040 manual building Demo

A collection of RP2040 (Raspberry Pi Pico) firmware projects with a shared
build toolchain, plus a template script for bootstrapping new projects.

---

## Prerequisites

| Tool | Path |
|---|---|
| **Pico SDK** | `D:/pico-sdk` |
| **ARM GCC toolchain** | `D:/arm-gnu-toolchain/bin` (arm-none-eabi-gcc 14.x) |
| **CMake** | ≥ 3.13 (available on `PATH`) |
| **Ninja** | `C:/msys64/mingw64/bin/ninja.exe` |
| **Python 3** | `C:/msys64/mingw64/bin/python3.exe` |
| **picotool** | Pre-built during `mini_blink` first build |

---

## Projects

| Project | Clock | Description |
|---|---|---|
| `mini_blink/` | 125 MHz | Minimal LED blinker (reference / sanity check) |
| `hello_serial/` | 125 MHz | Prints system clock & tick count via UART0 (GP0/GP1) every 5 s, LED blinks at 1 Hz |
| `hello_serial_200mhz/` | **200 MHz** | Same as above but overclocked (VREG 1.15 V + 200 MHz PLL) |

All UART projects use hardware UART0 on **GPIO0 (TX)** and **GPIO1 (RX)** at a
configurable baudrate (default 115 200).

---

## Quick-start — build & flash

```bash
# Build a project
cd hello_serial
mkdir build && cd build
cmake -G Ninja -DPICO_BOARD=pico -DPICO_SDK_PATH="D:/pico-sdk" \
      -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" \
      -DPICO_TOOLCHAIN_PATH="D:/arm-gnu-toolchain/bin" \
      -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" \
      -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" ..
ninja
```
## use an existing picotool
```
PICOTOOL_PATH="D:/picotool/picotool.exe" cmake -G Ninja \
  -DPICO_BOARD=pico \
  -DPICO_SDK_PATH="D:/pico-sdk" \
  -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" \
  -DPICO_TOOLCHAIN_PATH="D:/arm-gnu-toolchain/bin" \
  -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" \
  -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" \
  ..
```
## use arm-none-eabi-gcc 15
```
PICOTOOL_PATH="D:/picotool/picotool.exe" cmake -G Ninja \
  -DPICO_BOARD=pico \
  -DPICO_SDK_PATH="D:/pico-sdk" \
  -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" \
  -DPICO_TOOLCHAIN_PATH="D:\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin" \
  -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" \
  -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" \
  ..
```

The build produces a `.uf2` file. Hold the **BOOTSEL** button on the Pico,
connect USB, then copy the `.uf2` to the `RPI-RP2` drive.

---

## Template script

Use `create_pico_template.sh` to bootstrap a new project from an existing
template:

```bash
# Normal (125 MHz) — copies from hello_serial/
./create_pico_template.sh          # → creates template_1/

# Overclocked (200 MHz) — copies from hello_serial_200mhz/
./create_pico_template.sh 200mhz   # → creates template_200mhz_1/
```

The script automatically:
- Picks the next unused number (`template_1`, `template_2`, …)
- Renames the `.c` file to match the project name
- Updates `CMakeLists.txt` (project name, executable name, target names)

---

## Pinout

| GPIO | Function | Signal |
|---|---|---|
| 0 | UART0 TX | Output (connect to external UART RX) |
| 1 | UART0 RX | Input (connect to external UART TX) |
| 25 (LED) | GPIO out | Onboard LED, blinks at 1 Hz |

---

## Notes

- The SDK default for `stdio_init_all()` already routes `printf()` to UART0 on
  GP0/GP1 at 115 200 baud — no extra UART initialisation needed.
- Baudrate is configurable via the `UART_BAUDRATE` macro in each source file.
- For 200 MHz operation the core voltage is raised to 1.15 V via
  `vreg_set_voltage()`. This is safe for the RP2040 but exceeds the 133 MHz
  rated maximum — use at your own discretion.
