/**
 * \file hello_serial.c
 * \brief Dual-UART demo: initialises both UART0 (GP0/GP1) and UART1
 *        (GP4/GP5), then blinks the LED and prints diagnostics.
 *
 * Usage:
 *   - UART0 (GP0 TX, GP1 RX) — intended for talking to external
 *     devices (e.g. ESP8285 WiFi module).
 *   - UART1 (GP4 TX, GP5 RX) — used as the STDOUT-style debug
 *     output port.  The PRINTF macro aliases to u1_printf().
 *
 * Clock runs at 150 MHz (max spec).
 */

#include <stdio.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/vreg.h"

// -------------------------------------------------------------------------
// Configurable parameters
// -------------------------------------------------------------------------
#define UART0_BAUDRATE  115200
#define UART1_BAUDRATE  115200

/** Print buffer size (256 covers most AT command responses).
 *  Messages longer than this will be silently truncated. */
#define BUF_SIZE 256

// -------------------------------------------------------------------------
// UART output helpers
//
//   u0_printf() — send formatted string via UART0 (GP0/GP1)
//   u1_printf() — send formatted string via UART1 (GP4/GP5)
//   PRINTF(...)  — alias for u1_printf (STDOUT-style output)
// -------------------------------------------------------------------------
static void u0_printf(const char *fmt, ...)
{
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for (const char *p = buf; *p; p++) {
        if (*p == '\n')
            uart_putc_raw(uart0, '\r');
        uart_putc_raw(uart0, *p);
    }
}

static void u1_printf(const char *fmt, ...)
{
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for (const char *p = buf; *p; p++) {
        if (*p == '\n')
            uart_putc_raw(uart1, '\r');
        uart_putc_raw(uart1, *p);
    }
}

/** Default output goes to UART1 (the debug/STDOUT port). */
#define PRINTF(...)  u1_printf(__VA_ARGS__)

int main(void)
{
    set_sys_clock_khz(150000, false);

    // ------------------------------------------------------------------
    // Initialise both UARTs — raw hardware, no stdio
    // ------------------------------------------------------------------
    uart_init(uart0, UART0_BAUDRATE);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    uart_init(uart1, UART1_BAUDRATE);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    const uint32_t sys_clk_mhz = sys_clk_hz / 1000000;

    PRINTF("RP2350 - Pico 2 board (%u MHz)\n", sys_clk_mhz);
    PRINTF("UART0 on GP0/GP1 at %u baud  (device comms)\n", UART0_BAUDRATE);
    PRINTF("UART1 on GP4/GP5 at %u baud  (debug output)\n", UART1_BAUDRATE);
    PRINTF("System clock: %u Hz (%u MHz)\n", sys_clk_hz, sys_clk_mhz);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t loop_count = 0;

    while (true)
    {
        // Print system tick count every 5 seconds.
        if (loop_count % 5 == 0)
        {
            const uint64_t tick_us = time_us_64();
            const uint64_t tick_ms = tick_us / 1000;
            PRINTF("%u Hz, %llu ms\r\n", sys_clk_hz, tick_ms);
        }
        loop_count++;

        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}
