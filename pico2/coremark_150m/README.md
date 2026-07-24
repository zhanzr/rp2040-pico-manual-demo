# Pico 2 board

Cortex M33 150 MHz
## coremark

### Flash cached
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

### Flash uncached
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

## Build & Flash

```bash
# Configure (one-time)
cd pico2/coremark_150m
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..

# Build
ninja

# Flash via probe-rs (default)
ninja flash

# Flash via OpenOCD (fallback)
ninja flash-ocd

# Deep clean (removes .o, .elf, .uf2, SDK copies; keeps CMakeCache.txt)
ninja clean-all

# After clean-all, rebuild without re-supplying -D flags
ninja
```
