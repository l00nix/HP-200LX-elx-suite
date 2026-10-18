# ELXctl 0.1

Native 8086 command-line controls for the HP 200LX running firmware-aware ELKS.
Inspired by CXCTL, independently implemented using documented HP interfaces.
No Nano-X or kernel patch is required. No DOS code or HP firmware is bundled.

## Install and use

Copy `bin/elxctl` to an ELKS filesystem, for example `/bin/elxctl`, then:

```sh
chmod 755 /bin/elxctl
elxctl
```

Requires an HP firmware-aware ELKS kernel and read access to `/dev/kmem`
(normally root). HP identity is checked before any BIOS or hardware-port access.
The tool opens memory read-only. It does not change startup files or install a
resident process. Settings apply to the current session; reboot persistence is
not promised.

Each command without a value queries its current setting. With a value it
applies one change and reads the state back. Exit status is 0 for success,
1 for a hardware/state/read-back error, and 2 for invalid arguments.

| Command | Values | Meaning |
| --- | --- | --- |
| `status` | none | Show all supported settings; also the default |
| `contrast` | 0–31 | Native HP LCD contrast range |
| `volume` | 0–3 | Off, soft, medium, loud |
| `zoom` | `80x25`, `64x18`, `40x25`, `40x16` | Text viewport, keeping the current mode and text contents |
| `annunciators` | `left`, `right`, `off` | Firmware preference for Shift/Fn/Caps indicators |
| `firmware-timeout` | `off`, or 0–3604 | Firmware timeout in nominal seconds; 0 disables it |

Examples:

```sh
elxctl contrast
elxctl contrast 16
elxctl volume 2
elxctl zoom 40x16
elxctl zoom 80x25
elxctl annunciators right
elxctl firmware-timeout 180
```

Contrast extremes can make the display difficult to read. `elxctl contrast 16`
is a useful return point; the HP ON+plus/minus controls remain available.

Zoom requires a logical 80×25 text page. Modes 2 and 3 support all four sizes;
mode 7 supports only 80×25 and 40×16. Graphics modes are rejected. The utility
does not set a new video mode, clear the screen, resize the ELKS terminal or
change application geometry. Run zoom commands while at a text console, not
while another process is switching graphics modes. Firmware-aware ELKS handles
cursor tracking within the zoomed viewport.

Annunciator settings are firmware preferences. Actual visibility depends on
firmware light-sleep/display handling; changing the preference does not add
annunciator rendering to ELKS.

**The firmware timeout is not an ELKS auto-sleep setting.** Its BIOS sleep path
is separate from ELKS's power policy. Values are rounded upward to the next
nominal 55 ms tick; 180 seconds is stored as 3273 ticks (180.015 nominal
seconds). Setting it to off does not disable ELKS's own power behavior.

## Preview without applying settings

```sh
elxctl --dry-run contrast 18
elxctl --dry-run zoom 64x18
elxctl --simulate
elxctl --simulate zoom 40x16
```

`--dry-run` reads the current hardware settings and displays the planned BIOS
call, but does not apply the requested change. Its read-only queries temporarily
select hardware registers as needed, restoring the selector afterward.

`--simulate` uses an isolated in-memory model, with no HP hardware access.
It is explicitly labelled and resets on each invocation; it is not a hardware
test. It can be combined with `--dry-run`. Host builds require simulation.

## Implementation and scope

- Contrast uses INT 15h/6200h and 6201h, the HP's full 0–31 range.
- Volume uses INT 15h/48xxh. Read-back reads Hornet register 1Eh bits 7:6,
  masking interrupts across the indexed read and restoring the index selector.
  The query never writes the system-control register.
- Zoom uses INT 10h/D0xxh, choosing the monochrome/color variant for the existing
  mode. The BIOS leaves the screen contents intact.
- Annunciator control uses INT 15h/61xxh, preserving corner selection when
  disabling. Its firmware service preserves the current icon-presence bits.
- Timeout uses INT 15h/4600h and reads back the BDA reset interval.
- Firmware bridges preserve the C ABI, segment registers and incoming interrupt
  enable state. The HP firmware may internally enable interrupts; these calls
  are not claimed to be atomic across their entire execution.
- All input is parsed before opening hardware. Invalid/extra arguments cannot
  partially apply a sequence: one setting is accepted per invocation.
- A failed read-back is reported as failure, not as success or automatic
  rollback. A concurrent firmware/user change may also cause this result.

Serial power/routing, charging/battery-type changes, LCD power-off, display
inversion, clock speed, BIOS message suppression and RTC adjustment are outside
0.1. In particular, serial power belongs to the ELKS serial driver's open/close
lifecycle, and the DOS DISP approach has power sequencing and contrast restore
requirements. Those are separate additions, not silently mapped to raw writes.

## Build and validation

```sh
make test
ELKS=/path/to/configured/elks sh build-elks.sh
python3 tests/test_cli.py /path/to/elksemu build-elks/elxctl
/path/to/elksemu build-elks/test-commands
python3 tests/test_abi.py build-elks/elxctl.elf
```

The last test requires Unicorn. It executes the actual linked 16-bit bridge
instructions with mocked BIOS and I/O responses, checking argument passing,
callee-saved/segment/stack/IF preservation, and the selector-preserving volume
read. It does not emulate HP firmware behavior.

The build targets i8086 and includes ELF, disassembly, map, stack-usage and ISA/
memory audit evidence. CLI/command tests cover getters, setters, dry-run,
simulation, invalid/overflow inputs, every supported zoom transition, graphics
and geometry rejection, every accepted timeout value, and read-back mismatch.
Host tests also run under AddressSanitizer and UndefinedBehaviorSanitizer.

**Real HP testing is still pending.** First record `elxctl status`, then test
one control at a time and return it to the recorded value. No palmtop, CF image,
kernel or startup configuration was modified while producing this package.
See VALIDATION.json for the build results.

## References and licensing

GPL-2.0-or-later; see COPYING. Read-only HP identification derives from ELXstat;
the build/audit helpers derive from the existing ELX/ELKSWeb toolchain.
CXCTL's command capabilities were a reference, but no CXCTL assembly was copied.

Primary hardware reference: *HP 100LX/200LX Developer's Guide*, INT 10h/D0h;
INT 15h/46h, 48h, 61h and 62h; BIOS data area video/timeout/annunciator fields;
and Hornet register 1Eh. ROM BIOS 2.14 service code was inspected to cross-check
timeout storage, contrast range, annunciator masks and speaker volume bits.
