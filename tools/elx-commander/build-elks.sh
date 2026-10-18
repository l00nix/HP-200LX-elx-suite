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
flags="-Os -melks-libc -march=i8086 -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror -Wno-misleading-indentation -ffunction-sections -fdata-sections -fstack-usage"
inc="-Isrc -I$ELKS/include -I$ELKS/libc/include -I$ELKS/elks/include"
for name in main files terminal; do
    "$CC" $flags $inc -c "src/$name.c" -o "$BUILD/obj/$name.o"
done
"$CC" $flags -Wl,--gc-sections,-Map,"$BUILD/elxcmd.map" -maout-heap=8192 -maout-stack=4096 \
    "$BUILD/obj/main.o" "$BUILD/obj/files.o" "$BUILD/obj/terminal.o" -o "$BUILD/elxcmd"
"$CROSS/bin/ia16-elf-size" "$BUILD/elxcmd.elf"
"$CROSS/bin/ia16-elf-objdump" -d -z --insn-width=16 -M intel,i8086 "$BUILD/elxcmd.elf" > "$BUILD/elxcmd.dis"
python3 toolchain/isa-audit.py "$BUILD/elxcmd" "$BUILD/elxcmd.elf" "$BUILD/elxcmd.dis" --map "$BUILD/elxcmd.map" --output "$BUILD/elxcmd-audit.json"
"$CC" --version > "$BUILD/compiler-version.txt"
"$CC" $flags $inc -Wl,--gc-sections -maout-heap=8192 -maout-stack=4096 \
    tests/ops.c src/files.c src/terminal.c -o "$BUILD/test-ops"
