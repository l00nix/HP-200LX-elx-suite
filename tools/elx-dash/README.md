# ELXdash 0.1

A light-theme, standalone 640×200 HP 200LX dashboard for firmware-aware ELKS.
It needs neither Nano-X nor a mouse.

## Run

ELKS, from the local text console (normally root):

```sh
chmod 755 elxdash
./elxdash
```

`--demo` runs the same interface with synthetic values and no HP setting
changes. It still switches to graphics and restores the text display afterward.
The monochrome pixel polarity is chosen for the HP LCD and can appear inverted on ordinary CGA.
`--help` and `--version` do not access hardware.

## Controls

- **Tab / Down / Up:** select contrast, volume, text zoom, icons or timeout.
- **Left / Right**, or **- / +:** decrease/increase or cycle the selected choice.
- **R:** reread firmware settings and the battery cache.
- **Q / Esc:** exit and apply the queued text-zoom choice.

The selected control is highlighted. Contrast (0–31) and speaker volume
(0 off, 1 soft, 2 medium, 3 loud) update their half-circle gauges after firmware
read-back. The top two gauges are read-only battery voltages, not capacity.
A contrast setting at an extreme may be hard to see; HP's ON+plus/minus controls
remain available. Changes to contrast, volume, icon preference and timeout take
effect immediately and are kept on normal exit; this is not a preview/apply form.

Text zoom choices are 80×25, 64×18, 40×25 and 40×16. Modes 2/3 support all four;
mode 7 offers 80×25 and 40×16. A star marks a queued choice. It is applied and
verified after returning to text mode, since text zoom does not resize graphics.
If unchanged, the original zoom is restored after the video-mode reset. A caught
termination signal cancels only the queued zoom, restoring the original choice.
No promise is made about cleanup after a forced kill, power loss or crash.

Icons select the BIOS annunciator preference: Left, Right, Off. Their actual
visibility depends on firmware display/light-sleep handling. The setting does
not add ELKS annunciator rendering.

BIOS timeout is adjusted in nominal 30-second steps, from Off to 3604 seconds,
rounded upward to a 55 ms firmware tick. Current values are shown in whole
seconds. This is the BIOS event-wait timeout, **not ELKS auto-sleep policy**.
ELXctl remains available for exact second values on ELKS.

No charging, battery chemistry, serial power, clock, LCD power-off or inversion
controls are included. The global ON+/ LCD inversion setting is left as-is.

## Battery markers

Outlined triangles mark nominal low warnings; the solid main-battery triangle
marks nominal 2.00 V critical. Main low is 2.20 V alkaline / 2.30 V NiCad, chosen
from the configured BIOS chemistry flag. Backup low is 2.70 V for ROM 2.14;
other revisions omit that marker and say the threshold is unknown. See the
shared `elx-utils/THRESHOLDS.md` for ROM evidence, rounding and hysteresis.

Actual LOW / VERY LOW / clear flags are shown separately. A warning can remain
active above its marker because the BIOS requires a higher recovery voltage.
Values are firmware-cached, sample age unknown. The display rereads every five
seconds; neither automatic refresh nor R forces a new battery measurement.
Settings are reread before each edit and afterward. Read failures leave the
last snapshot visible as STALE and disable editing until a successful R/refresh.
A read-back mismatch shows the actual setting and a message, not false success.

## Console lifecycle and limitations

ELXdash requires HP firmware-aware ELKS, readable `/dev/kmem`, a recognizable
HP 200LX ROM, and the graphics-console ownership ioctls. Start from a logical
80×25 text page. Serial sessions are rejected. It restores text mode and terminal
input flags on normal exit, but clears the text screen, like ELXbat.

**Physical HP testing is pending.** Automated checks use host simulation, native
ELKS binaries in elksemu and isolated ROM comparison paths. They do not establish
real firmware/driver timing, battery freshness or sleep behavior.

## Build

Keep the sibling `elx-dash`, `elx-ctl` and `elx-utils` directories together.

```sh
cd elx-dash
make test
ELKS=/path/to/configured/elks sh build-elks.sh
```

The ELKS toolchain is ia16-elf-gcc targeting i8086, small model, 2048-byte heap
and 4096-byte stack. Build evidence records the compiler version used.

Host preview (synthetic data, actual renderer):

```sh
build-host/elxdash --demo --preview dash.pbm
```

`--script` is a host/test-build-only key sequence for deterministic UI tests.
`tests/test-native.sh` requires `ELKS` and `EMU` (an LP64-compatible elksemu).
`tests/test_rom_thresholds.py PRIVATE-ROM.BIN` takes a private BIOS 2.14 capture.

GPL-2.0-or-later for project code; see COPYING. The reused ELKSWeb font retains
its separate FONT-LICENSE. No original CXCTL, BATG, DOS system files or HP ROM
are distributed. ELXctl 0.1 remains a separate command-line tool.
