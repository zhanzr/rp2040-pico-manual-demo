#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/platform.h"
#include "hardware/structs/sio.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/sync.h"
#include "hardware/structs/xip_ctrl.h"
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"

//extern volatile uint32_t g_ticks;

	// Returns a 64-bit millisecond tick count since boot
static inline uint64_t get_pico_ticks_ms(void) {
    return time_us_64() / 1000;
}

uint32_t HAL_GetTick(void) {
	return get_pico_ticks_ms();
}

void HAL_Delay(uint32_t t) {
  uint32_t d = t + get_pico_ticks_ms();
  while (d > get_pico_ticks_ms()) {
    __asm("nop");
  }
}
