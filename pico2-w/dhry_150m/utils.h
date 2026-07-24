#ifndef __UTILS_H__
#define	__UTILS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_FUNC
// #define RAM_FUNC __attribute__((section(".time_critical")))

// -------------------------------------------------------------------------
// UART1 debug output (GP4 TX, GP5 RX)
//
// Call uart1_init() once at startup, then use PRINTF(...) like printf().
// -------------------------------------------------------------------------
void uart1_init(void);
void uart_printf(const char *fmt, ...);
#define PRINTF(...)  uart_printf(__VA_ARGS__)

uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t t);

#ifdef __cplusplus
}
#endif

#endif	// __UTILS_H__
