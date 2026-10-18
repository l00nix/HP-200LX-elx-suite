#!/bin/sh
set -eu
: "${ELKS:?Set ELKS to configured source/toolchain}"
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
for name in main commands hardware status; do
    "$CC" $flags $inc -c "src/$name.c" -o "$BUILD/obj/$name.o"
done
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/elxctl.map" -maout-heap=2048 -maout-stack=2048 \
    "$BUILD/obj/main.o" "$BUILD/obj/commands.o" "$BUILD/obj/hardware.o" "$BUILD/obj/status.o" -o "$BUILD/elxctl"
"$CROSS/bin/ia16-elf-size" "$BUILD/elxctl.elf"
"$CROSS/bin/ia16-elf-objdump" -d -z --insn-width=16 -M intel,i8086 "$BUILD/elxctl.elf" > "$BUILD/elxctl.dis"
python3 toolchain/isa-audit.py "$BUILD/elxctl" "$BUILD/elxctl.elf" "$BUILD/elxctl.dis" --map "$BUILD/elxctl.map" --output "$BUILD/elxctl-audit.json"
"$CC" --version > "$BUILD/compiler-version.txt"

"$CC" $flags $inc tests/commands.c src/commands.c -o "$BUILD/test-commands"
