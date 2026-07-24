/**
 * \file main.c
 * \brief Minimal RP2350 (Raspberry Pi Pico 2 W) board verification template
 *
 * Board:  Raspberry Pi Pico 2 W  (PICO_BOARD=pico2_w, PICO_PLATFORM=rp2350)
 *
 * Purpose:
 *   - Confirms the build toolchain (cmake + arm-none-eabi-gcc) is functional.
 *   - Verifies the Pico SDK is correctly integrated.
 *   - Produces a .uf2 file ready to flash onto a Pico 2 W board.
 *
 * Clock:
 *   No explicit clock setting — runs at the Pico SDK default (150 MHz).
 *
 * LED:
 *   The Pico 2 W has no PICO_DEFAULT_LED_PIN.  The onboard LED is
 *   controlled via the CYW43 wireless chip (GPIO 0 on the WL chip).
 *   The cyw43 driver must be initialised before toggling the LED.
 *
 * Expected behaviour:
 *   The onboard LED blinks slowly (1 s on, 1 s off) to provide a clear
 *   visual confirmation that the device is running custom firmware.
 */

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    // Initialise the cyw43 driver (required for wireless-chip LED).
    cyw43_arch_init();

    // Main loop: blink the LED at a slow, visible rate.
    while (true)
    {
        cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 1);   // LED on
        sleep_ms(1000);
        cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 0);   // LED off
        sleep_ms(1000);
    }
}
