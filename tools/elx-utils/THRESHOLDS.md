# Nominal BIOS battery warning markers

The HP 100LX/200LX Developer's Guide documents nominal main low thresholds of
2.2 V alkaline and 2.3 V NiCad, software shutdown around 2.0 V, and hardware
backup-mode entry around 1.8 V. These are battery-pair voltages. The display's
2.0–3.5 V scale deliberately does not imply a battery capacity percentage.

A private HP 200LX ROM BIOS 2.14 capture (05/24/94) was inspected and its
comparison instructions exercised with all 64 ADC codes using Unicorn:

| Flag | Set below adjusted ADC code | Clear at or above code | Shown nominal marker |
|---|---:|---:|---:|
| Main low, alkaline configured | 23 | 32 | 2.20 V |
| Main low, NiCad configured | 27 | 32 | 2.30 V |
| Main very low | 15 | 32 | 2.00 V |
| Backup low | 41 | 48 | 2.70 V |

Conversion used by the existing utilities is 1.570 + raw × 1.730/63 volts.
The corresponding comparison boundaries are approximately 2.202, 2.311,
1.982, and 2.696 V. Labels and marker positions use the rounded nominal values
above. Actual ADC resolution is about 27.5 mV; the three-digit voltage readout
does not imply 1 mV measurement accuracy.

ROM locations (F000 segment): main comparison AAE3–AB44; threshold constants
AAEE–AAF4 / AAFB–AAFF; backup comparison ABC6–ABFA. The charging-state helper
at AB45 was stubbed out in the comparison test; the test does not validate
charging policy or the firmware's complete periodic service path. The ROM
itself is private input, not included in any distribution.

These are reference profiles, not a runtime BIOS threshold query. The main
profile follows HP's documented configured chemistry flag. The backup profile
is enabled only when the ROM version string is `ROM BIOS Ver 2.14`; an unknown
revision shows an unverified threshold and no backup marker. A patched ROM
retaining that string could have different thresholds. There is no claim to
detect every patched firmware variant.

Warning hysteresis matters: a flag can remain LOW after voltage has risen above
the marker. ESC acknowledgement affects messages separately. Displays use the
actual cached main/backup/very-low flags and never replace them with a voltage
comparison. No assumption is made that ELKS runs the complete DOS BIOS warning
or shutdown path. Cache sample age remains unknown; R rereads the cache rather
than forcing an ADC measurement. Markers do not modify any hardware settings.
