#!/bin/sh
set -eu
: "${ELKS:?}"
CROSS=${CROSS:-$ELKS/cross}
PATH=$CROSS/bin:$ELKS/elks/tools/bin:$PATH
export PATH
EMU=${EMU:-$ELKS/elksemu/elksemu}
CC=$CROSS/bin/ia16-elf-gcc
flags="-Os -melks-libc -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror"
inc="-Isrc -I$ELKS/include -I$ELKS/libc/include -I$ELKS/elks/include"
mkdir -p build-tests
"$CC" $flags $inc -maout-heap=2048 -maout-stack=2048 tests/test_math.c src/status.c src/battery.c -o build-tests/math
"$EMU" build-tests/math
"$CC" $flags $inc -DELX_CGA_TEST -maout-heap=2048 -maout-stack=2048 \
    src/elxbat.c src/gauges.c src/widgets.c src/cga.c src/status.c src/battery.c -o build-tests/elxbat-render
python3 tests/test_target.py "$EMU"
"$CC" $flags $inc -maout-heap=2048 -maout-stack=2048 src/elxbat.c tests/test_power_loop.c -o build-tests/power-loop
"$EMU" build-tests/power-loop
