> **Legacy suite snapshot.** Maintained development has moved to
> [cmdr](https://github.com/l00nix/cmdr). The standalone executable is `cmdr`.
> Commands and versions below document the original suite release.

# ELX Commander 0.1

A small two-panel file manager for the HP 200LX ELKS toolbox. Run `elxcmd` in
an 80-column, 25-row text terminal, locally or over serial. It uses the ELKS
ANSI text-console interface; no Nano-X, CGA graphics switch or mouse is needed.

This is an experimental first release. The native 8086 binary has been built
and exercised in elksemu; physical HP 200LX testing is still pending. The
included preview is captured from the actual native program in the emulator
using sample directories, then rendered as a terminal image.

## Install and start

Transfer `bin/elxcmd` using your usual binary-safe transfer method, then:

```sh
chmod 755 /root/elxcmd
/root/elxcmd --version
/root/elxcmd /root /tmp
```

You may supply zero, one or two directory paths. With no paths both panels
start in the current directory. With one path both start there. Nothing is
changed until you perform a file operation. Normal terminal colours are used,
so the HP's normal light text display is retained. No global inversion setting
is changed. An 80x25 view is required; larger terminals use the top-left 80x25.

Once tested, you can install it as `/bin/elxcmd`. This package does not modify
CF images, startup files or the kernel.

## Keyboard

| Key | Action | Letter alternative |
|---|---|---|
| Tab | Switch active panel | |
| Arrows, PgUp/PgDn, Home/End | Move selection | |
| Enter | Open folder / view regular file | |
| Left / Backspace | Parent directory | |
| F1 | Help | ? |
| F2 | Change the active panel's path | P |
| F3 | Text viewer | V |
| F4 | Edit selected regular file with `vi` | E |
| F5 | Copy selected regular file to other panel | C |
| F6 | Move selected regular file to other panel | M |
| F7 | Create a folder in the active panel | N |
| F8 | Delete selected file, link or empty folder | D |
| F9 | Open `/bin/sh` in active directory | ! |
| F10 | Quit | Q |
| R | Refresh both panels | |

Lowercase letters also work. ELKS `Esc` then `1`..`9`,`0`, xterm SS3/CSI,
and Linux-console F-key sequences are understood. In a prompt, Enter accepts,
Escape cancels, and Backspace edits. The viewer supports Space/Enter/PgDn for
next page, Home for the beginning, and Q/Escape to return. It wraps long lines
and displays non-ASCII/control bytes as dots; it never executes file contents.
F4 requires `vi`; F9 requires `/bin/sh`. The shell's directory does not change
the panels when you return; use F2 to change a panel path.

## File-operation behaviour

- Copy and move operate on the highlighted regular file only. The filename
  prompt lets you change the destination name in the other panel. Full source
  and destination paths are shown before confirmation. Only Y confirms.
- Existing destinations are refused, including symlinks. There is no overwrite
  mode in 0.1. Copy uses exclusive file creation, bounded buffers and checked
  writes. Failed or interrupted copies attempt to remove their partial target;
  a cleanup failure is reported.
- Move is implemented as a complete copy followed by source removal. If source
  removal fails, both complete files remain. It needs enough free destination
  space even on the same filesystem. To rename, point both panels at the same
  folder and supply a different destination filename.
- Copy/move attempt the source permission bits subject to the current umask and
  target filesystem. Timestamps, ownership, links and special permission bits
  are not preserved. ELKS filesystem name/permission limits still apply.
- Delete requires confirmation and removes regular files, symlinks themselves,
  or empty directories. It never recursively deletes folders or follows links.
- Keep source and target files unchanged in other programs during operations.
  This is an interactive file manager, not a transactional backup tool. A
  completed copy does not guarantee persistence through sudden power loss.

## Limits of 0.1

One selected item at a time; no recursive copy/move, multi-selection, search,
archive browsing or configurable sorting. Directories sort first, then names
in byte order. Hidden files are included. Each panel holds at most 128 entries,
with names up to 63 bytes and full paths up to 255 bytes. Longer names/paths,
stat failures or additional entries produce a visible **INCOMPLETE LIST**
notice; the program does not silently truncate names for file operations.
Visible name columns are 26 characters wide; full operation paths appear on
confirmation screens. Display text is ASCII, with unsupported bytes shown as `?`.

Ctrl-C or a termination signal exits through cleanup; a copy in progress is
cancelled. Terminal input settings and cursor visibility are restored, and the
screen is cleared. The previous screen contents are not restored. As with the
other HP applications, exit to the idle local shell before using suspend.

## Build and checks

Host build:

```sh
make host
python3 tests/test_ui.py build-host/elxcmd
cc -O2 -Isrc tests/ops.c src/files.c src/terminal.c -o build-host/test-ops
python3 tests/test_files.py build-host/test-ops
cc -O2 -Isrc tests/test_faults.c -o build-host/test-faults
build-host/test-faults
```

Configured ELKS toolchain:

```sh
ELKS=/path/to/elks sh build-elks.sh
python3 tests/test_files.py /path/to/elks/elksemu/elksemu build-elks/test-ops
# For the existing LP64 elksemu with a long/int32_t timeval mismatch:
python3 tests/prepare_emulator.py /path/to/elks/elksemu build-emulator
python3 tests/test_ui.py build-emulator/elksemu build-elks/elxcmd
```

The build explicitly targets i8086 and retains ELF, map, disassembly, stack
usage and instruction/memory audit results. File tests use isolated temporary
directories. UI tests exercise the actual binary in a pseudo-terminal, including
confirmation/cancellation and terminal restoration. Fault tests simulate short
writes, full storage, and source-removal failure. `VALIDATION.json` records the
checks completed for the packaged binary. Emulator tests use a host filesystem;
MINIX/FAT and HP keyboard/display behaviour still need physical testing.

The existing 64-bit elksemu reads/writes ELKS select timeouts with host `long`
instead of `int32_t`. This turns a 20 ms wait into an extremely long delay. The
helper above corrects those four accesses in a separate emulator copy; the
shared toolchain is untouched. The shipped binary is not patched for the emulator.

## First palmtop check

Start in two scratch directories with disposable files. Check arrows, Tab and
F1/F10; use letter shortcuts if a terminal maps F-keys differently. Try viewing,
copying, an existing-target refusal, moving, creating a folder, cancelling a
delete and confirming a delete. Check that normal shell input works after exit.
Then try F4 and F9 if `vi` and `/bin/sh` are installed. Keep valuable files out
of the first trial.

## Source and attribution

New ELX Commander code is GPL-2.0-or-later; see COPYING. It is an independent
text-mode interface inspired by classic two-panel file managers. It uses the
same ANSI terminal conventions as ELKS's bundled `fm`, but does not include
Norton Commander code or graphics. ELF-retention and ISA-audit tools derive
from the existing ELKSWeb / ELX utilities tooling (GPL-2.0-or-later).
