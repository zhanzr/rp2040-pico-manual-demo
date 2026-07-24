# cyw43_test — CYW43 Wireless Chip Test (Pico 2 W)

Tests the on-board CYW43 wireless chip on the Raspberry Pi Pico 2 W.

## What it does

1. Initialises the CYW43 driver (PIO SPI bus + firmware download)
2. Scans for nearby WiFi access points and prints the list
3. Connects to a configured AP (credentials in `wifi_config.h`)
4. Prints the assigned IP address
5. Blinks the onboard LED at 1 Hz
6. Reports WiFi link status every 30 s

## Prerequisites

The CYW43 driver and lwIP must be initialised in the SDK:

```bash
cd $PICO_SDK_PATH
git submodule update --init lib/cyw43-driver
```

## WiFi credentials

Copy `wifi_config.h.example` to `wifi_config.h` and fill in your AP name and
password:

```bash
cp wifi_config.h.example wifi_config.h
# then edit wifi_config.h
```

The file `wifi_config.h` is excluded via `.gitignore` — your credentials
remain private.

## Building

```bash
cd pico2-w/cyw43_test
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja .. -Dpioasm_DIR="$HOME/tool/pioasm/pioasm"
ninja
ninja flash          # flash via probe-rs (10 MHz SWD)
```

## Connecting

| Pico 2 W GPIO | Function | Connect to                    |
|---------------|----------|-------------------------------|
| GP4           | UART1 TX | Debug serial adapter RX       |
| GP5           | UART1 RX | Debug serial adapter TX       |

The CYW43 WiFi chip is connected internally via PIO SPI — no external
wiring is needed for wireless operation.
