#define PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK 1

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/platform.h"
#include "pico/cyw43_arch.h"
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
    // --- Pico 2 W LED: controlled via CYW43 wireless chip ---
    cyw43_arch_init();

    set_sys_clock_khz(150000, false);

    // Initialise UART1 as the debug output port (GP4 TX, GP5 RX)
    uart1_init();

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    PRINTF("Standard flash function pointer address: %p\n", coremark_main);

// Get the standard (cached) address of the benchmark function
	uint32_t cached_address = (uint32_t)&coremark_main;

	// Convert to the uncached XIP alias:
	//   Cached:  0x10000000 + offset  (XIP_BASE)
	//   Uncached:0x14000000 + offset  (XIP_NOCACHE_NOALLOC_BASE)
	// Mask out the XIP_BASE top bits and apply the uncached base.
	uint32_t uncached_address = (cached_address & 0x03FFFFFF) | 0x14000000;

	// Cast them back to executable function pointers
	cm_main_func_t run_cached_cm_main   = (cm_main_func_t)cached_address;
	cm_main_func_t run_uncached_cm_main = (cm_main_func_t)uncached_address;

	while(1) {
			// --- TEST 1: CACHED FLASH PERFORMANCE ---
			PRINTF("\n[1/2] Starting CACHED flash run (Target: %08X)...\n", cached_address);
			
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 1); // LED solid during cached test
			run_cached_cm_main(); 
			
			PRINTF("CACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
			sleep_ms(3000);

			// --- TEST 2: UNCACHED FLASH PERFORMANCE ---
			PRINTF("\n[2/2] Starting UNCACHED flash run (Target: %08X)...\n", uncached_address);
			PRINTF("Expect this to run significantly slower!\n");
			
			// Blink fast right before it enters slow mode so you know it's switching
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 0);
			sleep_ms(200);
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 1);
			sleep_ms(200);
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 0);

			// This executes the exact code out of flash but forces a raw 
			// serial QSPI pin lookup for every loop fetch cycle.
			run_uncached_cm_main(); 
			
			PRINTF("UNCACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
			sleep_ms(3000);
	}
}