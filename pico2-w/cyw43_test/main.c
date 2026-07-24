/**
 * \file main.c
 * \brief CYW43 wireless chip test for Pico 2 W:
 *        initialises the on-board CYW43 WiFi, scans for APs,
 *        joins a configured network, and prints connection status.
 *
 * Hardware:
 *   - CYW43 WiFi chip (connected via internal SPI — no external wiring needed)
 *   - UART1 on GP4/GP5 for debug output at 115200 baud
 *   - Onboard LED controlled via CYW43 GPIO
 *
 * Usage:
 *   Copy wifi_config.h.example to wifi_config.h and fill in your AP
 *   credentials before building.
 *
 * Clock runs at 150 MHz (max spec).
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/clocks.h"
#include "hardware/uart.h"

#include "lwip/netif.h"
#include "lwip/ip_addr.h"

// WiFi credentials — create wifi_config.h from the .example file.
// The build will fail if this file is missing or empty.
#include "wifi_config.h"

#ifndef DEFAULT_AP
#error "wifi_config.h is missing DEFAULT_AP. Copy wifi_config.h.example to wifi_config.h and fill in your credentials."
#endif

// -------------------------------------------------------------------------
// Configurable parameters
// -------------------------------------------------------------------------
#define UART1_BAUDRATE  115200
#define BUF_SIZE        256

// -------------------------------------------------------------------------
// UART1 debug output helpers (GP4 TX, GP5 RX)
// -------------------------------------------------------------------------
static void uart1_init(void)
{
    uart_init(uart1, UART1_BAUDRATE);
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);
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

#define PRINTF(...)  u1_printf(__VA_ARGS__)

// -------------------------------------------------------------------------
// Scan result callback — prints each AP found
// -------------------------------------------------------------------------
static int scan_result_cb(void *env, const cyw43_ev_scan_result_t *result)
{
    if (result) {
        // Indicate security type.
        // auth_mode is a bitmap parsed by the CYW43 firmware driver
        // (see cyw43_ll_wifi_parse_scan_result in cyw43_ll.c):
        //   bit 0: WEP
        //   bit 1: WPA
        //   bit 2: WPA2
        const char *auth;
        uint8_t mode = result->auth_mode;
        if (mode == 0) {
            auth = "Open";
        } else if (mode & 4) {
            auth = (mode & 2) ? "WPA2/WPA" : "WPA2";
        } else if (mode & 2) {
            auth = "WPA";
        } else if (mode & 1) {
            auth = "WEP";
        } else {
            auth = "?";
        }
        PRINTF("  [%02d] %-32s  %4d dBm  ch%2d  %s\n",
               *(int *)env, result->ssid, result->rssi, result->channel, auth);
        (*(int *)env)++;
    }
    return 0;
}

// -------------------------------------------------------------------------
// Connect to an AP with visible progress feedback
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// Connect to an AP — wrapper around the official blocking call
// -------------------------------------------------------------------------
static int connect_with_progress(const char *ssid, const char *pw,
                                 uint32_t auth, uint32_t timeout_ms)
{
    PRINTF("  Connecting...");
    int err = cyw43_arch_wifi_connect_timeout_ms(ssid, pw, auth, timeout_ms);
    if (err) {
        PRINTF("\n  FAILED (err=%d)\n", err);
    } else {
        PRINTF("\n  LINK_UP\n");
    }
    return err;
}

// -------------------------------------------------------------------------
// Print assigned IP address
// -------------------------------------------------------------------------
static void print_ip_address(void)
{
    struct netif *netif = netif_default;
    if (netif && netif_is_up(netif)) {
        PRINTF("  IP: %s\n", ipaddr_ntoa(&netif->ip_addr));
        PRINTF("  GW: %s\n", ipaddr_ntoa(&netif->gw));
        PRINTF("  NM: %s\n", ipaddr_ntoa(&netif->netmask));
    } else {
        PRINTF("  No IP address assigned\n");
    }
}

// -------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------
int main(void)
{
    set_sys_clock_khz(150000, false);

    // Initialise UART1 as the debug output port (GP4 TX, GP5 RX)
    uart1_init();

    PRINTF("\n");
    PRINTF("========================================\n");
    PRINTF("  RP2350 - Pico 2 W (150 MHz)\n");
    PRINTF("  CYW43 Wireless Test\n");
    PRINTF("========================================\n");
    PRINTF("UART1 debug output on GP4/GP5 at %u baud\n", UART1_BAUDRATE);
    PRINTF("System clock: %u Hz (%u MHz)\n", clock_get_hz(clk_sys),
           clock_get_hz(clk_sys) / 1000000);

    // --------------------------------------------------------------
    // Initialise the CYW43 wireless driver
    // --------------------------------------------------------------
    PRINTF("\n--- Initialising CYW43 wireless chip ---\n");
    int err = cyw43_arch_init_with_country(CYW43_COUNTRY_CHINA);
    if (err) {
        PRINTF("ERROR: cyw43_arch_init() failed (%d)\n", err);
        PRINTF("Check that the CYW43 chip is properly powered.\n");
        while (true) {
            tight_loop_contents();
        }
    }
    PRINTF("CYW43 initialised OK (country: %c%c rev %u)\n",
           (char)(cyw43_arch_get_country_code() & 0xff),
           (char)((cyw43_arch_get_country_code() >> 8) & 0xff),
           (cyw43_arch_get_country_code() >> 16) & 0xff);

    // Enable station (client) mode
    cyw43_arch_enable_sta_mode();
    PRINTF("Station mode enabled\n");

    // --------------------------------------------------------------
    // Scan for available APs
    // --------------------------------------------------------------
    PRINTF("\n--- Scanning for available APs ---\n");
    cyw43_wifi_scan_options_t scan_opts = {0};
    int ap_count = 0;
    err = cyw43_wifi_scan(&cyw43_state, &scan_opts, &ap_count, scan_result_cb);
    if (err) {
        PRINTF("ERROR: Scan failed (%d)\n", err);
    } else {
        // Wait for scan to complete (background worker handles CYW43 events)
        uint32_t scan_start = to_ms_since_boot(get_absolute_time());
        while (cyw43_wifi_scan_active(&cyw43_state)) {
            if (to_ms_since_boot(get_absolute_time()) - scan_start > 10000) {
                PRINTF("  Scan timed out\n");
                break;
            }
            sleep_ms(10);
        }
        PRINTF("  Found %d AP(s)\n", ap_count);
    }

    // --------------------------------------------------------------
    // Connect to the configured WiFi network
    // --------------------------------------------------------------
    // Official pico-examples uses WPA2_AES_PSK; try WPA2-only first, then
    // fall back to WPA2/WPA mixed mode.
    PRINTF("\n--- Connecting to \"%s\" (WPA2-AES, 30s timeout) ---\n",
           DEFAULT_AP);

    int conn = connect_with_progress(DEFAULT_AP, DEFAULT_PASSWD,
                                     CYW43_AUTH_WPA2_AES_PSK, 30000);
    if (conn == 0) {
        PRINTF("  Connected successfully\n");
        print_ip_address();
    } else {
        PRINTF("\n  WPA2-AES failed. Trying WPA2/WPA mixed mode ...\n");
        conn = connect_with_progress(DEFAULT_AP, DEFAULT_PASSWD,
                                     CYW43_AUTH_WPA2_MIXED_PSK, 20000);
        if (conn == 0) {
            PRINTF("  Connected successfully (WPA2-MIXED)\n");
            print_ip_address();
        } else {
            PRINTF("  Fallback also failed (%d). Check credentials.\n", conn);
        }
    }

    // --------------------------------------------------------------
    // Main loop — blink LED and report status periodically
    // --------------------------------------------------------------
    PRINTF("\n--- Entering main loop ---\n");

    // Initial state: flash quickly during WiFi init
    uint32_t loop_count = 0;

    while (true)
    {
        // Check connection status every 30 seconds
        if (loop_count % 30 == 0) {
            int link = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
            PRINTF("--- WiFi status: %s ---\n", link ? "UP" : "DOWN");
            if (link)
                print_ip_address();
        }

        // Blink LED (1 Hz)
        cyw43_gpio_set(&cyw43_state, CYW43_WL_GPIO_LED_PIN,
                       loop_count & 1);

        loop_count++;
        sleep_ms(500);
    }
}
