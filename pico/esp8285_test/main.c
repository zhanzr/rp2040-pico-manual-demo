/**
 * \file main.c
 * \brief ESP8285 WiFi module test: communicates via UART0 (GP0/GP1)
 *        using AT commands, prints debug output via UART1 (GP4/GP5).
 *
 * Usage:
 *   - UART0 (GP0 TX, GP1 RX) — connected to ESP8285 UART.
 *   - UART1 (GP4 TX, GP5 RX) — debug/STDOUT output via PRINTF().
 *
 * Clock defaults to 200 MHz (overclocked).  To use 125 MHz, comment
 * out the OVERCLOCK block and uncomment the NORMAL line.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/vreg.h"

// WiFi credentials — create wifi_config.h from the .example file.
// The build will fail if this file is missing or empty.
#include "wifi_config.h"

#ifndef DEFAULT_AP
#error "wifi_config.h is missing DEFAULT_AP. Copy wifi_config.h.example to wifi_config.h and fill in your credentials."
#endif

// -------------------------------------------------------------------------
// Configurable parameters
// -------------------------------------------------------------------------
#define UART0_BAUDRATE  115200
#define UART1_BAUDRATE  115200

/** Print buffer size (256 covers most AT command responses).
 *  Messages longer than this will be silently truncated. */
#define BUF_SIZE 256

// -------------------------------------------------------------------------
// UART output helpers
//
//   u0_printf() — send formatted string via UART0 (GP0/GP1)
//   u1_printf() — send formatted string via UART1 (GP4/GP5)
//   PRINTF(...)  — alias for u1_printf (STDOUT-style output)
// -------------------------------------------------------------------------
static void u0_printf(const char *fmt, ...)
{
    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    for (const char *p = buf; *p; p++) {
        if (*p == '\n')
            uart_putc_raw(uart0, '\r');
        uart_putc_raw(uart0, *p);
    }
}

static void u1_printf(const char *fmt, ...)
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

/** Default output goes to UART1 (the debug/STDOUT port). */
#define PRINTF(...)  u1_printf(__VA_ARGS__)

// -------------------------------------------------------------------------
// UART0 read helpers (ESP8285 AT command responses)
// -------------------------------------------------------------------------
/** Response timeout in milliseconds */
#define AT_TIMEOUT_MS 3000

/** Number of APs to display at most */
#define MAX_APS 20

/**
 * Send a raw string via UART0 (no \r\n appended).
 */
static void u0_send(const char *s)
{
    while (*s) {
        uart_putc(uart0, *s++);
    }
}

/**
 * Send an AT command (appends \r\n) via UART0.
 */
static void u0_at_cmd(const char *cmd)
{
    u0_send(cmd);
    uart_putc(uart0, '\r');
    uart_putc(uart0, '\n');
}

/**
 * Read one line from UART0 into buf (up to buf_size-1 bytes).
 * A line ends at '\n' or timeout.  Returns the number of bytes
 * read (excluding null terminator), or -1 on timeout.
 */
static int u0_read_line(char *buf, int buf_size, uint32_t timeout_ms)
{
    int i = 0;
    uint32_t start = to_ms_since_boot(get_absolute_time());
    buf[0] = '\0';

    while (i < buf_size - 1) {
        if (uart_is_readable(uart0)) {
            char c = uart_getc(uart0);
            if (c == '\n') {
                buf[i] = '\0';
                // Trim trailing \r if present
                if (i > 0 && buf[i - 1] == '\r')
                    buf[i - 1] = '\0';
                return i;
            }
            buf[i++] = c;
        } else {
            if (to_ms_since_boot(get_absolute_time()) - start >= timeout_ms)
                return -1;  // timeout
            tight_loop_contents();
        }
    }
    buf[i] = '\0';
    return i;
}

/**
 * Flush any pending UART0 input.
 */
static void u0_flush(void)
{
    while (uart_is_readable(uart0))
        uart_getc(uart0);
}

/**
 * Send an AT command and check whether the response contains the
 * expected string (e.g. "OK").
 */
static bool u0_at_expect(const char *cmd, const char *expected,
                         uint32_t timeout_ms)
{
    u0_flush();
    u0_at_cmd(cmd);

    char buf[BUF_SIZE];
    uint32_t start = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        int n = u0_read_line(buf, sizeof(buf), 200);
        if (n < 0) continue;   // partial timeout, keep waiting
        if (strstr(buf, expected))
            return true;
    }
    return false;
}

/**
 * Send the AT command, print all response lines, and return true if
 * the final line contains the expected string.
 */
static bool u0_at_show(const char *cmd, const char *expected,
                       uint32_t timeout_ms)
{
    u0_flush();
    u0_at_cmd(cmd);

    char buf[BUF_SIZE];
    uint32_t start = to_ms_since_boot(get_absolute_time());
    bool ok = false;

    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        int n = u0_read_line(buf, sizeof(buf), 200);
        if (n < 0) continue;
        PRINTF("  [ESP] %s\n", buf);
        if (strstr(buf, expected))
            ok = true;
    }
    return ok;
}

// -------------------------------------------------------------------------
// ESP8285 WiFi module — public API
// -------------------------------------------------------------------------

// Cache for module availability
static bool g_wifi_checked = false;
static bool g_wifi_available = false;

