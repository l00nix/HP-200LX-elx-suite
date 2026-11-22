#!/bin/sh
set -eu
: "${ELKS:?Set ELKS to configured source/toolchain}"
CROSS=${CROSS:-$ELKS/cross}
BUILD=${BUILD:-build-elks}
mkdir -p "$BUILD/host-tools" "$BUILD/obj"
cp ../elx-ctl/toolchain/retain-elf2elks.py "$BUILD/host-tools/elf2elks"
chmod +x "$BUILD/host-tools/elf2elks"
ELX_REAL_ELF2ELKS=$ELKS/elks/tools/bin/elf2elks
PATH=$(pwd)/$BUILD/host-tools:$CROSS/bin:$ELKS/elks/tools/bin:$PATH
export PATH ELX_REAL_ELF2ELKS
CC=$CROSS/bin/ia16-elf-gcc
flags="-Os -melks-libc -march=i8086 -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror -ffunction-sections -fdata-sections -fstack-usage"
inc="-Isrc -I../elx-utils/src -I../elx-ctl/src -I$ELKS/include -I$ELKS/libc/include -I$ELKS/elks/include"
objects=""
for source in src/main.c src/model.c src/menu.c src/charge.c src/charge_elks.c src/demo.c src/draw.c src/platform_elks.c ../elx-ctl/src/commands.c ../elx-ctl/src/hardware.c ../elx-utils/src/status.c ../elx-utils/src/battery.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c ../elx-utils/src/console.c; do
 name=$(basename "$source" .c)
 "$CC" $flags $inc -c "$source" -o "$BUILD/obj/$name.o"
 objects="$objects $BUILD/obj/$name.o"
done
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/elxdash.map" -maout-heap=2048 -maout-stack=4096 $objects -o "$BUILD/elxdash"
"$CROSS/bin/ia16-elf-size" "$BUILD/elxdash.elf"
"$CROSS/bin/ia16-elf-objdump" -d -z --insn-width=16 -M intel,i8086 "$BUILD/elxdash.elf" > "$BUILD/elxdash.dis"
python3 ../elx-ctl/toolchain/isa-audit.py "$BUILD/elxdash" "$BUILD/elxdash.elf" "$BUILD/elxdash.dis" --map "$BUILD/elxdash.map" --output "$BUILD/elxdash-audit.json"
"$CC" --version > "$BUILD/compiler-version.txt"
