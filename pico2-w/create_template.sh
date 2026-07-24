#!/usr/bin/env bash
# --------------------------------------------------------------------------
# create_pico2w_template.sh — Create a new RP2350 (Pico 2 W) project from a
#                              template.
#
# The template sources from pico/hello_serial/ (dual-UART: UART0 on GP0/GP1,
# UART1 on GP4/GP5) but targets the **RP2350** chip (Pico 2 W).  The
# created project runs at **150 MHz** (max spec).
#
# Usage:
#   ./create_template.sh
#
# The generated project always inits both UARTs:
#   - UART0 (GP0 TX, GP1 RX) — intended for device comms (ESP8285, etc.)
#   - UART1 (GP4 TX, GP5 RX) — debug output via PRINTF(...)
#
# Windows (Git Bash / WSL):
#   bash create_template.sh
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
SRC_DIR="../pico/hello_serial"
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

# ----- Inject RP2350 platform & Pico 2 W board settings -----------------
python3 -c "
import sys
path = sys.argv[1]
with open(path) as f:
    content = f.read()
content = content.replace('include(pico_sdk_import.cmake)', '''# 1. Target platform & board — must be set BEFORE project() and
#    pico_sdk_import.cmake so the SDK selects the right toolchain.
set(PICO_PLATFORM rp2350)
set(PICO_BOARD   pico2_w)

# 2. Include the SDK import helper (must come before the project() call).
include(pico_sdk_import.cmake)''', 1)
with open(path, 'w') as f:
    f.write(content)
" "${DST_DIR}/CMakeLists.txt"

# ----- Update chip references from RP2040 to RP235x ---------------------
sed_inplace "s/--chip RP2040/--chip RP235x/g"                               "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/target\/rp2040\.cfg/target\/rp2350.cfg/g"                     "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/RP2040/RP2350/g"                                              "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/Flashing hello_serial/Flashing ${PROJECT_NAME}/g"             "${DST_DIR}/CMakeLists.txt"

echo ""
echo "Done — template created at: ${DST_DIR}"
echo "  Target chip: RP2350 (Pico 2 W)"
echo "  Dual UART: UART0 (GP0/GP1) + UART1 (GP4/GP5)"
echo "  System clock: 150 MHz (RP2350 max spec)"
echo "  PRINTF(...) outputs to UART1 (GP4 TX)"
echo ""
echo "Next steps:"
echo ""
echo "  cd ${DST_DIR}"
echo "  mkdir build && cd build"
echo ""
echo "  # macOS / Linux"
echo "  PICO_SDK_PATH=/path/to/pico-sdk cmake -G Ninja .."
echo "  #   Optional (avoids source builds):"
echo '  #     export picotool_DIR="$HOME/tool/picotool"'
echo '  #     export pioasm_DIR="$HOME/tool/pioasm/pioasm"'
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