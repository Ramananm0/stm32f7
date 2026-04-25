#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${ROOT_DIR}/stm32f7disco"
BUILD_DIR="${PROJECT_DIR}/Debug"
CUBE_GCC_DIR="/opt/st/stm32cubeide_2.1.1/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.linux64_1.0.100.202602081740/tools/bin"

if [ ! -d "${BUILD_DIR}" ]; then
    echo "Missing ${BUILD_DIR}. Copy or generate the STM32CubeIDE Debug build directory first." >&2
    exit 1
fi

if [ ! -f "${PROJECT_DIR}/Middlewares/micro_ros_stm32cubemx_utils/microros_static_library_ide/libmicroros/libmicroros.a" ]; then
    echo "Missing micro-ROS static library under ${PROJECT_DIR}/Middlewares." >&2
    exit 1
fi

export PATH="${CUBE_GCC_DIR}:${PATH}"

make -C "${BUILD_DIR}" all -j"$(nproc)"
arm-none-eabi-objcopy -O binary "${BUILD_DIR}/stm32f7disco.elf" "${BUILD_DIR}/stm32f7disco.bin"
