# ELXstat 0.1 and ELXbat 0.1.3

Native HP 200LX utilities for the ELKS toolbox. These are experimental,
read-only battery/status utilities. User photos confirm that both 0.1 utilities
run on an HP 200LX with ELKS 3.8 beta 1 and display matching voltages.
ELXstat reports status in the shell. ELXbat draws two half-circle voltage gauges
with large numerical readings directly in CGA graphics, using the same console
and framebuffer approach as ELKSWeb. It does not require Nano-X or a mouse.

ELXbat 0.1.1 corrects the framebuffer polarity so that it opens with dark gauges
and text on a light background under the HP's normal LCD setting. The earlier
0.1 build appeared dark on the actual palmtop despite its light host preview.
This update changes pixel drawing, not the global ON+/ inversion setting.
If you previously inverted the LCD with ON+/, return it to its normal setting;
ON+/ remains available to invert the display manually. ELXstat stays at 0.1.
The updated binary's appearance still needs confirmation on the palmtop.
The photos do not establish battery-cache freshness or complete exit/sleep testing.

## Warning markers (introduced in ELXbat 0.1.2)

Outlined triangles show nominal low-battery thresholds: 2.20 V for configured
alkaline main batteries, 2.30 V for configured NiCad, and 2.70 V for backup on
ROM BIOS 2.14. A solid main-battery triangle marks nominal 2.00 V critical.
The backup marker is omitted for other ROM version strings. The scale remains
2.0–3.5 V, with below-scale samples labelled. Battery values are volts, not
remaining charge percentages. The actual firmware LOW / VERY LOW / clear flags
remain separate from markers and are never inferred from gauge position.

See THRESHOLDS.md for raw ADC comparisons, hysteresis and limitations. These
are reference markers; no warning threshold, charging policy or polling interval
is changed. Text output also includes the marker values. 0.1.2 is cross-built
and emulator-tested but still awaits physical HP testing.

## Requirements

- An HP 200LX running the HP firmware-aware ELKS port, such as 3.8 beta 1.
- `/dev/kmem` readable by the caller (normally root).
- Recognizable HP 200LX ROM identity service and ROM version string. The local
  ROM 2.14 capture supports this implementation; other revisions need testing.
- For graphics, launch on the local HP console with no other graphics program
  running. The ELKS console must support `DCGET_GRAPH`/`DCREL_GRAPH`.
- No new kernel, DOS loader, root filesystem image, or display server is needed.

The live reader rejects kernels whose uname version/release does not contain
`hp200lx`, and rejects an unrecognized HP ROM signature. Those are compatibility
checks, not proof that a particular kernel continues servicing batteries.

## Install and run

Transfer the two files in `bin/` to `/root/elxstat` and `/root/elxbat` using the
existing binary-safe transfer method. On ELKS:

```sh
chmod 755 /root/elxstat /root/elxbat
/root/elxstat --version
/root/elxbat --version
/root/elxstat -v
/root/elxbat --text
```

From the **local HP screen**, run `/root/elxbat` for graphics. Q, Escape, or Ctrl-C
exits; R rereads the firmware cache. The default refresh interval is five seconds.
The application restores the previous BIOS text mode and terminal input settings,
and clears the text screen on exit. It does not restore the previous screen
contents or zoom setting. BIOS graphics timing and /DS return-to-text operation
still require testing on the physical palmtop.

Once validated, optionally copy the tools into `/bin`:

```sh
cp /root/elxstat /bin/elxstat
cp /root/elxbat /bin/elxbat
```

Do not test suspend with the graphics application running. Exit to the local
idle shell first, following the existing HP port's sleep procedure.

## Commands

```text
elxstat                       Battery and power summary
elxstat -v                    All implemented sections
elxstat battery               Main/backup voltage and battery flags
elxstat power                 Battery summary plus firmware power settings
elxstat display               Mode, zoom code, BIOS text geometry, display page
elxstat serial                BIOS COM1 address; current serial-stdin protocol
elxstat system                ROM/kernel identity and diagnostic raw values
elxbat                        Standalone CGA half-circle gauges
elxbat --text                 One serial-friendly text snapshot
elxbat --text --watch          Repeated snapshots; Ctrl-C stops
elxbat --interval 10           Graphics refresh every ten seconds
```

`--graphics` explicitly requests graphics; it still requires a controlling local
console capable of acquiring the graphics lock. From a serial shell, use `--text`.
The program never opens the UART to inspect it. ELXstat can read the protocol from
an already-open serial stdin descriptor; otherwise it reports that field unavailable.
Both utilities accept `--help` and `--version` without accessing hardware.

## What the readings mean

The gauges measure **voltage, not remaining charge percentage or charging history**.
Both span 2.0-3.5 V, like the BATG reference. Readings below the scale retain their
numerical value, display an empty arc, and say "Below 2.0 V". An invalid sample
shows "Unavailable" instead of a plausible voltage. The numerical conversion
range is 1.570-3.300 V with approximately 27 mV steps; three decimal places do not
imply millivolt measurement accuracy.

