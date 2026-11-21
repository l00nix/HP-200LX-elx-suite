#!/bin/sh
# Build only the four HP firmware tools. Never install into the toolchain tree.
set -eu
cd "$(dirname "$0")"
: "${ELKS:=${TOPDIR:-}}"
: "${ELKS:?Set ELKS to a configured ELKS source/toolchain}"
ELKS=$(cd "$ELKS" && pwd)
export ELKS
CROSS=${CROSS:-${CROSSDIR:-$ELKS/cross}}
if [ ! -x "$CROSS/bin/ia16-elf-gcc" ]; then
    compiler=$(command -v ia16-elf-gcc) || {
        echo "IA16 compiler not found in CROSS or PATH" >&2
        exit 1
    }
    CROSS=$(dirname "$(dirname "$compiler")")
fi
export CROSS
for tool in elx-utils elx-ctl elx-dash; do
    (cd "tools/$tool" && sh build-elks.sh)
done
mkdir -p build
for spec in elx-utils/elxstat elx-utils/elxbat elx-ctl/elxctl elx-dash/elxdash; do
    dir=${spec%/*}; name=${spec#*/}
    cp "tools/$dir/build-elks/$name" "build/$name"
    cp "tools/$dir/build-elks/$name-audit.json" "build/$name-audit.json"
done