bool is_wifi_module_available(bool force_check)
{
    if (g_wifi_checked && !force_check)
        return g_wifi_available;

    u0_flush();
    PRINTF("Checking ESP8285 WiFi module ...\n");

    // The ESP8285 boot  message (if just powered) contains "ready".
    // Send a few AT commands with a brief gap to wake it up.
    for (int i = 0; i < 3; i++) {
        u0_at_cmd("AT");
        sleep_ms(100);
    }

    // Now expect "OK" within the timeout period.
    g_wifi_available = u0_at_expect("AT", "OK", AT_TIMEOUT_MS);
    g_wifi_checked = true;

    if (!g_wifi_available) {
        PRINTF("  ERROR: ESP8285 not responding\n");
        return false;
    }

    PRINTF("  ESP8285 responding OK\n");

    // Set station-only mode (pure client — disables the built-in SoftAP).
    // This setting is saved in flash and persists across reboots, but
    // we set it every boot for robustness (handles module replacement).
    PRINTF("  Configuring station-only mode ...\n");
    if (u0_at_expect("AT+CWMODE=1", "OK", 2000)) {
        PRINTF("  OK — ESP8285 now in station (client) mode\n");
        sleep_ms(500);  // allow mode switch to settle
    } else {
        PRINTF("  WARNING: AT+CWMODE=1 failed, continuing anyway\n");
    }

    return true;
}

void print_out_available_ap_list(void)
{
    if (!is_wifi_module_available(false)) {
        PRINTF("ERROR: WiFi module not available, cannot scan APs\n");
        return;
    }

    PRINTF("Scanning for available APs ...\n");
    // Retry CWLAP once if it fails (the ESP may need extra time after mode
    // switch or boot to initialise the Wi-Fi radio).
    for (int attempt = 0; attempt < 2; attempt++) {
        if (u0_at_show("AT+CWLAP", "OK", 10000))
            return;
        if (attempt == 0) {
            PRINTF("  Scan failed, retrying ...\n");
            sleep_ms(2000);
        }
    }
    PRINTF("  No APs found or scan failed\n");
}

void join_wifi_ap(const char *ssid, const char *password)
{
    if (!is_wifi_module_available(false)) {
        PRINTF("ERROR: WiFi module not available, cannot join AP\n");
        return;
    }

    PRINTF("Joining AP \"%s\" ...\n", ssid);

    // Build the AT+CWJAP command with escaped quotes around SSID/password.
    char cmd[BUF_SIZE];
    int n = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if (n >= (int)sizeof(cmd)) {
        PRINTF("ERROR: SSID/password too long (%d chars max)\n",
               (int)sizeof(cmd) - 30);
        return;
    }

    u0_flush();
    u0_at_cmd(cmd);

    // CWJAP can take a while — print progress lines as they arrive.
    char buf[BUF_SIZE];
    uint32_t start = to_ms_since_boot(get_absolute_time());
    bool ok = false;

    while (to_ms_since_boot(get_absolute_time()) - start < 15000) {
        int r = u0_read_line(buf, sizeof(buf), 500);
        if (r < 0) continue;
        PRINTF("  [ESP] %s\n", buf);
        if (strstr(buf, "OK")) {
            ok = true;
            break;
        }
        if (strstr(buf, "FAIL") || strstr(buf, "ERROR")) {
            break;
        }
    }

    if (ok) {
        PRINTF("  Successfully joined AP \"%s\"\n", ssid);
        // Optionally show the assigned IP.
        u0_at_show("AT+CIFSR", "OK", 3000);
    } else {
        PRINTF("  Failed to join AP \"%s\"\n", ssid);
    }
}

int main(void)
{
    // ==================================================================
    // OVERCLOCK to 200 MHz (default)
    // ==================================================================
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    sleep_ms(10);
    set_sys_clock_khz(200000, false);

    // ==================================================================
    // NORMAL 125 MHz  (comment the block above, uncomment the line below)
    // ==================================================================
    // set_sys_clock_khz(125000, true);

    // ------------------------------------------------------------------
    // Initialise both UARTs — raw hardware, no stdio
    // ------------------------------------------------------------------
    uart_init(uart0, UART0_BAUDRATE);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    uart_init(uart1, UART1_BAUDRATE);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    const uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    const uint32_t sys_clk_mhz = sys_clk_hz / 1000000;

    PRINTF("RP2040 - Pico board (%u MHz)\n", sys_clk_mhz);
    PRINTF("UART0 on GP0/GP1 at %u baud  (device comms)\n", UART0_BAUDRATE);
    PRINTF("UART1 on GP4/GP5 at %u baud  (debug output)\n", UART1_BAUDRATE);
    PRINTF("System clock: %u Hz (%u MHz)\n", sys_clk_hz, sys_clk_mhz);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t loop_count = 0;

    // --------------------------------------------------------------
    // Test the ESP8285 WiFi module on UART0
    // --------------------------------------------------------------
    is_wifi_module_available(false);

    // Scan & join happen only once, on the first pass.
    bool wifi_initialised = false;

    while (true)
    {
        // Print system tick count every 5 seconds.
        // if (loop_count % 5 == 0)
        // {
        //     const uint64_t tick_us = time_us_64();
        //     const uint64_t tick_ms = tick_us / 1000;
        //     PRINTF("%u Hz, %llu ms\n", sys_clk_hz, tick_ms);
        // }

        // Once the module is confirmed, scan APs then join the default AP.
        if (!wifi_initialised && is_wifi_module_available(false)) {
            print_out_available_ap_list();

            join_wifi_ap(DEFAULT_AP, DEFAULT_PASSWD);

            wifi_initialised = true;
        }

        // Print WiFi network status every 30 seconds when connected.
        if (wifi_initialised && loop_count % 30 == 0) {
            PRINTF("--- WiFi status ---\n");
            u0_at_show("AT+CWJAP?",  "OK", 2000);   // AP info + RSSI
            u0_at_show("AT+CIFSR",   "OK", 2000);   // allocated IP
            PRINTF("-------------------\n");
        }

        loop_count++;

        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}
