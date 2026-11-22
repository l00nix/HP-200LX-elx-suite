# ELXdash 0.2.0-test4

Native 640x200 dashboard for firmware-aware ELKS on the HP 200LX.
No Nano-X or resident DOS software is required. The suite builds ELKS only.

[User guide](../../docs/tools/elxdash.md) covers keyboard navigation, the Battery
menu, hardware controls, experimental charging limits and cleanup behavior.
[Validation](../../docs/TESTING.md) separates historical HP acceptance from
checks of the exact current build.

This source is imported from the applications/elx-dash directory of the
published HP 200LX 4.0 source bundle. It includes the incremental renderer,
working Battery-menu selection, and test4 default charge-start lock. The
ELKS source retains shared declarations from the earlier DOS implementation;
DOS platform/backend files and executables are not included in this suite.

Build with the suite root build.sh or this directory's build-elks.sh. The
native executable uses the sibling elx-utils and elx-ctl sources. `make test`
runs host model/menu, CLI, bounded-cycle, detached-supervisor, power-state and
incremental-frame checks. `tests/test-native.sh` runs corresponding 8086
model/rendering checks with an isolated elksemu. The supervisor fixture mocks
BIOS calls; it never charges a real battery.

Credits: BATG inspired the gauges, CXCTL inspired hardware controls, and
ABC/LX and ABCUTIL by Andreas Garzotto / D&A Software informed the charging
workflow. The native ELKS supervisor is project code; original ABC binaries
and source are not bundled. @l00nix supplied HP hardware testing, reference
screenshots and design input. HP's Developer's Guide documents firmware
interfaces; ELKSWeb supplied graphics/console techniques and the font.
