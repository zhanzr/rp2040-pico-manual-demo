cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_serial C CXX ASM)
set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(hello_serial
        hello_serial.c
        )

target_link_libraries(hello_serial pico_stdlib)

# Generate .uf2 using the picotool pre-built with mini_blink.
# If picotool is not available, the build still succeeds — just no .uf2.
set(PICOTOOL_EXE "D:/rp2040_vs_demo/mini_blink/build/_deps/picotool/picotool.exe")
if(EXISTS "${PICOTOOL_EXE}")
    add_custom_command(TARGET hello_serial POST_BUILD
        COMMAND "${PICOTOOL_EXE}" uf2 convert hello_serial.elf hello_serial.uf2
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating hello_serial.uf2"
        BYPRODUCTS hello_serial.uf2
    )
else()
    message(STATUS "picotool not found — skip .uf2 generation; use mini_blink picotool manually")
endif()
