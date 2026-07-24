#include <stdarg.h>
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
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"

#define UART1_BAUDRATE 115200
#define BUF_SIZE       128

void uart1_init(void)
{
    uart_init(uart1, UART1_BAUDRATE);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);
}

void uart_printf(const char *fmt, ...)
{
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for (const char *p = buf; *p; p++) {
        if (*p == '\n')
            uart_putc_raw(uart1, '\r');
        uart_putc_raw(uart1, *p);
    }
}

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
