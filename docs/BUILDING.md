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

Package 0.2.1 updates the four-tool suite to ELXstat 0.1, ELXbat 0.1.4,
ELXctl 0.1 and ELXdash 0.2.0-test4. ELKS sources come from the published HP
200LX 4.0 application bundle. DOS backends and DOS executables are not built.
The Battery menu's native charging supervisor remains experimental and requires
explicit runtime opt-in; installation never starts it.
