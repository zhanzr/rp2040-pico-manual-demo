# hello_serial — Dual-UART Demo (200 MHz / 125 MHz)

Initialises both hardware UARTs simultaneously, blinks the onboard LED,
and prints diagnostics at 5-second intervals.

## Clock speed

**Default: 200 MHz** (overclocked — VREG 1.15 V + 200 MHz PLL).

To switch to the standard **125 MHz**, edit `hello_serial.c`:

1. Comment out the `OVERCLOCK` block (lines with `vreg_set_voltage`,
   `set_sys_clock_khz(200000, ...)`).
2. Uncomment the `NORMAL` line: `set_sys_clock_khz(125000, true);`

## UARTs (both always active)

| Port   | TX | RX | Baud   | Purpose               |
|--------|----|----|--------|-----------------------|
| UART0  | GP0| GP1| 115200 | Device comms (ESP8285)|
| UART1  | GP4| GP5| 115200 | Debug / STDOUT output |

Use `u0_printf(...)` to send data via UART0, and `u1_printf(...)` or
`PRINTF(...)` (alias) via UART1.

## Building

```bash
cd pico/hello_serial
mkdir build && cd build
PICO_SDK_PATH=$HOME/pico-sdk cmake -G Ninja ..
ninja                    # build .elf
ninja uf2               # generate .uf2 for BOOTSEL-mode flash
ninja flash             # flash via OpenOCD + SWD
```

## Connecting

| Pico GPIO | Function | Connect to                    |
|-----------|----------|-------------------------------|
| GP0       | UART0 TX | ESP8285 / external UART RX    |
| GP1       | UART0 RX | ESP8285 / external UART TX    |
| GP4       | UART1 TX | Debug serial adapter RX       |
| GP5       | UART1 RX | Debug serial adapter TX       |
| GP25 (LED)| GPIO out | Onboard LED (blinks 1 Hz)     |

For UART1 mode: use GP4 (TX) and GP5 (RX) instead.
