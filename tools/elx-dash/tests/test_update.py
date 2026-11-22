#!/usr/bin/env python3
"""Incremental frames must equal clean full renders, without unrelated damage."""
from pathlib import Path
import subprocess,sys,tempfile
exe=[str(Path(p).resolve()) for p in sys.argv[1:]]
with tempfile.TemporaryDirectory() as tmp:
 subprocess.run(exe,cwd=tmp,check=True,timeout=30)
 frames=sorted(Path(tmp).glob('update-*.pbm'))
 for p in frames:
  assert p.read_bytes()==p.with_name(p.name.replace('update-','full-')).read_bytes(),p.name
 # Main voltage update must leave all pixels outside that gauge alone.
 a=frames[0].read_bytes()[11:];b=frames[1].read_bytes()[11:]
 for y in range(200):
  for x in range(80):
   if not (38<=y<110 and x<40):assert a[y*80+x]==b[y*80+x]
 print(f'PASS: {len(frames)} incremental frames equal full reference renders; isolated gauge update')
