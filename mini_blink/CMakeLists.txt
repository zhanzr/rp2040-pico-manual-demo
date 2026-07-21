#=============================================================================
# Minimal RP2040 toolchain / development verification template
#
# Use this project to confirm that the CMake + ARM GCC toolchain is set up
# correctly and the Pico SDK is integrated properly.
#
# Building (from the build/ directory):
#   cmake ..
#   cmake --build .
#
# Output: build/mini_blink.uf2  — drag onto a Pico in BOOTSEL mode.
#=============================================================================

cmake_minimum_required(VERSION 3.13)

# 1. Include the SDK import helper (must come before the project() call).
include(pico_sdk_import.cmake)

# 2. Declare the project — languages must be listed AFTER the import hook.
project(mini_blink C CXX ASM)
set(CMAKE_C_STANDARD 11)

# 3. Initialise SDK definitions & platform libraries.
pico_sdk_init()

# 4. Define the executable and link the core SDK library.
add_executable(mini_blink main.c)
target_link_libraries(mini_blink pico_stdlib)

# 5. Generate extra output formats (.uf2, .hex, .elf, .dis).
pico_add_extra_outputs(mini_blink)
