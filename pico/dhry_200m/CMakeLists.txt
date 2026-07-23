cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(dhry_200m C CXX ASM)
set(CMAKE_C_STANDARD 11)

# Most aggressive speed optimization flags
add_compile_options(-Ofast -funroll-loops)

pico_sdk_init()

add_executable(dhry_200m
        main.c
        dhry_1.c
        dhry_2.c
        utils.c
        )

target_link_libraries(dhry_200m pico_stdlib hardware_vreg)

# Disable stdio backends — using raw UART API via utils.c
pico_enable_stdio_uart(dhry_200m 0)
pico_enable_stdio_usb(dhry_200m 0)

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
            "${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.elf"
            "${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.uf2"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating dhry_200m.uf2 …"
        DEPENDS dhry_200m
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.uf2"
    )
else()
    add_custom_target(uf2
        COMMAND ${CMAKE_COMMAND} -E echo
            "picotool not found — install picotool or set picotool_DIR")
endif()

# -------------------------------------------------------------------------
# Flashing configuration — set via cmake -D to match your debug probe.
#
# Interface examples:
#   Picoprobe / CMSIS-DAP  (default — see OPENOCD_ADAPTER_ARGS below)
#   J-Link:                -DOPENOCD_INTERFACE=jlink.cfg -DOPENOCD_ADAPTER_ARGS=
#   ST-Link:               -DOPENOCD_INTERFACE=stlink.cfg -DOPENOCD_ADAPTER_ARGS=
#
# If usb_bulk doesn't find your probe, try HID auto-detect:
#   cmake -DOPENOCD_ADAPTER_ARGS= ..
# -------------------------------------------------------------------------
set(OPENOCD_INTERFACE "interface/cmsis-dap.cfg" CACHE STRING
    "OpenOCD interface config file")
set(OPENOCD_ADAPTER_SPEED "5000" CACHE STRING
    "OpenOCD SWD clock speed in kHz")
set(OPENOCD_ADAPTER_ARGS "-c;cmsis_dap_backend usb_bulk" CACHE STRING
    "Extra OpenOCD adapter arguments (semicolon-separated; empty for J-Link/HID)")

# Custom target: flash — program the Pico via OpenOCD
add_custom_target(flash
    COMMAND openocd
        -f "${OPENOCD_INTERFACE}"
        -c "adapter speed ${OPENOCD_ADAPTER_SPEED}"
        -f target/rp2040.cfg
        ${OPENOCD_ADAPTER_ARGS}
        -c init
        -c "reset halt"
        -c "program ${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.elf verify reset exit"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Flashing dhry_200m.elf to RP2040 via OpenOCD …"
    USES_TERMINAL
)

# -------------------------------------------------------------------------
# Custom target: clean-all — deep clean compiled objects and SDK files
#   Removes all .o / .elf / .uf2 files and pico-sdk copies, but preserves
#   CMakeCache.txt + build.ninja so you can rebuild immediately with `ninja`
#   without re-supplying -D flags. To fully wipe + reconfigure, remove
#   CMakeCache.txt manually and re-run cmake.
# -------------------------------------------------------------------------
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/dhry_200m.dir"
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
        "${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.elf"
        "${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.uf2"
    COMMAND ${CMAKE_COMMAND} -E echo "Deep clean done. Run 'ninja' to rebuild with cached configuration."
    COMMENT "Removing all build artifacts (keeps CMakeCache.txt) …"
    USES_TERMINAL
)
