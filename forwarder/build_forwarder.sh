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

# IMPORTANT (GCC 15 / new binutils): the default is now `-z text`, which makes
# ld reject dynamic relocations living in a read-only segment with:
#   "read-only segment has dynamic relocations"
# The forwarder (hbloader + trampoline) legitimately has such relocations, and
# the Horizon NSO loader applies them at load time. The proven fix (see the
# kvm-unit-tests arm64 patch) is `-pie` + `-Wl,-z,notext`. switch.specs already
# contributes `-pie`; we add `-z notext` to relax the read-only check.
"$CC" build/main.o build/trampoline.o \
    -specs="$LIBNX/switch.specs" -g -L"$LIBNX/lib" -lnx \
    -o build/wo1wan-forwarder.elf -pie -Wl,-z,notext

echo "== verify ELF type =="
"$DEVKITPRO/devkitA64/bin/aarch64-none-elf-readelf" -h build/wo1wan-forwarder.elf \
    | grep -E "Type:|Machine:" || true

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
echo "== verify PFS0 magic =="
head -c 4 wo1wan.nsp | od -c | head -1 || true
echo "FORWARDER_BUILD_OK"
