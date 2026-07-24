#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Minimal lwIP configuration for Pico 2 W cyw43 LED / driver support.
// No networking is used — only the cyw43 GPIO for the onboard LED.

#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_RAW                    1
#define LWIP_DHCP                   0
#define LWIP_DNS                    0
#define LWIP_NETIF_STATUS_CALLBACK  0
#define LWIP_NETIF_LINK_CALLBACK    0

#endif
