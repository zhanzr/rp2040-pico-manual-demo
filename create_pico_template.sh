#!/usr/bin/env bash
# --------------------------------------------------------------------------
# create_pico_template.sh — Create a new RP2040 project from a template.
#
# Usage (macOS / Linux):
#   ./create_pico_template.sh          # normal  (125 MHz,  hello_serial)
#   ./create_pico_template.sh 200mhz   # overclocked (200 MHz, hello_serial_200mhz)
#
# Usage (Windows — Git Bash / WSL):
#   bash create_pico_template.sh       # normal
#   bash create_pico_template.sh 200mhz
#
# What it does:
#   1. Scans for existing template_N / template_200mhz_N folders.
#   2. Picks the next unused number.
#   3. Creates the folder and copies CMakeLists.txt, the .c source,
#      pico_sdk_import.cmake, and (if present) README.md from the
#      matching template project.
#   4. Renames all project references in the copied CMakeLists.txt.
# --------------------------------------------------------------------------
set -euo pipefail

# ----- Cross-platform sed in-place  ------------------------------------
# BSD sed (macOS) requires -i '' ; GNU sed (Linux, Git Bash) uses -i only.
sed_inplace() {
    if [[ "$(uname)" == "Darwin" ]]; then
        sed -i '' "$@"
    else
        sed -i "$@"
    fi
}

MODE="${1:-normal}"

case "${MODE}" in
    normal|125mhz)
        SRC_DIR="hello_serial"
        PREFIX="template"
        SRC_FILE="hello_serial.c"
        ;;
    200mhz|overclock)
        SRC_DIR="hello_serial_200mhz"
        PREFIX="template_200mhz"
        SRC_FILE="hello_serial_200mhz.c"
        ;;
    *)
        echo "Usage: $0 [normal|200mhz]"
        exit 1
        ;;
esac

# ----- Locate the script's own directory --------------------------------
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# ----- Find the next unused number --------------------------------------
NEXT=1
while [[ -d "${SCRIPT_DIR}/${PREFIX}_${NEXT}" ]]; do
    ((NEXT++))
done

DST_DIR="${SCRIPT_DIR}/${PREFIX}_${NEXT}"
echo "Creating ${DST_DIR} …"
mkdir -p "${DST_DIR}"

# ----- Copy files (rename .c to match the new project) ------------------
PROJECT_NAME="${PREFIX}_${NEXT}"
SRC_BASE="${SRC_FILE%.c}"

cp "${SCRIPT_DIR}/${SRC_DIR}/CMakeLists.txt"       "${DST_DIR}/"
cp "${SCRIPT_DIR}/${SRC_DIR}/${SRC_FILE}"          "${DST_DIR}/${PROJECT_NAME}.c"
cp "${SCRIPT_DIR}/${SRC_DIR}/pico_sdk_import.cmake" "${DST_DIR}/"

if [[ -f "${SCRIPT_DIR}/${SRC_DIR}/README.md" ]]; then
    cp "${SCRIPT_DIR}/${SRC_DIR}/README.md" "${DST_DIR}/"
fi

# ----- Update project/source names in CMakeLists.txt --------------------
# The source CMakeLists.txt already contains flash, clean-all targets and
# pico_add_extra_outputs — all are renamed below.

sed_inplace "s/project(${SRC_BASE}\b/project(${PROJECT_NAME}/"                    "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/add_executable(${SRC_BASE}\b/add_executable(${PROJECT_NAME}/"      "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/${SRC_BASE}\\.c/${PROJECT_NAME}.c/g"                               "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/target_link_libraries(${SRC_BASE}\b/target_link_libraries(${PROJECT_NAME}/" "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/DEPENDS ${SRC_BASE}\b/DEPENDS ${PROJECT_NAME}/"                    "${DST_DIR}/CMakeLists.txt"
sed_inplace "s|CMakeFiles/${SRC_BASE}\.dir|CMakeFiles/${PROJECT_NAME}.dir|g"      "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/${SRC_BASE}\\.elf/${PROJECT_NAME}.elf/g"                           "${DST_DIR}/CMakeLists.txt"
sed_inplace "s/${SRC_BASE}\\.uf2/${PROJECT_NAME}.uf2/g"                           "${DST_DIR}/CMakeLists.txt"

echo "Done — template created at: ${DST_DIR}"
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
echo "  ninja flash             # flash via OpenOCD + SWD"
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
echo "  # Hardware flash via OpenOCD (any OS)"
echo "  ninja flash"
