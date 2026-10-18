#!/bin/sh
set -eu
: "${ELKS:?Set ELKS to your configured ELKS source/toolchain tree}"
CROSS=${CROSS:-$ELKS/cross}
BUILD=${BUILD:-build-elks}
mkdir -p "$BUILD/host-tools" "$BUILD/obj"
cp toolchain/retain-elf2elks.py "$BUILD/host-tools/elf2elks"
chmod +x "$BUILD/host-tools/elf2elks"
ELX_REAL_ELF2ELKS=$ELKS/elks/tools/bin/elf2elks
PATH=$(pwd)/$BUILD/host-tools:$CROSS/bin:$ELKS/elks/tools/bin:$PATH
export PATH ELX_REAL_ELF2ELKS
CC=$CROSS/bin/ia16-elf-gcc
flags="-Os -melks-libc -march=i8086 -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fstack-usage"
inc="-Isrc -I$ELKS/include -I$ELKS/libc/include -I$ELKS/elks/include"
for name in elxstat elxbat gauges widgets cga console status battery; do
    "$CC" $flags $inc -c "src/$name.c" -o "$BUILD/obj/$name.o"
done
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/elxstat.map" -maout-heap=2048 -maout-stack=2048 "$BUILD/obj/elxstat.o" "$BUILD/obj/status.o" "$BUILD/obj/battery.o" -o "$BUILD/elxstat"
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/elxbat.map" -maout-heap=2048 -maout-stack=2048 \
    "$BUILD/obj/elxbat.o" "$BUILD/obj/gauges.o" "$BUILD/obj/widgets.o" "$BUILD/obj/cga.o" "$BUILD/obj/console.o" "$BUILD/obj/status.o" "$BUILD/obj/battery.o" -o "$BUILD/elxbat"
for name in elxstat elxbat; do
    "$CROSS/bin/ia16-elf-size" "$BUILD/$name.elf"
    "$CROSS/bin/ia16-elf-objdump" -d -z --insn-width=16 -M intel,i8086 "$BUILD/$name.elf" > "$BUILD/$name.dis"
    python3 toolchain/isa-audit.py "$BUILD/$name" "$BUILD/$name.elf" "$BUILD/$name.dis" --map "$BUILD/$name.map" --output "$BUILD/$name-audit.json"
done
"$CC" --version > "$BUILD/compiler-version.txt"
