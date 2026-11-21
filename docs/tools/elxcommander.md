> **Legacy suite snapshot.** Maintained development has moved to
> [cmdr](https://github.com/l00nix/cmdr). The standalone executable is `cmdr`.
> Commands and versions below document the original suite release.

# ELX Commander 0.1

A two-panel, 80×25 text file manager for ELKS. Local console or serial; no Nano-X.

## How to use

```sh
chmod 755 elxcmd
./elxcmd /root /tmp
```

Zero paths opens the current directory in both panels; one path opens it in
both. Two paths set the left and right panels independently.

![ELX Commander emulator preview](../assets/previews/elxcommander-emulator.png)

*Actual native program running in an emulator with sample directories; terminal
output rendered as an image. Physical HP keyboard/filesystem checks are pending.*

| Key | Action | Alternative |
| --- | --- | --- |
| ![Tab](../assets/keys/tab.svg) | Switch panel | |
| ![↑](../assets/keys/up.svg) / ![↓](../assets/keys/down.svg) | Select item | |
| ![PgUp](../assets/keys/pgup.svg) / ![PgDn](../assets/keys/pgdn.svg) / ![Home](../assets/keys/home.svg) / ![End](../assets/keys/end.svg) | Navigate list | |
| ![Enter](../assets/keys/enter.svg) | Open folder / view file | |
| ![←](../assets/keys/left.svg) / ![Backspace](../assets/keys/backspace.svg) | Parent directory | |
| ![F1](../assets/keys/f1.svg) | Help | ![?](../assets/keys/question.svg) |
| ![F2](../assets/keys/f2.svg) | Change panel path | ![P](../assets/keys/p.svg) |
| ![F3](../assets/keys/f3.svg) | View text | ![V](../assets/keys/v.svg) |
| ![F4](../assets/keys/f4.svg) | Edit with `vi` | ![E](../assets/keys/e.svg) |
| ![F5](../assets/keys/f5.svg) | Copy file | ![C](../assets/keys/c.svg) |
| ![F6](../assets/keys/f6.svg) | Move file | ![M](../assets/keys/m.svg) |
| ![F7](../assets/keys/f7.svg) | Make directory | ![N](../assets/keys/n.svg) |
| ![F8](../assets/keys/f8.svg) | Delete file/link/empty directory | ![D](../assets/keys/d.svg) |
| ![F9](../assets/keys/f9.svg) | Shell in active directory | ![!](../assets/keys/bang.svg) |
| ![F10](../assets/keys/f10.svg) | Quit | ![Q](../assets/keys/q.svg) |
| ![R](../assets/keys/r.svg) | Refresh panels | |

Lowercase alternatives work. Prompts use ![Enter](../assets/keys/enter.svg) to accept,
![Esc](../assets/keys/esc.svg) to cancel, and ![Backspace](../assets/keys/backspace.svg) to edit. Only ![Y](../assets/keys/y.svg)
confirms a file operation. The viewer uses ![Space](../assets/keys/space.svg) / ![Enter](../assets/keys/enter.svg) /
![PgDn](../assets/keys/pgdn.svg) for next page, ![Home](../assets/keys/home.svg) for start, and ![Q](../assets/keys/q.svg) /
![Esc](../assets/keys/esc.svg) to return. ELKS function-key sequences also work as
![Esc](../assets/keys/esc.svg) **then** a digit (1–9, 0 for F10).

Copy/move handles one regular file; existing targets are refused. Move copies
then removes the source, so it needs free space. Delete is non-recursive and
requires confirmation. No multiselect, recursive copying or archive browsing
is included. Panels show at most 128 entries; incomplete listings are labelled.

Try disposable files first. [Full file-operation semantics and build notes](../../tools/elx-commander/README.md) · [Suite](../../README.md)
