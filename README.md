# RP2040 manual building Demo

A collection of RP2040 (Raspberry Pi Pico) firmware projects with a shared
build toolchain, plus a template script for bootstrapping new projects.

---

## Prerequisites

| Tool | macOS / Linux | Windows |
|---|---|---|
| **probe-rs** | `brew install probe-rs-tools` | [Download from GitHub](https://github.com/probe-rs/probe-rs/releases) or `cargo install probe-rs-tools` |
| **Pico SDK** | `$HOME/pico-sdk` (set `PICO_SDK_PATH`) | `D:/pico-sdk` (set `PICO_SDK_PATH`) |
| **ARM GCC toolchain** | `arm-none-eabi-gcc` on PATH, or via e.g. `/Applications/ArmGNUToolchain/.../bin` | e.g. `D:/arm-gnu-toolchain/bin` |
| **CMake** | ≥ 3.13 — `brew install cmake` / `apt install cmake` | Download from cmake.org |
| **Ninja** | `brew install ninja` / `apt install ninja-build` | `C:/msys64/mingw64/bin/ninja.exe` |
| **Python 3** | `python3` on PATH | `C:/msys64/mingw64/bin/python3.exe` |
| **picotool** (optional) | Install once via `cmake --install` (see below) — then set `picotool_DIR` | Same |

---

## Projects

All project sources live under `pico/`:

| Project | Clock | Description |
|---|---|---|
| `pico/mini_blink/` | 125 MHz | Minimal LED blinker (reference / sanity check) |
| `pico/hello_serial/` | **200 MHz** (default) / 125 MHz | Dual-UART demo — UART0 (GP0/GP1) for device comms, UART1 (GP4/GP5) for debug via `PRINTF()`. LED blinks at 1 Hz. |
| `pico/coremark_200m/` | **200 MHz** | CoreMark benchmark (cached vs uncached XIP flash comparison) |
| `pico/dhry_200m/` | **200 MHz** | Dhrystone 2.1 benchmark (cached vs uncached XIP flash comparison) |

The `hello_serial` project initialises **both UARTs** simultaneously:
- **UART0** (GP0 TX, GP1 RX) — intended for talking to external devices
  (e.g. ESP8285 WiFi module) via `u0_printf()`.
- **UART1** (GP4 TX, GP5 RX) — debug/STDOUT output via `PRINTF()` (alias
  for `u1_printf()`).

Similarly, `coremark_200m` and `dhry_200m` now output to **UART1 on
GP4/GP5** using the `PRINTF()` macro defined in their `utils.h`.

---

## Quick-start — build & flash

> **Tip:** On macOS / Linux, you only need to set `PICO_SDK_PATH` — the SDK
> auto-detects the ARM GCC toolchain if it is on your `PATH`.  
> On Windows (MSYS2 MinGW64), you may need to supply all paths explicitly.
>
> Flashing requires **probe-rs** (`brew install probe-rs-tools` on macOS).
> For unsupported probes, use `ninja flash-ocd` instead (requires OpenOCD).

### macOS / Linux

```bash
# Set once — add to your ~/.bashrc or ~/.zshrc
export PICO_SDK_PATH="$HOME/pico-sdk"

# Optional: point CMake to a pre-installed picotool (avoids source build)
export picotool_DIR="$HOME/tool/picotool"

# Configure & build (produces .elf only)
cd pico/mini_blink
mkdir build && cd build
cmake -G Ninja ..
ninja

# Flash via probe-rs (default, recommended)
ninja flash

# Or flash via OpenOCD (fallback)
ninja flash-ocd

# Or generate .uf2 for BOOTSEL-mode flashing
ninja uf2
```

### Windows (MSYS2 MinGW64)

```bash
cd pico/mini_blink
mkdir build && cd build
cmake -G Ninja -DPICO_BOARD=pico -DPICO_SDK_PATH="D:/pico-sdk" ^
      -Dpicotool_DIR="D:/picotool" ^
      -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" ^
      -DPICO_TOOLCHAIN_PATH="D:/arm-gnu-toolchain/bin" ^
      -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" ^
      -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" ..
ninja                    # build .elf only

ninja flash              # flash via probe-rs (default)
ninja flash-ocd          # flash via OpenOCD (fallback)
ninja uf2                # generate .uf2 for BOOTSEL mode
```

### Manual UF2 flash (any OS)

If OpenOCD is not available, hold the **BOOTSEL** button on the Pico, connect
USB, then copy the `.uf2` file from the `build/` directory to the `RPI-RP2`
drive.

---

## probe-rs setup (one-time)

[probe-rs](https://github.com/probe-rs/probe-rs) is the default flashing tool.
It supports Picoprobe, Raspberry Pi Debug Probe, J-Link, ST-Link, and many more.

### macOS

```bash
brew install probe-rs-tools
```

### Linux

```bash
# From source (requires Rust):
cargo install probe-rs-tools

# Or download a pre-built binary from:
# https://github.com/probe-rs/probe-rs/releases
```

### Windows

```bash
# From source (requires Rust):
cargo install probe-rs-tools

# Or download the installer from:
# https://github.com/probe-rs/probe-rs/releases
```

After installation, verify with:
```bash
probe-rs list    # should show your debug probe
```

> **Note:** probe-rs auto-detects your probe — no config files needed.
> For unsupported probes, fall back to OpenOCD via `ninja flash-ocd`.

---

## Picotool setup (optional — for UF2 generation)

The SDK builds picotool from source on every fresh `cmake` — this takes ~5 min.
Avoid this by installing picotool once, then pointing new projects to it.

### macOS / Linux

```bash
# 1. Build any project once (this compiles picotool from source)
cd pico/mini_blink
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..
ninja mini_blink                     # this builds picotool from source

# 2. Install the freshly-built picotool to a permanent location
cmake --install _deps/picotool-build --prefix $HOME/tool 2>/dev/null

# 3. Add to your shell config (~/.bashrc or ~/.zshrc)
echo 'export picotool_DIR="$HOME/tool/picotool"' >> ~/.bashrc
```

After that, all future `cmake` runs will find the installed picotool instantly:

```bash
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..   # 1 s, no download
ninja                                              # build .elf only (fast)
ninja uf2                                          # generate .uf2 when needed
ninja flash                                        # flash via SWD
```

### Windows (MSYS2 MinGW64)

```bash
# After building, install to a permanent location
cmake --install build/_deps/picotool-build --prefix "D:/picotool"

# Then on future cmake invocations:
cmake -G Ninja -Dpicotool_DIR="D:/picotool" ..
```

### Available ninja targets

| Command | Description |
|---|---|
| `ninja` | Build the project (default target — produces `.elf` only) |
| `ninja uf2` | Explicitly generate `.uf2` for BOOTSEL-mode flashing |
| `ninja flash` | Flash `.elf` to Pico via **probe-rs** (default) |
| `ninja flash-ocd` | Flash `.elf` to Pico via **OpenOCD** (fallback) |
| `ninja clean-all` | Deep clean — removes objects and SDK copies, keeps CMake cache |

---

## Flash configuration

### probe-rs (default)

[probe-rs](https://github.com/probe-rs/probe-rs) supports a wide range of
debug probes out of the box — Picoprobe, Raspberry Pi Debug Probe,
J-Link, ST-Link, etc.  No configuration needed:

```bash
ninja flash
```

### OpenOCD (fallback)

For probes that probe-rs does not support, use `ninja flash-ocd`.  The
OpenOCD interface is configurable via CMake cache variables:

```bash
# CMSIS-DAP / Picoprobe (default) — works out of the box
ninja flash-ocd

# J-Link
cmake -G Ninja -DOPENOCD_INTERFACE=jlink.cfg -DOPENOCD_ADAPTER_ARGS= ..
ninja flash-ocd

# ST-Link
cmake -G Ninja -DOPENOCD_INTERFACE=stlink.cfg -DOPENOCD_ADAPTER_ARGS= ..
ninja flash-ocd
```

| Cache variable | Default | Description |
|---|---|---|
| `OPENOCD_INTERFACE` | `interface/cmsis-dap.cfg` | OpenOCD interface config file |
| `OPENOCD_ADAPTER_SPEED` | `5000` | SWD clock speed in kHz |
| `OPENOCD_ADAPTER_ARGS` | `-c;cmsis_dap_backend usb_bulk` | Extra `-c` args (empty for J-Link/ST-Link) |

> **Tip:** Once set via `cmake -D`, these values are cached in
> `CMakeCache.txt`.  To change later, edit `CMakeCache.txt` or re-run with
> new `-D` flags.

## Template script

Use `pico/create_template.sh` to bootstrap a new project from the
`pico/hello_serial/` dual-UART template:

```bash
cd pico && bash create_template.sh   # → creates template_1/ in pico/
```

The script automatically:
- Picks the next unused number (`template_1`, `template_2`, …)
- Names the source file `main.c` — generic so the folder can be renamed
- Sets the project name, executable name, and all CMake targets to match

**Renaming a project folder after creation:**

If you rename the folder (e.g. `template_1` → `esp8285_test`), you must
also update every occurrence of the old project name in `CMakeLists.txt`
(project, add_executable, target_link_libraries, pico_enable_stdio_*,
DEPENDS, custom targets for flash, uf2, clean-all, elf, uf2, and the
CMakeFiles/...dir path).  A quick way on macOS:

```bash
cd pico/esp8285_test
sed -i '' 's/template_1/esp8285_test/g' CMakeLists.txt
```
- Updates `CMakeLists.txt` (project name, executable name, target names)

The created project always defaults to **200 MHz** and always initialises
both UART0 and UART1.  Edit the `.c` file to switch to 125 MHz.

---

## Pinout

### `pico/hello_serial` (dual UART)

| GPIO | Function | Signal |
|---|---|---|
| 0 | UART0 TX | Device comms (e.g. ESP8285) |
| 1 | UART0 RX | Device comms |
| 4 | UART1 TX | Debug output (`PRINTF`) |
| 5 | UART1 RX | Debug input |
| 25 (LED) | GPIO out | Onboard LED, blinks at 1 Hz |

### Other projects

| Project | Output port |
|---|---|
| `pico/mini_blink/` | (none — LED only) |
| `pico/coremark_200m/` | UART1 (GP4 TX) via `PRINTF()` |
| `pico/dhry_200m/` | UART1 (GP4 TX) via `PRINTF()` |


## MacOS serial port terminal
The target board's TX(GP0) connects to the probe board's RX(GP5)
### using screen
```bash
ls /dev/tty.usbmodem*

screen /dev/tty.usbmodemXXXXX 115200
```
Exit: Ctrl-A then k, y
Detach & reattach: Ctrl-A then D, screen -r

## Linux serial port terminal
### using tio
```bash
ls /dev/ttyUSB* /dev/ttyACM* /dev/ttyS*
ls: cannot access '/dev/ttyUSB*': No such file or directory
 /dev/ttyACM0   /dev/ttyS0   /dev/ttyS1   /dev/ttyS2   /dev/ttyS3

tio -b 115200 /dev/ttyACM0 
```
Exit: Ctrl-T then Q
---

## Notes

- The merged `pico/hello_serial/` project initialises **both** UART0 (GP0/GP1)
  and UART1 (GP4/GP5) using the raw hardware API (`uart_init` / `uart_puts`).
  Use `u0_printf()` for UART0 (device comms) and `PRINTF()` / `u1_printf()`
  for UART1 (debug output).
- Baudrate is configurable via the `UART0_BAUDRATE` / `UART1_BAUDRATE` macros.
- For 200 MHz operation the core voltage is raised to 1.15 V via
  `vreg_set_voltage()`. This is safe for the RP2040 but exceeds the 133 MHz
  rated maximum — use at your own discretion.
- No USB CDC or stdio is used — raw UART hardware only.
