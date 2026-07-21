#define PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK 1
#define PICO_DEFAULT_UART_BAUD_RATE 115200U

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/platform.h"
#include "hardware/structs/sio.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/sync.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/uart.h"
#include "hardware/vreg.h"

#include "utils.h"
#include "custom_def.h"
#include "core_portme.h"

volatile uint32_t g_ticks;
extern uint32_t __INITIAL_SP;
extern void _stage2_boot(void);

// coremark_main is defined in coremark_1_0_1/core_main.c
int coremark_main(void);
typedef int (*cm_main_func_t)(void);

int main(void)
{
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(10);
    set_sys_clock_khz(200000, false);

    // Initialises both UART stdio (GP0/GP1) and USB stdio.
    stdio_init_all();

    // Override the baudrate if the default doesn't match PICO_DEFAULT_UART_BAUD_RATE.
    uart_set_baudrate(uart0, PICO_DEFAULT_UART_BAUD_RATE);

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    printf("RP2040 - Pico board (200 MHz)\n");
    printf("Hardware UART0 on GP0/GP1 at %u baud\n", PICO_DEFAULT_UART_BAUD_RATE);
    printf("System clock: %u Hz (%u MHz)\n", sys_clk_hz,
           sys_clk_hz / 1000000);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t loop_count = 0;

    printf("Standard flash function pointer address: %p\n", coremark_main);

// Get the standard address of the benchmark function (typically 0x10xxxxxx)
	uint32_t cached_address = (uint32_t)&coremark_main;

	// Force the base pointer window from 0x10000000 to 0x13000000
	// This retains the original lower offset bits and the essential Thumb bit (bit 0)
	uint32_t uncached_address = (cached_address & 0x0FFFFFFF) | 0x13000000;

	// Cast them back to executable function pointers
	cm_main_func_t run_cached_cm_main   = (cm_main_func_t)cached_address;
	cm_main_func_t run_uncached_cm_main = (cm_main_func_t)uncached_address;

	while(1) {
			// --- TEST 1: CACHED FLASH PERFORMANCE ---
			printf("\n[1/2] Starting CACHED flash run (Target: %08X)...\n", cached_address);
			
			gpio_put(LED_PIN, 1); // LED Solid during cached test
			run_cached_cm_main(); 
			
			printf("CACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
			sleep_ms(3000);

			// --- TEST 2: UNCACHED FLASH PERFORMANCE ---
			printf("\n[2/2] Starting UNCACHED flash run (Target: %08X)...\n", uncached_address);
			printf("Expect this to run significantly slower!\n");
			
			// Blink fast right before it enters slow mode so you know it's switching
			gpio_put(LED_PIN, 0); 
			sleep_ms(200);
			gpio_put(LED_PIN, 1);
			sleep_ms(200);
			gpio_put(LED_PIN, 0); 

			// This executes the exact code out of flash but forces a raw 
			// serial QSPI pin lookup for every loop fetch cycle.
			run_uncached_cm_main(); 
			
			printf("UNCACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
			sleep_ms(3000);
	}
}
