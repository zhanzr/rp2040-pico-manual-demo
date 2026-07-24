cmake_minimum_required(VERSION 3.13)

# Target platform & board — must be set BEFORE project() and
# pico_sdk_import.cmake so the SDK selects the right toolchain.
set(PICO_PLATFORM rp2350)
set(PICO_BOARD   pico2)

include(pico_sdk_import.cmake)

project(dhry_150m C CXX ASM)
set(CMAKE_C_STANDARD 11)

# Most aggressive speed optimization flags
add_compile_options(-Ofast -funroll-loops)

pico_sdk_init()

add_executable(dhry_150m
        main.c
        dhry_1.c
        dhry_2.c
        utils.c
        )

target_link_libraries(dhry_150m pico_stdlib hardware_vreg)

# Disable stdio backends — using raw UART API via utils.c
pico_enable_stdio_uart(dhry_150m 0)
pico_enable_stdio_usb(dhry_150m 0)

# -------------------------------------------------------------------------
# Custom target: uf2 — generate .uf2 from .elf (explicit, not automatic)
#   Run:  ninja uf2          # generates .uf2 for BOOTSEL-mode flashing
#   Build only .elf: ninja   # default target; flash via: ninja flash
# -------------------------------------------------------------------------
find_program(PICOTOOL_BIN picotool
    HINTS "$ENV{HOME}/tool/picotool" "$ENV{USERPROFILE}/tool/picotool"
    DOC "picotool executable path")
if(PICOTOOL_BIN)
    add_custom_target(uf2
        COMMAND "${PICOTOOL_BIN}" uf2 convert
            "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.elf"
            "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.uf2"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating dhry_150m.uf2 …"
        DEPENDS dhry_150m
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.uf2"
    )
else()
    add_custom_target(uf2
        COMMAND ${CMAKE_COMMAND} -E echo
            "picotool not found — install picotool or set picotool_DIR")
endif()

# -------------------------------------------------------------------------
# Flashing — probe-rs (default) and OpenOCD (fallback)
#
# Default (probe-rs):
#   ninja flash
#
# Fallback (OpenOCD — for probes not supported by probe-rs):
#   ninja flash-ocd
#
# Interface examples for OpenOCD:
#   CMSIS-DAP / Picoprobe  (default)
#   J-Link:                cmake -DOPENOCD_INTERFACE=jlink.cfg -DOPENOCD_ADAPTER_ARGS= ..
#   ST-Link:               cmake -DOPENOCD_INTERFACE=stlink.cfg -DOPENOCD_ADAPTER_ARGS= ..
# -------------------------------------------------------------------------

# Custom target: flash — program the Pico 2 via probe-rs
add_custom_target(flash
    COMMAND probe-rs download --chip RP235x --speed 10000
            "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.elf"
    COMMAND probe-rs reset --chip RP235x
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Flashing dhry_150m.elf to RP2350 via probe-rs …"
    USES_TERMINAL
)

set(OPENOCD_INTERFACE "interface/cmsis-dap.cfg" CACHE STRING
    "OpenOCD interface config file")
set(OPENOCD_ADAPTER_SPEED "5000" CACHE STRING
    "OpenOCD SWD clock speed in kHz")
set(OPENOCD_ADAPTER_ARGS "-c;cmsis_dap_backend usb_bulk" CACHE STRING
    "Extra OpenOCD adapter arguments (semicolon-separated; empty for J-Link/HID)")

# Custom target: flash-ocd — fallback using OpenOCD
add_custom_target(flash-ocd
    COMMAND openocd
        -f "${OPENOCD_INTERFACE}"
        -c "adapter speed ${OPENOCD_ADAPTER_SPEED}"
        -f target/rp2350.cfg
        ${OPENOCD_ADAPTER_ARGS}
        -c init
        -c "reset halt"
        -c "program ${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.elf verify reset exit"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Flashing dhry_150m.elf to RP2350 via OpenOCD …"
    USES_TERMINAL
)

# -------------------------------------------------------------------------
# Custom target: clean-all — deep clean compiled objects and SDK files
#   Removes all .o / .elf / .uf2 files and pico-sdk copies, but preserves
#   CMakeCache.txt + build.ninja so you can rebuild immediately with `ninja`
# -------------------------------------------------------------------------
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/dhry_150m.dir"
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/pkgRedirects"
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/CMakeScratch"
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cmake.check_cache"
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/TargetDirectories.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/InstallScripts.json"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/generated"
        "${CMAKE_CURRENT_BINARY_DIR}/pico-sdk"
        "${CMAKE_CURRENT_BINARY_DIR}/pioasm"
        "${CMAKE_CURRENT_BINARY_DIR}/pioasm-install"
        "${CMAKE_CURRENT_BINARY_DIR}/.ninja_log"
        "${CMAKE_CURRENT_BINARY_DIR}/.ninja_deps"
        "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.elf"
        "${CMAKE_CURRENT_BINARY_DIR}/dhry_150m.uf2"
    COMMAND ${CMAKE_COMMAND} -E echo "Deep clean done. Run 'ninja' to rebuild with cached configuration."
    COMMENT "Removing all build artifacts (keeps CMakeCache.txt) …"
    USES_TERMINAL
)
