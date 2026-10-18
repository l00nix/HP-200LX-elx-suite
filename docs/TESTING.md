# What has been checked

The current releases have host tests, native ELKS cross-builds, emulator checks
and static i8086/memory audits. Dashboard tests cover read-back failures,
bounds, pending/restored zoom and actual framebuffer output. DOS tests execute
the compiled EXE with mocked DOS/HP services, including text/cursor/zoom
restoration. They are not a full DOS compatibility test.

BIOS 2.14 warning-comparison paths were exercised across all 64 ADC values
against a private ROM. The charging helper was mocked; no full charging or
sleep validation is implied. The ROM is not distributed.

The original ELXstat and ELXbat 0.1 were observed on the user's HP 200LX. Current
ELXbat 0.1.2, ELXctl, Commander, ELXcap and both dashboards still need physical
validation. Neither an emulator preview nor a compiled binary is a hardware test.

## Hardware checks still needed

| Tool | Check on the HP |
| --- | --- |
| ELXstat | Compare fields and cache behaviour with firmware state |
| ELXbat | Light polarity, marker legibility, actual warning flags, exit |
| ELXctl | Record settings, change one at a time, read back and restore |
| ELXdash | Keys, immediate controls, pending zoom, terminal restoration |
| LXdash | Real DOS behaviour, text/cursor restoration, any TSR interactions |
| Commander | Scratch-file operations on target filesystem; keys and shell/editor |
| ELXcap | Pixel accuracy, lower-screen noise, text reconstruction; hotkey separately |

Battery sample age under ELKS, power/sleep behaviour and performance at the
original CPU speed remain unverified. No release changes battery thresholds,
charging chemistry or serial power. Firmware timeout is separate from ELKS
sleep policy.

## Replacing previews

Record tool version, OS/kernel or DOS version, ROM revision and whether the
image is a photo or ELXcap capture. Avoid personal file paths or document
contents in the capture. Preserve the original pixel file; provide an
aspect-correct PNG for README viewing. Update the caption and preview provenance.
