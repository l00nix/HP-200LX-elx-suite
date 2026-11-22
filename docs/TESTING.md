# What has been checked

Package 0.2.1 contains ELXstat 0.1, ELXbat 0.1.4, ELXctl 0.1 and
ELXdash 0.2.0-test4. Cmdr and scrcap have separate repositories and records.

## Source provenance

The newer battery and dashboard files come from the `applications/` directories
of the published HP 200LX 4.0 source bundle. Imported files were checked against
that bundle's SHA-256 source manifest. The ELKS runtime sources retain their
original versions. The host Makefile omits DOS-only tests because DOS backends
are not distributed here. No ABC/LX binaries or HP ROM are included.

## Current software checks

- Host tests cover CLI validation, controls, power polling, failed reads,
  menu navigation and confirmation, queued zoom, and framebuffer rendering.
- Incremental renderer tests compare 58 dashboard frames with full reference
  renders, check isolated gauge changes, stable menus and unchanged polling.
  ELXbat has equivalent incremental-frame and production polling-loop tests.
- The bounded-charge model and actual detached-supervisor fixture test the
  default Start lock, duplicate starts, parent exit, stop IPC, clock/read faults,
  adapter loss and verified shutdown. BIOS calls are mocked, never hardware.
- All four native IA16 builds pass i8086 instruction and near-memory audits
  against ELKS `df2a61c5d72415ff4825aab33be427de9306c230` and matching GCC 6.3.0/libc.
- Native elksemu dashboard tests pass for the model, CLI/rendering, power
  transitions and bounded-cycle model. These are software/emulator checks.
- EPT packages own exactly four executables and three license/usage files.
  Installed paths fit 8.3. Installation performs no kernel, startup, firmware
  setting or charging action.

The 0.2.1 package passed an isolated ELKS/QEMU EPT 1.4 installation/removal
cycle: all seven hashes, the four version commands, package-owned deletion and
preservation of an unrelated file and the EPT executable. No HP hardware calls
or graphics were launched on that generic VM.

## Physical HP evidence and remaining scope

During HP 200LX 4.0 acceptance, the user confirmed steady ELXbat 0.1.4 updates,
R and power-icon behavior. ELXdash 0.2.0-test4 menu navigation, steady polling
and default Start lock were also reported working. These earlier observations
are not a new physical acceptance of every rebuilt binary or every control.
A blank screen after one dashboard exit was reported during that sequence;
full text restoration on the current build still needs a separate check.

On 21 November 2026 the earlier package 0.2.0 passed an actual HP installation
from 8088retro using EPT 1.4, all seven hashes, version commands, ELXstat firmware
reads and a repeat-install check. That package used the older dashboard and
battery display; its installation result must not be mistaken for 0.2.1 GUI
acceptance. The test HP runs `ELKS 0.9.2 hp200lx-4.0-local`.

The connected HP then upgraded from 0.2.0 to **0.2.1** using `ept update`
and `ept install elxtools` directly against 8088retro. All seven installed
files matched the published SHA-256 hashes; ELXbat reported 0.1.4 and ELXdash
0.2.0-test4. ELXbat's text report read the firmware successfully, and a repeated
install correctly reported up to date. The existing elxcap/elxcmd files were
unchanged. No graphical program or charging test was launched in this session.

Current hardware acceptance should cover local graphics, unchanged polling,
R, power-icon transitions, Battery-menu arrows, locked Start, Q/Esc text
restoration and keyboard/zoom restoration. ELXctl setting changes require
read-back and restoration. Cached battery sample age remains unknown.

The ELKS charging supervisor is experimental and not physically qualified.
It is locked by default and requires `--charge-test` plus explicit confirmation.
DOS LXdash/ABC charging tests do not validate this separate implementation.
No full-charge profile or safe unattended operation is claimed. Firmware timeout
is separate from ELKS sleep policy.

## Previews

README previews retain their original captions and versions. They illustrate
older renderer snapshots and do not claim to be current hardware captures.
Replacement captures should record tool/kernel/ROM version and capture origin.
