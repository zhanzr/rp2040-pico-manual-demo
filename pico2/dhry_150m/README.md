# Pico 2 board

RP2350 dual-architecture benchmark: runs on either **ARM Cortex-M33** or **RISC-V Hazard3** at 150 MHz.

At startup the program prints which CPU mode it is executing on, so you can
compare identical code across the two ISAs on the same chip.

---

## Build & Flash

**Pick your CPU at cmake time** — everything else is the same.

### Prerequisites — RISC-V toolchain (only for RISC-V builds)

```bash
brew tap riscv-software-src/riscv
brew trust riscv-software-src/riscv
brew install riscv-gnu-toolchain
riscv64-unknown-elf-gcc -print-multi-lib   # must list rv32imac/ilp32
```

### Configure (one-time)

Choose ARM (default) or RISC-V:

```bash
# ┌─ ARM Cortex-M33 ─────────────────────────┐
# │ cd pico2/dhry_150m                       │
# │ mkdir build && cd build                  │
# │ PICO_SDK_PATH=$HOME/pico-sdk             │
# │     cmake -G Ninja ..                    │
# └──────────────────────────────────────────┘

# ┌─ RISC-V Hazard3 ─────────────────────────┐
# │ cd pico2/dhry_150m                       │
# │ mkdir build && cd build                  │
# │ PICO_SDK_PATH=$HOME/pico-sdk             │
# │     cmake -G Ninja ..                    │
# │     -DPICO_PLATFORM=rp2350-riscv         │
# │     -DPICO_GCC_TRIPLE=riscv64-unknown-elf│
# └──────────────────────────────────────────┘
```

### Build & Flash (same commands either way)

```bash
ninja          # build
ninja flash    # flash via probe-rs
ninja flash-ocd  # fallback: flash via OpenOCD
ninja clean-all  # deep clean
```

> **Note — RISC-V debug limitation:** probe-rs connects via the ARM Debug Port.
> Once RISC-V code is running, the ARM DP is locked and probe-rs cannot
> reconnect. To re-flash after RISC-V is already running, hold **BOOTSEL** +
> press **RESET**, then use UF2 mode:
> ```bash
> picotool uf2 convert dhry_150m.elf dhry_150m.uf2
> picotool load dhry_150m.uf2
> picotool reboot
> ```

---

## Results
