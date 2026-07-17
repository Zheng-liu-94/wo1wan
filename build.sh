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

mkdir -p build out

echo "=== 编译 main.c ==="
${CC} ${CFLAGS} -c source/main.c -o build/main.o

echo "=== 链接 ==="
${CC} ${LDFLAGS} build/main.o ${LIBS} -o build/wo1wan.elf

echo "=== 生成 NACP ==="
echo "NACPTOOL=${NACPTOOL}"
# Portable invocation: positional args (title author version output) work on both
# old and new nacptool. The --title/--author/--version long-flag form is not honored
# by every devkitPro build, so we use the positional form and fall back to copying
# the generated .nacp if it landed somewhere unexpected.
${NACPTOOL} --create "wo1wan" "wiliwili-fork" "1.0.0" build/wo1wan.nacp 2>&1
if [ ! -f build/wo1wan.nacp ]; then
  echo "positional form produced no file; searching for any *.nacp ..."
  f=$(find . -maxdepth 2 -name '*.nacp' 2>/dev/null | head -1)
  if [ -n "$f" ]; then
    echo "found $f -> copying to build/wo1wan.nacp"
    cp "$f" build/wo1wan.nacp
  fi
fi
echo "nacptool rc=$?"
ls -la build/ 2>&1 || true

echo "=== 生成 .nro ==="
${ELF2NRO} build/wo1wan.elf out/wo1wan.nro \
  --icon=icon.jpg --nacp=build/wo1wan.nacp

echo "=== 完成 ==="
ls -lh out/wo1wan.nro
