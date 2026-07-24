// clk_peri tracks clk_sys automatically

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/platform.h"
#if !defined(__riscv)
#include "pico/cyw43_arch.h"
#endif
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
    // RP2350 boots at 150 MHz by default.  Explicitly configure with VREG
    // voltage so the PLL reliably locks on both ARM and RISC-V cores.
    vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(150000, true);

    // Initialise UART1 FIRST so we can print debug output immediately,
    // even if CYW43 init later fails or hangs (critical on RISC-V).
    uart1_init();

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    PRINTF("RP2350 - Pico 2 W board\n");
    PRINTF("CPU: %s @ %u MHz\n", CPU_ARCH, sys_clk_hz / 1000000);
    PRINTF("System clock: %u Hz (%u MHz)\n", sys_clk_hz,
           sys_clk_hz / 1000000);

    // CYW43 wireless chip init — only for the on-board LED indicator.
    // On RISC-V this is skipped because the CYW43 PIO SPI driver may not
    // be fully functional, and we don't need it for a Dhrystone benchmark.
#if !defined(__riscv)
    cyw43_arch_init();
#endif

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
			
#if !defined(__riscv)
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 1); // LED solid during cached test
#endif
            run_cached_dhry(sys_clk_hz);
			PRINTF("CACHED run complete. %s\n", COMPILER_NAME);
			sleep_ms(3000);

			// --- TEST 2: UNCACHED FLASH PERFORMANCE ---
			PRINTF("\n[2/2] Starting UNCACHED flash run (Target: 0x%08X)...\n", uncached_address);
			PRINTF("Expect this to run significantly slower!\n");
			
#if !defined(__riscv)
			// Blink fast right before it enters slow mode so you know it's switching
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 0);
			sleep_ms(200);
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 1);
			sleep_ms(200);
			cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN, 0);
#endif

#if defined(__riscv)
            // On RISC-V, the XIP uncached alias (0x14000000) causes a bus fault
            // on the Hazard3 core — the chip resets or hangs silently.
            // This is likely an RP2350 erratum or bus-attribute mismatch;
            // skip the uncached test on RISC-V until root-caused.
            PRINTF("UNCACHED test skipped on RISC-V (HW limitation)\n");
            PRINTF("See RP2350 datasheet, XIP uncached alias behaviour.\n");
#else
            // This executes the exact code out of flash but forces a raw 
            // serial QSPI pin lookup for every loop fetch cycle.
            run_uncached_dhry(sys_clk_hz);
			
			PRINTF("UNCACHED run complete. %s\n", COMPILER_NAME);
#endif
            sleep_ms(3000);
	}
}