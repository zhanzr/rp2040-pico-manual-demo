# hello_serial — Lessons Learned

## What was wrong?

The original code used `stdio_uart_init_full()` to redirect `printf()` to hardware UART0:

```c
// BAD — caused the CPU to hang on Pico (RP2040)
stdio_uart_init_full(uart0, 0, 1, UART_BAUDRATE);
```

**Root cause:** On RP2040, `stdio_uart_init_full()` reconfigures the entire stdio
subsystem to use the UART backend. When called *without* first initialising the
default stdio via `stdio_init_all()`, the stdio layer's internal locking and
buffering can end up in an inconsistent state, causing a hard fault on the first
`printf()` call — no LED, no serial output.

On top of that, the `printf()` arguments were swapped relative to the format
string (see below), which itself causes a hard fault on Cortex-M0+ due to
variadic stack misalignment. Together these two issues made the board appear
completely dead.

## How it was fixed

Two changes were needed:

### 1. Initialise stdio before redirecting the UART backend

```c
// Must come first — sets up stdio locking & buffering
stdio_init_all();
```

This initialises the stdio layer properly so that subsequent I/O functions
work reliably.

### 2. Drive UART0 directly with the low-level hardware API (instead of stdio)

```c
uart_init(uart0, UART_BAUDRATE);
gpio_set_function(0, GPIO_FUNC_UART);
gpio_set_function(1, GPIO_FUNC_UART);

// ... later, inside the loop:
uart_puts(uart0, buf);
```

By using `uart_init()` / `gpio_set_function()` we configure the UART peripheral
registers directly. `uart_puts()` writes to the UART transmit FIFO without
going through the stdio abstraction layer at all, avoiding any potential
interaction issues with the stdio backend.

### 3. Fixed the format-string argument order

```c
// WRONG — causes hard fault on Cortex-M0+
printf("%u Hz, %llu ms\n", tick_ms, sys_clk_hz);
//      ^uint32           ^uint64

// RIGHT
printf("%u Hz, %llu ms\n", sys_clk_hz, tick_ms);
//      ^uint32           ^uint64
```

On Cortex-M0+, variadic arguments are passed on the stack with natural
alignment. Reading an 8-byte value where a 4-byte value is expected (or vice
versa) misaligns the stack frame, triggering a hard fault that silently resets
the CPU.

## Summary

| Symptom | Cause | Fix |
|---|---|---|
| No LED, no serial | `stdio_uart_init_full()` without prior `stdio_init_all()` caused stdio layer hang | Call `stdio_init_all()` first; use `uart_init()` + `uart_puts()` instead of stdio for UART output |
| LED blinked but no serial output | Format-string type mismatch caused hard fault | `printf("%u Hz, %llu ms\n", sys_clk_hz, tick_ms)` |

## Hardware

- **UART:** UART0 on **GPIO0 (TX)**, GPIO1 (RX)
- **Baudrate:** Configurable via `UART_BAUDRATE` (default 115200)
- **LED:** Onboard LED (PICO_DEFAULT_LED_PIN), blinks at 1 Hz
