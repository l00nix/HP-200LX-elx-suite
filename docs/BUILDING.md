# Building the four HP tools

Keep the sibling directories under `tools/` together: ELXdash reuses control and
graphics sources. Cmdr and scrcap are separate projects and are not built here.

Host checks require a C compiler, make and Python 3:

```sh
make test
```

For native ELKS builds, use a configured ELKS tree with the IA16 compiler and its
matching libc installed:

```sh
ELKS=/path/to/elks sh build.sh
make install DESTDIR=/path/to/staging-root
```

The root build script runs the existing per-tool build scripts and collects four
executables plus audit reports under `build/`. Each tool's `build-elks/` retains
its ELF/map/disassembly. Builds target i8086 and audit instruction coverage and
near-memory use. Installation copies only the seven explicit paths documented
in the EPT recipe. It does not install into or modify the ELKS toolchain tree.

Package version 0.2.0 is a packaging/split release. Tool source versions are
ELXstat 0.1, ELXbat 0.1.3, ELXctl 0.1 and ELXdash 0.1.1. Later experimental HP
charging work is not imported by this packaging change.
