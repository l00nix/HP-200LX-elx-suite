#!/usr/bin/env python3
"""Synthetic physical-memory snapshots, never hardware captures or ROM copies."""
from pathlib import Path
import struct
out=Path(__file__).resolve().parent/'fixtures'
out.mkdir(exist_ok=True)
b=bytearray(1024*1024)
def word(off,n): struct.pack_into('<H',b,off,n)
# Synthetic HP service identity return sequence: HP/0102/model0201/IRET.
b[0xfd88e:0xfd898]=bytes.fromhex('bb5048b90201ba0102cf')
b[0xfff36:0xfff36+17]=b'ROM BIOS Ver 2.14'
b[0xfec5c:0xfec5c+15]=b'Version 1.02 A '
b[0xffff5:0xffffd]=b'05/24/94'
# The DOS attachment's voltages correspond to adjusted codes 36 and 47.
b[0x49a:0x49d]=bytes([36,47,32])
b[0x4d4]=0x82 # configured NiCad, external, backup low, charging disabled
word(0x4d0,270); word(0x4d2,100)
word(0x400,0x3f8); word(0x44a,80)
b[0x449]=3;b[0x49f]=0x85;b[0x484]=24;b[0x4eb]=0x40
word(0x4a3,3273)
b[0x4fc]=18;b[0x4fd]=4
# Slices above intentionally use equal-length replacement; catch malformed image.
assert len(b)==1024*1024
(out/'dos-reference.img').write_bytes(b)
for name,offset,val in [('invalid',0x49a,255),('busy',0x4cd,8),('wrong-machine',0xfd88f,0),('below-scale',0x49a,0),('maximum',0x49a,63),('disabled-poll',0x4d0,0)]:
 c=bytearray(b);c[offset]=val
 if name=='disabled-poll':c[0x4d1]=0
 (out/(name+'.img')).write_bytes(c)
(out/'short.img').write_bytes(b[:4096])
