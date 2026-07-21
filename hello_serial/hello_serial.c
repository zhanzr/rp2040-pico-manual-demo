/**
 * \file hello_serial.c
 * \brief Prints system clock & tick counts via hardware UART0 (GP0/GP1)
 *        every 5 seconds, while blinking the onboard LED at 1 Hz.
 *
 * stdio_init_all() already initialises UART0 on the default pins (GP0 TX,
 * GP1 RX) at 115200 baud, so plain printf() goes directly to the UART.
 * Baudrate is configurable via UART_BAUDRATE below.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"

// -------------------------------------------------------------------------
// Configurable baudrate for UART0
// -------------------------------------------------------------------------
#define UART_BAUDRATE 115200

int main(void)
{
    // Initialises both UART stdio (GP0/GP1) and USB stdio.
    stdio_init_all();

    // Override the baudrate if the default doesn't match UART_BAUDRATE.
    uart_set_baudrate(uart0, UART_BAUDRATE);

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    printf("RP2040 - Pico board\n");
    printf("Hardware UART0 on GP0/GP1 at %u baud\n", UART_BAUDRATE);
    printf("System clock: %u Hz\n", sys_clk_hz);

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
            printf("%u Hz, %llu ms\n", sys_clk_hz, tick_ms);
        }
        loop_count++;

        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}
