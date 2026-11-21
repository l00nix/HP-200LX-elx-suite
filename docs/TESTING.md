# What has been checked

Cmdr and scrcap are now maintained separately. Use their own build scripts, tests
and release artifacts: [Cmdr](https://github.com/l00nix/cmdr) and
[scrcap](https://github.com/l00nix/ELKS-scrcap). The old `elx-commander` and `elx-cap`
paths below are retained only to reproduce historical suite packages. No new
standalone binary is claimed to have passed the old hardware tests.

The current releases have host tests, native ELKS cross-builds, emulator checks
and static i8086/memory audits. Dashboard tests cover read-back failures,
bounds, pending/restored zoom and actual framebuffer output.

BIOS 2.14 warning-comparison paths were exercised across all 64 ADC values
against a private ROM. The charging helper was mocked; no full charging or
sleep validation is implied. The ROM is not distributed.

The original ELXstat and ELXbat 0.1 were observed on the user's HP 200LX. Current
ELXbat 0.1.3, ELXctl, Commander, ELXcap and ELXdash still need physical
validation. Neither an emulator preview nor a compiled binary is a hardware test.

## Hardware checks still needed

| Tool | Check on the HP |
| --- | --- |
| ELXstat | Compare fields and cache behaviour with firmware state |
| ELXbat | Plug/unplug latency, charging flags, status recovery, light polarity, markers, exit |
| ELXctl | Record settings, change one at a time, read back and restore |
| ELXdash | Plug/unplug latency, charging flags, status recovery, keys, controls, zoom, restoration |
| Commander | Scratch-file operations on target filesystem; keys and shell/editor |
| ELXcap | Pixel accuracy, lower-screen noise, text reconstruction; hotkey separately |

Battery sample age under ELKS, power/sleep behaviour and performance at the
original CPU speed remain unverified. No release changes battery thresholds,
charging chemistry or serial power. Firmware timeout is separate from ELKS
sleep policy.

## Replacing previews

Record tool version, ELKS kernel version, ROM revision and whether the
image is a photo or ELXcap capture. Avoid personal file paths or document
contents in the capture. Preserve the original pixel file; provide an
aspect-correct PNG for README viewing. Update the caption and preview provenance.

## Power indicators (suite 0.1.1)

ELXbat 0.1.3 and ELXdash 0.1.1 add one-second read-only polling of BIOS byte
40:D4. Automated tests exercise battery/external/charging transitions, disabled
and enabled-but-inactive charging, read failures/recovery and pixel boundaries
of partial redraws. A fake-clock test of the production ELXbat loop checks
one-second polling, five-second gauge refresh, change-only redraws and recovery
without accepting a partially failed snapshot. Native 8086 builds and the same framebuffer/model tests run
under elksemu. Host tests also run with address/undefined-behaviour sanitizers.

Physical plug/unplug response time remains unverified: the firmware determines
when this cached flag changes. Test on external power and battery, confirm the
bolt tracks active charging rather than just a connected adapter, and compare
ELXstat. No charging configuration is changed by these indicators.
