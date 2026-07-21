# Pico board

Cortex M0+ 200 MHz
## dhrystone

### Flash cached
gcc version 14.3.1 20250623 (GNU Tools for STM32 14.3.rel1.20251027-0700)
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[50-4970]:      2.460
Dhrystones per Second:  406504.062
DMIPS/MHz:      1.157
```

gcc version 15.3.1 20260627 (Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149))
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[50-5100]:      2.525
Dhrystones per Second:  396039.594
DMIPS/MHz:      1.127
```

### Flash uncached
gcc version 14.3.1 20250623 (GNU Tools for STM32 14.3.rel1.20251027-0700)
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[8540-104960]:  48.210
Dhrystones per Second:  20742.584
DMIPS/MHz:      0.059
```

gcc version 15.3.1 20260627 (Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149))
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[8672-106712]:  49.020
Dhrystones per Second:  20399.836
DMIPS/MHz:      0.058
```
### SRAM
gcc version 14.3.1 20250623 (GNU Tools for STM32 14.3.rel1.20251027-0700)
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[51-5151]:      2.550
Dhrystones per Second:  392156.875
DMIPS/MHz:      1.116
```

gcc version 15.3.1 20260627 (Arm GNU Toolchain 15.3.Rel1 (Build arm-15.149))
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[50-5280]:      2.615
Dhrystones per Second:  382409.188
DMIPS/MHz:      1.088
```

## Build & Flash

```bash
# Configure (one-time)
mkdir build && cd build
cmake -G Ninja -DPICO_BOARD=pico -DPICO_SDK_PATH="D:/pico-sdk" \
      -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" \
      -DPICO_TOOLCHAIN_PATH="D:/arm-gnu-toolchain/bin" \
      -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" \
      -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" ..

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