Version 0.1 reads HP's cached, reference-adjusted main/backup measurements at
BIOS data addresses 0040:009A and 0040:009B. Conversion follows the HP Developer's
Guide: `mV = 1570 + floor(1730 * code / 63)`, for codes 0-63. It does not call
INT 15h battery measurement functions or manipulate the ADC. The ROM 2.14
measurement paths at F000:AAE3 and F000:ABC6 store the adjusted averages there.
The DOS reference values 2.558 V and 2.860 V correspond to codes 36 and 47.

All battery, charger and low-battery indications are **firmware-cached; their age
is unknown**. A five-second refresh rereads the cache; it does not force a new
measurement. The polling interval/countdown are diagnostic configuration/state,
not proof that a measurement just succeeded. Repeated snapshots and checks after
AC insertion/removal must establish freshness on hardware. Pending battery/charger
updates are retried a bounded number of times; an unstable cache produces an error.

Battery type is the configured type, not chemical identification. Firmware alert
"clear" is a status bit, not a new capacity assessment. The charging timer and
battery interval use the guide's nominal 55 ms timer units. Display, alarm,
timeout and keyboard fields are labelled as firmware information; they must not
be mistaken for the ELKS driver's active policy. The BIOS text geometry describes
the text buffer, which may differ from the visible zoomed window.

## Scope of 0.1

Implemented: battery readings, configured type, low-battery flags, external-power
flag, charging enable/active/fast flags and timer, measurement interval, basic BIOS
display data, ROM identity, kernel identity, firmware keyboard settings, and a
current-serial-stdin protocol query. Shared code supplies both utilities.

Not implemented: charging controls, battery-type changes, automatic calibration,
capacity estimation, history logging, contrast/frame-rate access, PCMCIA CIS
identification, UART power/modem-line inspection, PIT/IRQ controls, or raw Hornet
register access. These require additional driver interfaces and hardware validation.

## Offline development

No physical ROM dump is distributed. `tests/make_fixtures.py` generates artificial
1 MiB physical-memory images containing an identity marker and chosen BIOS fields.
The programs accept `--image FILE` for these offline images and label them
**OFFLINE IMAGE**. These are test inputs, never claimed to be live captures.

```sh
make host
python3 tests/test_cli.py
build-host/elxbat --image tests/fixtures/dos-reference.img --preview preview.pbm
```

The host and test-target renderers use the same CGA drawing code as the native
application. PBM output is native 640x200. Display it at 640x400 to account for
CGA's 2:1 pixel aspect ratio. Host builds cannot read live `/dev/kmem`.

On the existing Linux build machine, or another configured ELKS toolchain:

```sh
ELKS=/path/to/elks sh build-elks.sh
ELKS=/path/to/elks sh tests/test-target.sh
```

The build produces native IA16 executables in `build-elks/`, retains their linked
ELF files, and audits their instruction set and near-memory budgets. Compiler
warnings are errors. The target tests run the actual 16-bit programs in elksemu
against synthetic images. They do not execute real HP BIOS or hardware.

`tests/test_console.c` exercises actual console lifecycle code with mocked I/O:
acquisition, translated keys, busy-owner refusal, startup failures and restoration.
Host sanitizer tests cover conversion and timer overflow. See the package's
`VALIDATION.json` for the checks actually completed.

## Hardware test sequence

1. Run both `--version` commands, then `elxstat -v` and `elxbat --text` over serial.
2. Compare raw codes and voltages with DOS readings under comparable conditions;
   allow for elapsed time/load and the approximately 27 mV quantization.
3. Confirm the configured polling interval is nonzero and the countdown changes.
   Check whether power/charging flags follow AC removal/reconnection; this is
   required before treating them as current.
4. Run graphics locally. Confirm two readable gauges, Q/Escape/Ctrl-C exit, normal
   keyboard input and text display afterward, including with /DS.
5. Exit graphics, follow the port's supported sleep procedure, wake, and repeat
   snapshots to check resumed firmware measurement service.

## Attribution

ELXstat/ELXbat source is GPL-2.0-or-later; see COPYING. Naming and behaviour are
inspired by LXSTAT and BATG. No code or executable from those DOS utilities is
included. Source addresses/formulas come from the HP 100LX/200LX Developer's Guide.
The user's book supplied the DOS command output and BATG visual reference; the
half-circle design follows the later user-supplied gauge reference.

Console lifecycle and CGA clipping techniques, the ELF-retention helper, and the
instruction audit derive from ELKSWeb (GPL-2.0-or-later). The embedded regular font
is ELKSWeb's X.Org helvR08 ASCII subset; its Adobe/DEC notices and redistribution
terms are retained in FONT-LICENSE and src/font.h. Battery measurements/status
are read only. Graphics mode and terminal input changes are temporary UI operations.

## Automatic power indicator

The graphical display checks the firmware power-status byte once per second,
independently of the five-second default gauge refresh. The plug with a short
cable means external power; a battery outline means battery power. A lightning
bolt appears only when the firmware reports active charging, including fast
charging. Enabled-but-inactive charging does not show a bolt. The battery
outline is a power-source symbol, not a remaining-charge estimate.

A read failure replaces the symbol with `?` and labels the power status unknown.
The indicator recovers after a successful read. Power changes redraw only the
status strip, with no full-screen flash. Reads do not change any power or charging
settings. Firmware-cache update latency is unknown until hardware plug/unplug
testing; a one-second poll does not guarantee one-second physical detection.
