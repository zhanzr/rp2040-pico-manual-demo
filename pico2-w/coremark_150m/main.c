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
#include "hardware/structs/xip.h"
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

int main(void)
{
    vreg_set_voltage(VREG_VOLTAGE_1_10);
    set_sys_clock_khz(150000, true);

    uart1_init();

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);

    PRINTF("RP2350 - Pico 2 W board (%s)\n", CPU_ARCH);
    PRINTF("CPU: %s @ %u MHz\n", CPU_ARCH, sys_clk_hz / 1000000);

    while(1) {
        // --- TEST 1: CACHED FLASH ---
        PRINTF("\n[1/2] Starting CACHED flash run...\n");
        coremark_main();
        PRINTF("CACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
        sleep_ms(3000);

        // --- TEST 2: UNCACHED FLASH (cache disabled) ---
        // Disable the XIP cache via hardware so every instruction fetch
        // goes directly to QSPI flash.  This works identically on both
        // ARM and RISC-V (unlike the 0x14000000 alias which faults on
        // Hazard3).
        PRINTF("\n[2/2] Starting UNCACHED flash run (cache disabled)...\n");
        PRINTF("Expect this to run significantly slower!\n");

        uint32_t saved_ctrl = xip_ctrl_hw->ctrl;
        xip_ctrl_hw->ctrl = saved_ctrl | XIP_CTRL_POWER_DOWN_BITS;
        __asm volatile("" ::: "memory");

        xip_ctrl_hw->ctrl = saved_ctrl
            & ~(XIP_CTRL_EN_SECURE_BITS | XIP_CTRL_EN_NONSECURE_BITS);
        __asm volatile("" ::: "memory");

        coremark_main();

        xip_ctrl_hw->ctrl = saved_ctrl;
        __asm volatile("" ::: "memory");

        PRINTF("UNCACHED run complete. %u %s\n", sys_clk_hz, COMPILER_NAME);
        sleep_ms(3000);
    }
}