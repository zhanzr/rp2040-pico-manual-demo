cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_serial_200mhz C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(hello_serial_200mhz
        hello_serial_200mhz.c
        )

target_link_libraries(hello_serial_200mhz pico_stdlib hardware_vreg)

# Generate .uf2 using the picotool pre-built with mini_blink.
set(PICOTOOL_EXE "D:/rp2040_vs_demo/mini_blink/build/_deps/picotool/picotool.exe")
if(EXISTS "${PICOTOOL_EXE}")
    add_custom_command(TARGET hello_serial_200mhz POST_BUILD
        COMMAND "${PICOTOOL_EXE}" uf2 convert hello_serial_200mhz.elf hello_serial_200mhz.uf2
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating hello_serial_200mhz.uf2"
        BYPRODUCTS hello_serial_200mhz.uf2
    )
endif()
