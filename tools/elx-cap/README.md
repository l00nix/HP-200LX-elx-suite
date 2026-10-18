# ELXcap 0.1

Native 8086/ELKS screenshot utility for the HP 200LX. Inspired by GEMCAP's
hotkey-to-file workflow; an independent implementation with no GEMCAP or CAPLX
code, DOS TSR, or HP ROM/font data distributed. No Nano-X dependency.

## Image formats and quality

The default is uncompressed, 1-bit monochrome **BMP**, 640 × 200 pixels,
exactly 16,062 bytes. `-f pbm` selects binary PBM (P4), 16,011 bytes.
Both preserve every captured pixel. GEM IMG would not improve image quality;
its compression can reduce size. PNG can be produced losslessly on the Mac.
JPEG is unsuitable for crisp monochrome text and line art.

Files contain square pixels without a forced physical print size. If the image
looks too shallow compared with the LCD, adjust its display/print aspect ratio
in the document application; retain the original for pixel-exact archival use.

## Install and capture on the existing kernel

Copy `bin/elxcap` to an ELKS filesystem, for example `/bin/elxcap`, and run
`chmod 755 /bin/elxcap`. Requires the HP firmware-aware ELKS kernel and read
access to `/dev/kmem` (normally root). The program opens memory read-only.

From an existing **serial shell**, while the desired screen is on the palmtop:

```sh
mkdir /root/shots
elxcap /root/shots/SCREEN.BMP
```

This captures the physical HP display, not the serial terminal. No serial port
is opened by ELXcap. No filename means `CAP0000.BMP`, `CAP0001.BMP`, etc. in the
current directory. Existing files and symlinks are never overwritten.

To capture an application from the palmtop keyboard without a new kernel:

```sh
elxcap -q -d 10 /root/shots/BATTERY.BMP >/tmp/elxcap.log 2>&1 &
elxbat
```

This starts a ten-second delayed capture and then opens ELXbat. Choose a fresh
filename for each invocation, or run from `/root/shots` and omit the filename.
Do not quit the graphics application before the delay has elapsed.

Other options:

```sh
elxcap -f pbm /root/shots/SCREEN.PBM
elxcap -i /root/shots/INVERSE.BMP
elxcap --help
```

`-i` flips output pixels only. It does not change the LCD. Format selection is
explicit; a filename suffix alone does not change the format.

## Repeated capture without a kernel patch

```sh
elxcap --watch /root/shots >/tmp/elxcap.log 2>&1 &
```

The log reports the process ID. From a serial shell, use `kill -10 PID` to
request a screenshot (`SIGUSR1`), and `kill PID` to stop (`SIGTERM`). Substitute
the actual PID; do not type the word PID. This uses no terminal input and does
not compete with the foreground application's keyboard. Redirect logs to avoid
changing the screen being captured. Signals arriving during capture may be
coalesced; this is a still screenshot utility, not a video recorder.

## GEMCAP-style Alt+period (optional kernel integration)

**Alt+period is not available on an unchanged 3.8 beta1 kernel.**
The package includes a source patch, build-checked against an isolated copy of
that kernel. It must be integrated into a separately built and tested kernel
before this command works:

```sh
elxcap --hotkey /root/shots >/tmp/elxcap.log 2>&1 &
```

Press Alt plus period to request a numbered capture. The keyboard driver
consumes that chord while the listener is active; other input passes normally.
Repeated make codes are suppressed until the period key is released. Ctrl or
Shift combinations are not intercepted. No file writing, BIOS calls or display
access occurs inside the keyboard interrupt. The listener polls every 100 ms;
capture is not an atomic snapshot at the exact keypress instant.

The `/dev/kmem` extension uses commands `0x4501` (acquire/renew lease and return
16-bit event counter) and `0x4502` (release). One PID owns the lease. It expires
after three seconds without renewal, so terminating or crashing the listener
does not permanently reserve the shortcut. Acquisition conflicts return EBUSY;
an unpatched kernel is detected and reported rather than silently falling back.

