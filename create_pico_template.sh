#!/usr/bin/env bash
# --------------------------------------------------------------------------
# create_pico_template.sh — Create a new RP2040 project from a template.
#
# Usage:
#   ./create_pico_template.sh          # normal  (125 MHz,  hello_serial)
#   ./create_pico_template.sh 200mhz   # overclocked (200 MHz, hello_serial_200mhz)
#
# What it does:
#   1. Scans for existing template_N / template_200mhz_N folders.
#   2. Picks the next unused number.
#   3. Creates the folder and copies CMakeLists.txt, the .c source,
#      and (if present) README.md from the matching template project.
#   4. Renames all project references in the copied CMakeLists.txt.
# --------------------------------------------------------------------------
set -euo pipefail

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
sed -i "s/project(${SRC_BASE}\b/project(${PROJECT_NAME}/"                 "${DST_DIR}/CMakeLists.txt"
sed -i "s/add_executable(${SRC_BASE}\b/add_executable(${PROJECT_NAME}/"  "${DST_DIR}/CMakeLists.txt"
sed -i "s/${SRC_BASE}\\.c/${PROJECT_NAME}.c/g"                           "${DST_DIR}/CMakeLists.txt"
sed -i "s/target_link_libraries(${SRC_BASE}\b/target_link_libraries(${PROJECT_NAME}/" "${DST_DIR}/CMakeLists.txt"
sed -i "s/${SRC_BASE}\\.elf/${PROJECT_NAME}.elf/g"                       "${DST_DIR}/CMakeLists.txt"
sed -i "s/${SRC_BASE}\\.uf2/${PROJECT_NAME}.uf2/g"                       "${DST_DIR}/CMakeLists.txt"
sed -i "s/TARGET ${SRC_BASE}\b/TARGET ${PROJECT_NAME} /"                 "${DST_DIR}/CMakeLists.txt"

# ----- Append flash & clean-all custom targets to CMakeLists.txt --------
cat >> "${DST_DIR}/CMakeLists.txt" << 'CUSTOM_EOF'

# -------------------------------------------------------------------------
# Custom target: flash — program the Pico via OpenOCD + CMSIS-DAP
# -------------------------------------------------------------------------
add_custom_target(flash
    COMMAND openocd
        -f interface/cmsis-dap.cfg
        -c "adapter speed 10000"
        -f target/rp2040.cfg
        -c "cmsis_dap_backend usb_bulk"
        -c "init; reset halt; program ${CMAKE_CURRENT_BINARY_DIR}/__PROJECT__.elf verify reset exit"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Flashing __PROJECT__.elf to RP2040 via OpenOCD …"
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
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/__PROJECT__.dir"
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
        "${CMAKE_CURRENT_BINARY_DIR}/__PROJECT__.elf"
        "${CMAKE_CURRENT_BINARY_DIR}/__PROJECT__.uf2"
    COMMAND ${CMAKE_COMMAND} -E echo "Deep clean done. Run 'ninja' to rebuild with cached configuration."
    COMMENT "Removing all build artifacts (keeps CMakeCache.txt) …"
    USES_TERMINAL
)
CUSTOM_EOF

sed -i "s/__PROJECT__/${PROJECT_NAME}/g" "${DST_DIR}/CMakeLists.txt"

echo "Done — template created at: ${DST_DIR}"
echo ""
echo "Next steps:"
echo "  cd ${DST_DIR}"
echo "  mkdir build && cd build"
echo "  cmake -G Ninja -DPICO_BOARD=pico -DPICO_SDK_PATH=\"D:/pico-sdk\" \\"
echo "        -DCMAKE_TOOLCHAIN_FILE=\"D:/pico-sdk/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake\" \\"
echo "        -DPICO_TOOLCHAIN_PATH=\"D:/arm-gnu-toolchain/bin\" \\"
echo "        -DCMAKE_MAKE_PROGRAM=\"C:/msys64/mingw64/bin/ninja.exe\" \\"
echo "        -DPython3_EXECUTABLE=\"C:/msys64/mingw64/bin/python3.exe\" .."
echo "  ninja"
