# Pico 2 board

Cortex M33 150 MHz
## dhrystone

### Flash cached
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[21-4661]:      2.320 
Dhrystones per Second:  431034.469 
DMIPS/MHz:      1.635
```

### Flash uncached
```
-Ofast -funroll-loops

MicroSecond for one run through Dhrystone[8225-142079]:  66.927
Dhrystones per Second:  14941.653
DMIPS/MHz:      0.057
```

## Build & Flash

```bash
# Configure (one-time)
cd pico2/dhry_150m
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..

# Build
ninja

# Flash via probe-rs (default)
ninja flash

[# Flash via OpenOCD (fallback)
ninja flash-ocd

# Deep clean (removes .o, .elf, .uf2, SDK copies; keeps CMakeCache.txt)
ninja clean-all

# After clean-all, rebuild without re-supplying -D flags
ninja
```
