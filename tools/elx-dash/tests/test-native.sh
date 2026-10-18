#!/bin/sh
set -eu
: "${ELKS:?}" "${EMU:?Set EMU to the isolated LP64-compatible elksemu}"
CC=$ELKS/cross/bin/ia16-elf-gcc
PATH=$ELKS/cross/bin:$ELKS/elks/tools/bin:$PATH
export PATH
flags="-Os -melks-libc -march=i8086 -mtune=i8086 -mcmodel=small -mno-segment-relocation-stuff -fno-builtin-printf -fno-builtin-fprintf -Wall -Wextra -Werror -Wno-misleading-indentation"
inc="-Isrc -I../elx-utils/src -I../elx-ctl/src -I$ELKS/include -I$ELKS/libc/include -I$ELKS/elks/include"
mkdir -p build-tests
"$CC" $flags $inc tests/test_model.c src/model.c ../elx-ctl/src/commands.c ../elx-utils/src/battery.c -o build-tests/test-model
"$EMU" build-tests/test-model
"$CC" $flags $inc -DDASH_TEST -DELX_CGA_TEST -maout-heap=4096 -maout-stack=4096 src/main.c src/platform_host.c src/model.c src/demo.c src/draw.c ../elx-ctl/src/commands.c ../elx-utils/src/battery.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c -o build-tests/elxdash-preview
python3 tests/test_cli.py "$EMU" build-tests/elxdash-preview
