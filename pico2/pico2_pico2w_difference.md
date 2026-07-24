**Yes, conceptually and architecturally, that is an accurate mental model**.

The Pico 2 W is built on the exact same core hardware platform as the standard Pico 2 (RP2350 microcontroller, 520 KB SRAM, 4 MB Flash). The primary hardware addition is an onboard **Infineon CYW43439** chip (providing 2.4 GHz Wi-Fi 4 and Bluetooth 5.2), which communicates with the RP2350 over an internal SPI bus.

The specific internal differences between the two boards are detailed below:

---

### 1. Internal GPIO Reassignments

To communicate with the CYW43439 wireless chip, Raspberry Pi remapped four internal RP2350 GPIO pins on the Pico 2 W:

| Internal Function | Standard Pico 2 | Pico 2 W |
| --- | --- | --- |
| **GPIO25** | Onboard User LED | **Wireless SPI CS** (Chip Select) |
| **GPIO24** | VBUS Sense (USB power detection) | **Wireless SPI Data / IRQ** |
| **GPIO29** | ADC3 (VSYS voltage check) | **Wireless SPI CLK** / VSYS Sense |
| **GPIO23** | SMPS Power Save control | **Wireless Power On** (`WL_REG_ON`) |

*(Note: The external 40-pin header pinout—GPIO0 through GPIO22 and GPIO26 through GPIO28—remains identical on both boards.)*

---

### 2. What Happened to the Onboard LED?

* **Standard Pico 2:** The LED is hardwired directly to RP2350 **GPIO25**. Toggling GPIO25 in software blinks the light directly.
* **Pico 2 W:** The LED is physically connected to pin **`WL_GPIO0` on the CYW43439 chip itself**.

Because the LED is controlled by the wireless module rather than the main microcontroller directly, software cannot blink it using a simple `gpio_put(25, 1)`. Instead, the firmware must initialize the `cyw43` driver architecture and send an SPI command to the wireless chip to toggle the LED state.

---

### Summary

If you take a standard Pico 2, reassign **GPIO23–25** and **GPIO29** to run the CYW43439 SPI link, and move the LED control line to the wireless module's GPIO pin, you get the Pico 2 W.