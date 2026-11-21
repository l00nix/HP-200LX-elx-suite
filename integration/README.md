# EPT integration

Package name: `elxtools` (eight characters). The [recipe](elxtools.pkg) builds
only the four HP utilities via the root `build.sh`, then installs seven explicit
paths. It preserves the existing tool versions, includes both project and font
licenses, and does not modify the kernel, startup configuration or firmware.

The source repository is public. The verified release payload is staged for
the 8088retro EPT catalogue, with public recipe build validation in progress. The upstream EPT
recipe submission is separate; acceptance does not happen automatically when a
GitHub release is published. Publication links are recorded here after verification.

The recipe expects a configured ELKS source tree and its matching IA16 toolchain.
`ELKS` is supplied by EPT. The native build runs an i8086 instruction and near-memory
audit for every executable. This is an HP-specific package, not a generic PC suite.

To test staging manually:

```sh
ELKS=/path/to/elks sh build.sh
make install DESTDIR=/path/to/empty/staging-root
python3 scripts/check_package.py
```
