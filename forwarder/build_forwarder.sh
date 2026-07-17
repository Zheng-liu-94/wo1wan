#!/bin/bash
#
# Build the wo1wan Application forwarder (.nsp).
# This produces a Switch APPLICATION that, when launched from the home menu,
# loads sdmc:/switch/wo1wan.nro as AppletType_SystemApplication -- the only
# context allowed to launch the Web Applet.
#
set -ex

export DEVKITPRO=/opt/devkitpro
LIBNX=$DEVKITPRO/libnx
TOOLSBIN=$DEVKITPRO/tools/bin
CC=$DEVKITPRO/devkitA64/bin/aarch64-none-elf-gcc

# Locate tools (devkitA64 container layout)
ELF2NSO=$(command -v elf2nso 2>/dev/null || echo "$TOOLSBIN/elf2nso")
NPDMTOOL=$(command -v npdmtool 2>/dev/null || echo "$TOOLSBIN/npdmtool")
NACPTOOL=$(command -v nacptool 2>/dev/null || echo "$TOOLSBIN/nacptool")

echo "== tool paths =="
echo "CC       = $CC"
echo "ELF2NSO  = $ELF2NSO"
echo "NPDMTOOL  = $NPDMTOOL"
echo "NACPTOOL  = $NACPTOOL"
ls -la "$TOOLSBIN" | head -40 || true

cd "$(dirname "$0")"
mkdir -p build exefs control

# ---------------------------------------------------------------------------
# 1. Compile forwarder (mirrors wiliwili switch-forwarder Makefile flags)
# ---------------------------------------------------------------------------
echo "== compiling forwarder =="
"$CC" -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE -g -O2 -Wall \
    -I"$LIBNX/include" -D__SWITCH__ -DVERSION=\"v1.0.0\" \
    -c source/main.c -o build/main.o

"$CC" -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE -g -O2 -Wall \
    -I"$LIBNX/include" -D__SWITCH__ \
    -c source/trampoline.s -o build/trampoline.o

"$CC" build/main.o build/trampoline.o \
    -specs="$LIBNX/switch.specs" -g -L"$LIBNX/lib" -lnx \
    -o build/wo1wan-forwarder.elf -pie

# ---------------------------------------------------------------------------
# 2. NSO + NPDM + NACP
# ---------------------------------------------------------------------------
echo "== packaging exefs/control =="
"$ELF2NSO" build/wo1wan-forwarder.elf exefs/main
"$NPDMTOOL" wo1wan.json exefs/main.npdm
"$NACPTOOL" --create "wo1wan" "wo1wan" "1.0.0" control/control.nacp
cp icon.jpg control/icon_AmericanEnglish.dat

# ---------------------------------------------------------------------------
# 3. Build ExeFS PFS0 (.nsp) -- no Nintendo signature required by Atmosphere
# ---------------------------------------------------------------------------
echo "== building wo1wan.nsp (PFS0) =="
python3 make_nsp.py wo1wan.nsp

ls -lh wo1wan.nsp
echo "FORWARDER_BUILD_OK"
