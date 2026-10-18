#!/bin/sh
set -eu
: "${WATCOM:?Set WATCOM to Open Watcom root}"
BUILD=${BUILD:-build-dos}
PATH=$WATCOM/binl64:$PATH
INCLUDE=$WATCOM/h
export PATH INCLUDE
mkdir -p "$BUILD"
# -0 targets 8086; small model, DOS real-mode EXE, no extender.
wcl -q -bt=dos -0 -ms -os -s -wx -i=src -i=../elx-utils/src -i=../elx-ctl/src \
 -fo="$BUILD/" -fe="$BUILD/LXDASH.EXE" -fm="$BUILD/LXDASH.MAP" -k4096 \
 src/main.c src/model.c src/demo.c src/draw.c src/platform_dos.c \
 ../elx-ctl/src/commands.c ../elx-utils/src/battery.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c
wdis -a "$BUILD/platform_dos.o" > "$BUILD/platform_dos.dis"
