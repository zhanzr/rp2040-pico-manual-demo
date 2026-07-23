#!/usr/bin/env bash
# --------------------------------------------------------------------------
# create_pico_template.sh — Create a new RP2040 project from a template.
#
# The template sources from pico/hello_serial/ (dual-UART: UART0 on GP0/GP1,
# UART1 on GP4/GP5).  The created project always defaults to **200 MHz**
# (overclocked).  To switch to 125 MHz, edit the .c file: comment the
# OVERCLOCK block and uncomment the NORMAL line.
#
# Usage:
#   ./create_pico_template.sh
#
# The generated project always inits both UARTs:
#   - UART0 (GP0 TX, GP1 RX) — intended for device comms (ESP8285, etc.)
#   - UART1 (GP4 TX, GP5 RX) — debug output via PRINTF(...)
#
# Windows (Git Bash / WSL):
#   bash create_pico_template.sh
# --------------------------------------------------------------------------
set -euo pipefail

# ----- Cross-platform sed in-place  ------------------------------------
sed_inplace() {
    if [[ "$(uname)" == "Darwin" ]]; then
        sed -i '' "$@"
    else
        sed -i "$@"
    fi
}

# ----- Source template ---------------------------------------------------
SRC_DIR="hello_serial"
SRC_FILE="hello_serial.c"
PREFIX="template"

# ----- Locate the script's own directory --------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_PATH="${SCRIPT_DIR}/${SRC_DIR}"

# ----- Find the next unused number --------------------------------------
NEXT=1
while [[ -d "${SCRIPT_DIR}/${PREFIX}_${NEXT}" ]]; do
    ((NEXT++))
done

DST_DIR="${SCRIPT_DIR}/${PREFIX}_${NEXT}"
PROJECT_NAME="${PREFIX}_${NEXT}"

echo "Creating ${DST_DIR} …"
echo "  UART0 on GP0/GP1 + UART1 on GP4/GP5 (dual UART)"
mkdir -p "${DST_DIR}"

# ----- Copy files (the .c source is always named main.c) ----------------
cp "${SRC_PATH}/CMakeLists.txt"          "${DST_DIR}/"
cp "${SRC_PATH}/${SRC_FILE}"             "${DST_DIR}/main.c"
cp "${SRC_PATH}/pico_sdk_import.cmake"   "${DST_DIR}/"

if [[ -f "${SRC_PATH}/README.md" ]]; then
    cp "${SRC_PATH}/README.md" "${DST_DIR}/"
fi

# ----- Update project/source names in CMakeLists.txt --------------------
# Note: \b (word boundary) is a GNU sed extension — not available on
# macOS BSD sed.  Patterns avoid \b and rely on unique context instead.
sed_inplace "s/project(hello_serial /project(${PROJECT_NAME} /"               "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/add_executable(hello_serial$/add_executable(${PROJECT_NAME}/"  "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/hello_serial\\.c/main.c/g"                                     "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/target_link_libraries(hello_serial /target_link_libraries(${PROJECT_NAME} /" "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/pico_enable_stdio_uart(hello_serial /pico_enable_stdio_uart(${PROJECT_NAME} /" "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/pico_enable_stdio_usb(hello_serial /pico_enable_stdio_usb(${PROJECT_NAME} /" "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/DEPENDS hello_serial$/DEPENDS ${PROJECT_NAME}/"                "${DST_DIR}/CMakeLists.txt"
sed_inplace "s|CMakeFiles/hello_serial\\.dir|CMakeFiles/${PROJECT_NAME}.dir|g" "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/hello_serial\\.elf/${PROJECT_NAME}.elf/g"                      "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/hello_serial\\.uf2/${PROJECT_NAME}.uf2/g"                      "${DST_DIR}/CMakeLists.txt"

echo ""
echo "Done — template created at: ${DST_DIR}"
echo "  Dual UART: UART0 (GP0/GP1) + UART1 (GP4/GP5)"
echo "  System clock: 200 MHz (edit .c to switch to 125 MHz)"
echo "  PRINTF(...) outputs to UART1 (GP4 TX)"
echo ""
echo "Next steps:"
echo ""
echo "  cd ${DST_DIR}"
echo "  mkdir build && cd build"
echo ""
echo "  # macOS / Linux"
echo "  PICO_SDK_PATH=/path/to/pico-sdk cmake -G Ninja .."
echo "  #   Optional (avoids picotool source build):"
echo '  #     export picotool_DIR="$HOME/tool/picotool"'
echo "  ninja                    # build .elf only (fast)"
echo "  ninja uf2               # generate .uf2 (BOOTSEL-mode flash)"
echo "  ninja flash             # flash via probe-rs (default)"
echo "  ninja flash-ocd         # flash via OpenOCD (fallback)"
echo ""
echo "  # Windows (MSYS2 MinGW64)"
echo '  cmake -G Ninja -DPICO_SDK_PATH="D:/pico-sdk" ^'
echo '        -Dpicotool_DIR="D:/picotool" ^'
echo '        -DCMAKE_TOOLCHAIN_FILE="D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake" ^'
echo '        -DPICO_TOOLCHAIN_PATH="D:/arm-gnu-toolchain/bin" ^'
echo '        -DCMAKE_MAKE_PROGRAM="C:/msys64/mingw64/bin/ninja.exe" ^'
echo '        -DPython3_EXECUTABLE="C:/msys64/mingw64/bin/python3.exe" ..'
echo "  ninja"
echo ""
echo "  # Hardware flash (any OS)"
echo "  ninja flash              # probe-rs (default)"
echo "  ninja flash-ocd          # OpenOCD  (fallback)"
echo ""
echo "  # Renaming this folder later?"
echo "  #   sed -i '' 's/${PROJECT_NAME}/your_new_name/g' CMakeLists.txt"