To apply to a **separate copy** of the matching ELKS source:

```sh
python3 kernel/apply.py /path/to/isolated/elks-tree
```

Requires `CONFIG_HP200LX_FW`, `CONFIG_CHAR_DEV_MEM`, and the firmware-aware
`kbd-scancode.c` driver. `kernel/hotkey.patch` provides the same changes as a
unified diff; use either method, not both. No boot kernel or CF image is replaced
by this package. Keyboard behavior still needs physical HP validation.

## Capture coverage and limitations

- CGA mode 6: both video-memory banks are copied, then rearranged into rows.
  Default polarity is HP dark ink on light paper, matching ELXbat 0.1.1.
- Text modes 2/3: captures the active **full logical 80×25 page**, normalized
  to 8×8 characters. Reads the lower 128 glyphs from the HP's standard ROM table
  and the upper 128 from its current standard code-page RAM table. This is
  GEMCAP-style text reconstruction, not a capture of the LCD's zoomed viewport.
- Reverse/hidden text is preserved. Intensity is flattened to monochrome;
  blink is shown steadily, and the hardware text cursor is omitted.
- LCD zoom/pan, global ON+/ inversion, custom hardware font-bank replacements,
  annunciators, and hardware-only overlays are not reproduced. Use `-i` if an
  inverted output is wanted. Software graphics already in video RAM is retained.
- Other video modes/geometry are explicitly rejected. Mode 7 is not supported
  in 0.1; it is not treated as an ordinary 8×8 HP mode.
- A changed mode/page during reading is rejected. An application that redraws
  during the copy can still cause tearing; capture a static screen and retry.
- Memory is copied before writing the file. Disk-full/short-write/close errors
  are detected and the incomplete new file is removed when possible.
- Direct CGA reads need testing on the HP: CAPLX's author reported lower-screen
  noise with direct reads under DOS. Emulator tests cannot settle that issue.

## Build and test

```sh
make test
ELKS=/path/to/configured/elks sh build-elks.sh
python3 tests/test_capture.py /path/to/elksemu build-elks/elxcap
```

The cross-build targets `i8086`, uses 2 KB heap and 2 KB stack, retains ELF/map/
disassembly/stack-usage evidence, and checks its executable instructions and
near-memory budget. Host builds only read explicit `--image` test fixtures;
they never attempt to capture the host screen. Synthetic test images contain
no HP ROM or font copies.

The test suite checks graphics row ordering, BMP headers/palette, PBM equality,
inversion, active text pages, high code-page glyphs, reverse/hidden text, failed
reads and unsupported modes, overwrite/symlink refusal, numbering, delay,
repeated signal requests, shutdown, partial writes, disk-full cleanup and the
hotkey lease state machine. See the packaged VALIDATION.json for results and
the remaining hardware checks.

ELKS emulator note: the reused isolated emulator from the Commander build fixes
LP64 `select()` timeval conversion (32-bit ELKS longs versus 64-bit host longs).
Signal tests allow a 200 ms settling interval after readiness/capture output
to avoid an observed emulator signal-delivery timing issue around output
syscalls. No such delay is built into the application's signal handler.

## License and references

GPL-2.0-or-later; see COPYING. Toolchain verification helpers originate from
our ELKSWeb/ELX utilities. The optional kernel patch extends ELKS's existing
GPL driver. GEMCAP is a behavioral reference only.

Hardware reference: *HP 100LX/200LX Developer's Guide*, BIOS data area video
fields, F000:FA6E lower character table, 9000:FC00 standard upper font, and
CGA graphics addressing. The ROM-identity detection approach is shared with
ELXstat. No firmware interrupt hooks or display-controller writes are used.

GEMCAP reference archive: https://casio.ledudu.com/HPLX/gemcap.zip
