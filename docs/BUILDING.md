# Building the ELX suite

Cmdr and scrcap are now maintained separately. Use their own build scripts, tests
and release artifacts: [Cmdr](https://github.com/l00nix/cmdr) and
[scrcap](https://github.com/l00nix/scrcap). The old `elx-commander` and `elx-cap`
paths below are retained only to reproduce historical suite packages. No new
standalone binary is claimed to have passed the old hardware tests.

Keep the sibling directories under `tools/` together: the dashboard reuses the
control and graphics sources directly. Source builds do not install anything
on a palmtop or change an existing ELKS tree.

## Host builds

A C compiler, make and Python 3 are required. Host builds use explicit offline
fixtures or simulation for HP hardware functions.

```sh
make -C tools/elx-utils test
make -C tools/elx-ctl test
make -C tools/elx-dash test
make -C tools/elx-cap test
make -C tools/elx-commander host
python3 scripts/check_docs.py
```

Each tool's source README describes its full test suite, including Commander
file-operation tests. macOS sanitizer builds may need
`-Wno-deprecated-declarations` for bounded uses of legacy `sprintf`; target
libraries do not necessarily offer modern hosted-library replacements.

## ELKS

Use a configured ELKS tree with the ia16-elf compiler and matching headers/libs:

```sh
cd tools/elx-utils
ELKS=/path/to/configured/elks sh build-elks.sh
```

Repeat in `elx-ctl`, `elx-cap`, `elx-commander` and `elx-dash`. Outputs go into
each tool's `build-elks/` directory, with ELF/map/disassembly and static
instruction/memory audit evidence. Targets explicitly select i8086.

For emulator tests, use an elksemu with correct 32-bit ELKS timeval conversion
on a 64-bit host. Commander includes an isolated-emulator preparation helper;
it does not patch the shared toolchain. See its source README.

## Release preparation

Built executables live in GitHub Releases, not the Git source tree. Local
`release-assets/` is ignored. `release-notes/v0.1.1.md` and
`release-notes/v0.1.1-manifest.json` describe the current prerelease. The prepared
assets contain their own checksums and full packages with validation evidence.
