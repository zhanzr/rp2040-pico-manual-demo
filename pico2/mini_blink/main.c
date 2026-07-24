/**
 * \file main.c
 * \brief Minimal RP2350 (Raspberry Pi Pico 2) board verification template
 *
 * Board:  Raspberry Pi Pico 2  (set PICO_BOARD=pico2, PICO_PLATFORM=rp2350)
 *
 * Purpose:
 *   - Confirms the build toolchain (cmake + arm-none-eabi-gcc) is functional.
 *   - Verifies the Pico SDK is correctly integrated.
 *   - Produces a .uf2 file ready to flash onto a Pico 2 board.
 *
 * Clock:
 *   No explicit clock setting — runs at the Pico SDK default (150 MHz).
 *
 * Expected behaviour:
 *   The onboard LED blinks slowly (1 s on, 1 s off) to provide a clear
 *   visual confirmation that the device is running custom firmware.
 */

#include "pico/stdlib.h"

int main(void)
{
    // Onboard LED — the SDK maps this to the correct GPIO for the board.
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    // Configure the GPIO as an output.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Main loop: blink the LED at a slow, visible rate.
    while (true)
    {
        gpio_put(LED_PIN, 1);   // LED on
        sleep_ms(1000);
        gpio_put(LED_PIN, 0);   // LED off
        sleep_ms(1000);
    }
}