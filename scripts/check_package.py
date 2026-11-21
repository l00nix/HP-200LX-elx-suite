#!/usr/bin/env python3
"""Check package ownership and FAT-compatible installed names."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
s=(root/'integration/elxtools.pkg').read_text()
files=re.search(r'^FILES="([^"]+)"',s,re.M|re.S).group(1).split()
expected={'/bin/elxstat','/bin/elxbat','/bin/elxctl','/bin/elxdash',
          '/lib/elxtools/copying','/lib/elxtools/font.lic','/lib/elxtools/usage.txt'}
assert {p.split(':')[1] for p in files}==expected
assert len(files)==len(expected)
for pair in files:
 src,dest=pair.split(':');assert not dest.startswith('/etc/')
 for c in dest.strip('/').split('/'):
  assert re.fullmatch(r'[A-Za-z0-9_+-]{1,8}(?:\.[A-Za-z0-9]{1,3})?',c),dest
 if not src.startswith('build/'):assert (root/src).is_file(),src
for d in ('tools/elx-cap','tools/elx-commander'):assert not (root/d).exists(),d
print('PASS four HP tools only; seven package paths; 8.3 names; no startup/kernel files')
