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

# Generate .uf2 using the picotool pre-built with mini_blink.
set(PICOTOOL_EXE "D:/rp2040_vs_demo/mini_blink/build/_deps/picotool/picotool.exe")
if(EXISTS "${PICOTOOL_EXE}")
    add_custom_command(TARGET dhry_200m  POST_BUILD
        COMMAND "${PICOTOOL_EXE}" uf2 convert dhry_200m.elf dhry_200m.uf2
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating dhry_200m.uf2"
        BYPRODUCTS dhry_200m.uf2
    )
endif()

# -------------------------------------------------------------------------
# Custom target: flash — program the Pico via OpenOCD + CMSIS-DAP
# -------------------------------------------------------------------------
add_custom_target(flash
    COMMAND openocd
        -f interface/cmsis-dap.cfg
        -c "adapter speed 10000"
        -f target/rp2040.cfg
        -c "cmsis_dap_backend usb_bulk"
        -c "init; reset halt; program ${CMAKE_CURRENT_BINARY_DIR}/dhry_200m.elf verify reset exit"
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
