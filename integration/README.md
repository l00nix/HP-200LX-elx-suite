# EPT integration

Package name: `elxtools` (eight characters). The [recipe](elxtools.pkg) builds
only the four HP utilities via the root `build.sh`, then installs seven explicit
paths. It preserves the existing tool versions, includes both project and font
licenses, and does not modify the kernel, startup configuration or firmware.

The source repository is public. Package **0.2.0** is published in the
[8088retro EPT catalogue](http://ept.8088retro.com). The seven payload files,
source archive and catalogue were verified byte-for-byte over HTTP.

[Upstream EPT PR #13](https://github.com/sammothxc/ept/pull/13) adds the same
recipe to `sammothxc/ept`; it is submitted and awaiting review. The upstream
builder was tested against an anonymous clone of both `v0.2.0` and `main`, using
a configured ELKS tree and matching IA16 toolchain. Both builds produced the
same executable hashes as the published package. Upstream acceptance is separate
from availability on 8088retro.

The recipe expects a configured ELKS source tree and its matching IA16 toolchain.
`ELKS` is supplied by EPT. The native build runs an i8086 instruction and near-memory
audit for every executable. This is an HP-specific package, not a generic PC suite.

To test staging manually:

```sh
ELKS=/path/to/elks sh build.sh
make install DESTDIR=/path/to/empty/staging-root
python3 scripts/check_package.py
```
