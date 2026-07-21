/**
 * \file hello_serial_200mhz.c
 * \brief Same as hello_serial, but RP2040 overclocked to 200 MHz.
 *
 * The SDK confirms the RP2040 can run at up to 200 MHz (the official
 * limit is 133 MHz, but raising the core voltage to 1.15 V enables
 * stable 200 MHz operation). This project sets VCO to 200 MHz via
 * set_sys_clock_khz().
 *
 * stdio_init_all() already initialises UART0 on the default pins (GP0 TX,
 * GP1 RX) at 115200 baud, so plain printf() goes directly to the UART.
 * Baudrate is configurable via UART_BAUDRATE below.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/vreg.h"

// -------------------------------------------------------------------------
// Configurable baudrate for UART0
// -------------------------------------------------------------------------
#define UART_BAUDRATE 115200

int main(void)
{
    // ------------------------------------------------------------------
    // Overclock: set the system clock to 200 MHz.
    //
    // Per the RP2040 datasheet, clk_sys has a maximum of 133 MHz across
    // all process / voltage / temperature variations.  To reach 200 MHz
    // we raise DVDD (the core supply) from the default 1.10 V to
    // 1.15 V by setting VREG VSEL accordingly.
    //
    // set_sys_clock_khz(200000, false) reconfigures the PLL to produce
    // 200 MHz.  The second argument 'false' requests "best effort" — the
    // SDK picks the nearest achievable frequency if 200 MHz is not
    // exactly reachable with the 12 MHz crystal.
    // ------------------------------------------------------------------
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(10);
    set_sys_clock_khz(200000, false);

    // Initialises both UART stdio (GP0/GP1) and USB stdio.
    stdio_init_all();

    // Override the baudrate if the default doesn't match UART_BAUDRATE.
    uart_set_baudrate(uart0, UART_BAUDRATE);

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    printf("RP2040 - Pico board (200 MHz)\n");
    printf("Hardware UART0 on GP0/GP1 at %u baud\n", UART_BAUDRATE);
    printf("System clock: %u Hz (%u MHz)\n", sys_clk_hz,
           sys_clk_hz / 1000000);

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
