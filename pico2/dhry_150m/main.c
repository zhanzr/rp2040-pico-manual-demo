#define PICO_CLOCK_AJDUST_PERI_CLOCK_WITH_SYS_CLOCK 1

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

#include "custom_def.h"
#include "dhry.h"

volatile uint32_t g_ticks;
extern uint32_t __INITIAL_SP;
extern void _stage2_boot(void);

extern void Proc_5 (void);

// Define a function pointer prototype matching your dhry_main signature
typedef void (*dhrystone_func_t)(uint32_t);


// -------------------------------------------------------------------------
// Configurable baudrate for UART0
// -------------------------------------------------------------------------
int main(void) {
    const uint32_t LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t loop_count = 0;

    set_sys_clock_khz(150000, false);

    // Initialise UART1 as the debug output port (GP4 TX, GP5 RX)
    uart1_init();

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    PRINTF("RP2350 - Pico 2 board (150 MHz)\n");
    PRINTF("CPU: %s @ %u MHz\n", CPU_ARCH, sys_clk_hz / 1000000);
    PRINTF("UART1 output on GP4/GP5 at 115200 baud\n");
    PRINTF("System clock: %u Hz (%u MHz)\n", sys_clk_hz,
           sys_clk_hz / 1000000);

// Flash version
	
	PRINTF("RP2350 XIP Cache Benchmarker\n");
	PRINTF("Standard flash function pointer address: 0x%08X\n", (uint32_t)&dhry_main);

	// Get the standard (cached) address of the benchmark function
	uint32_t cached_address = (uint32_t)&dhry_main;

	// Convert to the uncached XIP alias:
	//   Cached:  0x10000000 + offset  (XIP_BASE)
	//   Uncached:0x14000000 + offset  (XIP_NOCACHE_NOALLOC_BASE)
	// Mask out the XIP_BASE top bits and apply the uncached base.
	uint32_t uncached_address = (cached_address & 0x03FFFFFF) | 0x14000000;

	// Cast them back to executable function pointers
	dhrystone_func_t run_cached_dhry   = (dhrystone_func_t)cached_address;
	dhrystone_func_t run_uncached_dhry = (dhrystone_func_t)uncached_address;

	while (true) {
			// --- TEST 1: CACHED FLASH PERFORMANCE ---
			PRINTF("\n[1/2] Starting CACHED flash run (Target: %08X)...\n", cached_address);
			
			gpio_put(PICO_DEFAULT_LED_PIN, 1); // LED Solid during cached test
            run_cached_dhry(sys_clk_hz);
			PRINTF("CACHED run complete. %s\n", COMPILER_NAME);
			sleep_ms(3000);

			// --- TEST 2: UNCACHED FLASH PERFORMANCE ---
			PRINTF("\n[2/2] Starting UNCACHED flash run (Target: 0x%08X)...\n", uncached_address);
			PRINTF("Expect this to run significantly slower!\n");
			
			// Blink fast right before it enters slow mode so you know it's switching
			gpio_put(LED_PIN, 0); 
			sleep_ms(200);
			gpio_put(LED_PIN, 1);
			sleep_ms(200);
			gpio_put(LED_PIN, 0); 

			// This executes the exact code out of flash but forces a raw 
			// serial QSPI pin lookup for every loop fetch cycle.
            run_uncached_dhry(sys_clk_hz);
			
			PRINTF("UNCACHED run complete. %s\n", COMPILER_NAME);
			sleep_ms(3000);
	}
}
