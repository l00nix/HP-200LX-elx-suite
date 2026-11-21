# Contributing

Keep changes compatible with the 8086 and the HP 200LX's memory limits. Keep
ELKS hardware access separate from host simulation and drawing/control logic.
Use the tool's existing test and build instructions; record physical testing
separately from mocked or emulator checks.

Hardware writes should use documented interfaces, validate inputs first,
preserve unrelated state and report failed read-back. Do not add charging,
serial-power or power-off writes as incidental UI changes.

For a bug report include tool version, OS/kernel, ROM revision, exact command
or keys, expected/actual result, and a screenshot where useful. Do not commit private ROMs, memory dumps, credentials,
CF images or unrelated project files.

Keyboard icons in `docs/assets/keys/` are local SVGs with accessible text labels.
Use them in user-facing guides when describing keys. A plus sign between icons
means a chord; the word “then” means sequential keys.
