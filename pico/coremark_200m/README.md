# Pico board

Cortex M0+ 200 MHz
## coremark

### Flash cached
gcc version 15.3.1 20260627 (Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149))
```
-Ofast -funroll-loops

2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 10033
Total time (secs): 10.033000
Iterations/Sec   : 398.684342
Iterations       : 4000
Compiler version : gcc version 15.3.1 20260627
Compiler flags   : -Ofast -funroll-loops
Memory location  : Static
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0x65c5
Correct operation validated. See readme.txt for run and reporting rules.
CoreMark 1.0 : 398.684342 / gcc version 15.3.1 20260627 -Ofast -funroll-loops / Static
```

### Flash uncached
gcc version 15.3.1 20260627 (Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149))
```
-Ofast -funroll-loops

2K performance run parameters for coremark.
CoreMark Size    : 666
Total ticks      : 225386
Total time (secs): 225.386000
Iterations/Sec   : 17.747331
Iterations       : 4000
Compiler version : gcc version 15.3.1 20260627
Compiler flags   : -Ofast -funroll-loops
Memory location  : Static
seedcrc          : 0xe9f5
[0]crclist       : 0xe714
[0]crcmatrix     : 0x1fd7
[0]crcstate      : 0x8e3a
[0]crcfinal      : 0x65c5
Correct operation validated. See readme.txt for run and reporting rules.
CoreMark 1.0 : 17.747331 / gcc version 15.3.1 20260627 -Ofast -funroll-loops / Static
```

## Build & Flash

```bash
# Configure (one-time)
mkdir build && cd build
PICOTOOL_PATH="D:/picotool/picotool.exe" cmake -G Ninja \
  -DPICO_BOARD=pico \
  -DPICO_SDK_PATH="D:/pico-sdk" \
  -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" \
  -DPICO_TOOLCHAIN_PATH="D:\Arm\GNU Toolchain mingw-w64-x86_64-arm-none-eabi\bin" \
  -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" \
  -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" \
  ..

# Build
ninja

# Flash via OpenOCD + CMSIS-DAP
ninja flash

# Remove object files only
ninja clean

# Deep clean (removes .o, .elf, .uf2, SDK copies; keeps CMakeCache.txt)
ninja clean-all

# After clean-all, rebuild without re-supplying -D flags
ninja
```