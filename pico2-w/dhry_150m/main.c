// clk_peri tracks clk_sys automatically

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
#include "hardware/structs/xip.h"     // xip_ctrl_hw, XIP_CTRL_*_BITS
#include "hardware/uart.h"
#include "hardware/vreg.h"

#include "custom_def.h"
#include "dhry.h"

volatile uint32_t g_ticks;
extern uint32_t __INITIAL_SP;
extern void _stage2_boot(void);

extern void Proc_5 (void);

int main(void) {
    // RP2350 boots at 150 MHz by default.  Explicitly configure with VREG
    // voltage so the PLL reliably locks on both ARM and RISC-V cores.
    vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(150000, true);

    // Initialise UART1 FIRST so we can print debug output immediately,
    // even if CYW43 init later fails or hangs (critical on RISC-V).
    uart1_init();

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    PRINTF("RP2350 - Pico 2 W board (%s)\n", CPU_ARCH);
    PRINTF("CPU: %s @ %u MHz\n", CPU_ARCH, sys_clk_hz / 1000000);
    PRINTF("System clock: %u Hz (%u MHz)\n", sys_clk_hz,
           sys_clk_hz / 1000000);

    PRINTF("\nRP2350 XIP Cache Benchmarker\n");

    while (true) {
        // ----------------------------------------------------------------
        // TEST 1: CACHED FLASH
        //   Normal execution from cached flash address 0x10000000+
        //   The XIP cache accelerates repeated instruction fetches.
        // ----------------------------------------------------------------
        PRINTF("\n[1/2] Starting CACHED flash run...\n");
        dhry_main(sys_clk_hz);
        PRINTF("CACHED run complete. %s\n", COMPILER_NAME);
        sleep_ms(3000);

        // ----------------------------------------------------------------
        // TEST 2: UNCACHED FLASH (cache disabled)
        //   Same code, same flash address, but the XIP cache is disabled
        //   so every instruction fetch goes directly to the QSPI flash.
        //
        //   The RP2350 XIP uncached alias (0x14000000) causes a bus fault
        //   on the RISC-V Hazard3 core, so instead we use the cache
        //   control register to disable caching, then re-enable after.
        //   This approach works identically on ARM and RISC-V.
        // ----------------------------------------------------------------
        PRINTF("\n[2/2] Starting UNCACHED flash run (cache disabled)...\n");
        PRINTF("Expect this to run significantly slower!\n");

        // Power-down the cache — this clears all cached lines and forces
        // the enable bits to 0 per the RP2350 datasheet.
        uint32_t saved_ctrl = xip_ctrl_hw->ctrl;
        xip_ctrl_hw->ctrl = saved_ctrl | XIP_CTRL_POWER_DOWN_BITS;
        __asm volatile("" ::: "memory");

        // Power back up, keeping cache disabled (EN bits cleared).
        xip_ctrl_hw->ctrl = saved_ctrl
            & ~(XIP_CTRL_EN_SECURE_BITS | XIP_CTRL_EN_NONSECURE_BITS);
        __asm volatile("" ::: "memory");

        dhry_main(sys_clk_hz);

        // Restore original cache state.
        xip_ctrl_hw->ctrl = saved_ctrl;
        __asm volatile("" ::: "memory");

        PRINTF("UNCACHED run complete. %s\n", COMPILER_NAME);
        sleep_ms(3000);
    }
}