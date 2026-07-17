#!/bin/bash
# wo1wan 构建脚本 —— 在 devkitpro/devkita64 容器中运行
# 用法: bash build.sh
set -euo pipefail

DEVKITPRO="/opt/devkitpro"
PREFIX="${DEVKITPRO}/devkitA64/bin/aarch64-none-elf-"
CC="${PREFIX}gcc"
ELF2NRO="${DEVKITPRO}/tools/bin/elf2nro"
NACPTOOL="${DEVKITPRO}/tools/bin/nacptool"

ARCH="-march=armv8-a+crypto+crc -mtune=cortex-a57 -mtp=soft -fPIE"
CFLAGS="-g -Wall -O2 ${ARCH} -I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/libnx/include/switch"
LDFLAGS="-specs=${DEVKITPRO}/libnx/switch.specs -g ${ARCH}"
LIBS="-L${DEVKITPRO}/libnx/lib -lnx"

echo "=== DEBUG: locate libnx headers ==="
echo "DEVKITPRO=${DEVKITPRO}"
echo "--- ls libnx/include ---"
ls -la ${DEVKITPRO}/libnx/include 2>/dev/null | head -50
echo "--- find hid.h ---"
find ${DEVKITPRO}/libnx -name 'hid.h' 2>/dev/null
echo "--- find keycodes.h ---"
find ${DEVKITPRO}/libnx -name 'keycodes.h' 2>/dev/null
echo "--- find web.h ---"
find ${DEVKITPRO}/libnx -name 'web.h' 2>/dev/null
echo "=== END DEBUG ==="

mkdir -p build out

echo "=== 编译 main.c ==="
${CC} ${CFLAGS} -c source/main.c -o build/main.o

echo "=== 链接 ==="
${CC} ${LDFLAGS} build/main.o ${LIBS} -o build/wo1wan.elf

echo "=== 生成 NACP ==="
${NACPTOOL} --create --language=en \
  --title="wo1wan" --author="wiliwili-fork" \
  --version="1.0.0" build/wo1wan.nacp

echo "=== 生成 .nro ==="
${ELF2NRO} build/wo1wan.elf out/wo1wan.nro \
  --icon=icon.jpg --nacp=build/wo1wan.nacp

echo "=== 完成 ==="
ls -lh out/wo1wan.nro
