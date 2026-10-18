#!/usr/bin/env python3
"""Verify real framebuffer transitions, bolt semantics and partial redraw boundaries."""
from pathlib import Path
import subprocess,sys,tempfile
exe=[str(Path(p).resolve()) for p in sys.argv[1:]]
with tempfile.TemporaryDirectory() as tmp:
 subprocess.run(exe,cwd=tmp,check=True,timeout=20)
 for tool,top,bottom,icon_y in [('bat',21,40,23),('dash',14,26,14)]:
  frames=[(Path(tmp)/f'{tool}-{i}.pbm').read_bytes() for i in range(7)]
  assert all(b.startswith(b'P4\n640 200\n') and len(b)==16011 for b in frames)
  frames=[b[11:] for b in frames]
  for b in frames[1:]:
   assert b[:80*top]==frames[0][:80*top] and b[80*bottom:]==frames[0][80*bottom:]
  def crop(b,x,y,w,h):
   return tuple(bool(b[yy*80+xx//8] & (0x80>>(xx%8))) for yy in range(y,y+h) for xx in range(x,x+w))
  icons=[crop(b,8,icon_y,40,12) for b in frames]
  assert icons[0]!=icons[1] and icons[1]==icons[2] # plugged != battery; enabled alone is not charging
  assert icons[3]==icons[4] and icons[3]!=icons[1] # normal/fast charging bolt
  assert icons[5]==icons[0] # unplug removes all bolt/plug pixels
  assert icons[6] not in icons[:6] # stale becomes question mark
  assert crop(frames[1],39,icon_y,9,12)==(False,)*108
  assert any(crop(frames[3],39,icon_y,9,12))
 print('PASS: plug, battery, charging-only bolt, stale, recovery and isolated partial redraw')
