# ELXbat 0.1.4

Two large half-circle battery-voltage gauges, dark ink on a light background.

## How to use

```sh
chmod 755 elxbat
./elxbat
./elxbat --text
./elxbat --text --watch --interval 5
```

Run graphics from the local HP text console; use `--text` over serial. Requires
an HP firmware-aware ELKS kernel and readable `/dev/kmem` (normally root).

| Key | Action |
| --- | --- |
| ![R](../assets/keys/r.svg) | Reread the firmware cache |
| ![Q](../assets/keys/q.svg) or ![Esc](../assets/keys/esc.svg) | Exit graphics |
| ![Ctrl + C](../assets/keys/ctrl-c.svg) | Stop text watch or exit through cleanup |

![ELXbat with nominal warning markers](../assets/previews/elxbat-offline.png)

*Actual renderer using an offline fixture. The backup LOW flag above its marker
illustrates firmware hysteresis, not a discrepancy in the gauge.*

Outlined triangles mark nominal low thresholds: main 2.20 V alkaline / 2.30 V
NiCad; backup 2.70 V on BIOS 2.14. The solid triangle is nominal main critical
2.00 V. Unknown ROM versions omit the backup marker. Cached LOW / VERY LOW /
clear flags are shown independently. Values are volts, not charge percentages.

The program never changes battery policy or thresholds. Global LCD inversion
remains controlled by ![ON + /](../assets/keys/on-slash.svg); use the normal LCD polarity
for the default light appearance. Version 0.1.4 redraws only changed gauges and status regions. Earlier HP testing
confirmed steady display and manual/power updates; see the [current build record](../TESTING.md).

[Threshold evidence](../../tools/elx-utils/THRESHOLDS.md) · [Full notes](../../tools/elx-utils/README.md) · [Suite](../../README.md)

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
