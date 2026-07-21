cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(coremark_200m C CXX ASM)
set(CMAKE_C_STANDARD 11)

# Most aggressive speed optimization flags
add_compile_options(-Ofast -funroll-loops)

pico_sdk_init()

# Add include paths so coremark sources find core_portme.h, coremark.h and utils.h
include_directories(coremark_1_0_1/rp2040)
include_directories(coremark_1_0_1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(coremark_200m
        main.c
        utils.c
        coremark_1_0_1/core_list_join.c
        coremark_1_0_1/core_main.c
        coremark_1_0_1/core_matrix.c
        coremark_1_0_1/core_state.c
        coremark_1_0_1/core_util.c
        coremark_1_0_1/rp2040/core_portme.c
        )

target_link_libraries(coremark_200m pico_stdlib hardware_vreg)

# Generate .uf2 using the picotool pre-built with mini_blink.
set(PICOTOOL_EXE "D:/rp2040_vs_demo/mini_blink/build/_deps/picotool/picotool.exe")
if(EXISTS "${PICOTOOL_EXE}")
    add_custom_command(TARGET coremark_200m  POST_BUILD
        COMMAND "${PICOTOOL_EXE}" uf2 convert coremark_200m.elf coremark_200m.uf2
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating coremark_200m.uf2"
        BYPRODUCTS coremark_200m.uf2
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
        -c "init; reset halt; program ${CMAKE_CURRENT_BINARY_DIR}/coremark_200m.elf verify reset exit"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Flashing coremark_200m.elf to RP2040 via OpenOCD …"
    USES_TERMINAL
)

# -------------------------------------------------------------------------
# Custom target: clean-all — deep clean compiled objects and SDK files
#   Removes all .o / .elf / .uf2 files and pico-sdk copies, but preserves
#   CMakeCache.txt + build.ninja so you can rebuild immediately with ninja
#   without re-supplying -D flags.
# -------------------------------------------------------------------------
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/coremark_200m.dir"
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
        "${CMAKE_CURRENT_BINARY_DIR}/coremark_200m.elf"
        "${CMAKE_CURRENT_BINARY_DIR}/coremark_200m.uf2"
    COMMAND ${CMAKE_COMMAND} -E echo "Deep clean done. Run 'ninja' to rebuild with cached configuration."
    COMMENT "Removing all build artifacts (keeps CMakeCache.txt) …"
    USES_TERMINAL
)
