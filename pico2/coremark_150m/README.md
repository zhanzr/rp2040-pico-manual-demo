# Pico 2 board

RP2350 dual-architecture CoreMark benchmark: runs on either **ARM Cortex-M33**
or **RISC-V Hazard3** at 150 MHz.

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
# ┌─ ARM Cortex-M33 ──────────────────────────┐
cd pico2/coremark_150m
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..

# ┌─ RISC-V Hazard3 ──────────────────────────┐
cd pico2/coremark_150m
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja .. \
    -DPICO_PLATFORM=rp2350-riscv \
    -DPICO_GCC_TRIPLE=riscv64-unknown-elf
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
> picotool uf2 convert coremark_150m.elf coremark_150m.uf2
> picotool load coremark_150m.uf2
> picotool reboot
> ```

---

## Results

### Cortex M33 150 MHz

**Flash cached**
```
-Ofast -funroll-loops

2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 10950
Total time (secs): 10.950000
Iterations/Sec   : 456.621005
Iterations       : 5000
Compiler version : GCC 15.3.1 20260627
Compiler flags   : -Ofast -funroll-loops
Memory location  : Static
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0xbd59
Correct operation validated. See readme.txt for run and reporting rules.
CoreMark 1.0 : 456.621005 / GCC 15.3.1 20260627 -Ofast -funroll-loops / Static
```

**Flash uncached**
```
-Ofast -funroll-loops

2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 287411
Total time (secs): 287.411000
Iterations/Sec   : 17.396690
Iterations       : 5000
Compiler version : GCC 15.3.1 20260627
Compiler flags   : -Ofast -funroll-loops
Memory location  : Static
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0xbd59
Correct operation validated. See readme.txt for run and reporting rules.
CoreMark 1.0 : 17.396690 / GCC 15.3.1 20260627 -Ofast -funroll-loops / Static
```
