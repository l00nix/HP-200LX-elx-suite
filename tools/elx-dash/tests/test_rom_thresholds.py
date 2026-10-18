#!/usr/bin/env python3
"""Check ROM 2.14 comparison paths against a private ROM (never bundled).
The NiCad charging-state helper is skipped; its charging policy is not tested.
"""
import sys
from pathlib import Path
from unicorn import Uc,UC_ARCH_X86,UC_MODE_16,UC_HOOK_CODE
from unicorn.x86_const import *
rom=Path(sys.argv[1]).read_bytes()
assert len(rom)==65536 and rom[0xff36:].startswith(b'ROM BIOS Ver 2.14\0')
def run(backup,raw,flags,flags2):
 u=Uc(UC_ARCH_X86,UC_MODE_16);u.mem_map(0,0x100000);u.mem_write(0xf0000,rom)
 u.mem_write(0x4d4,bytes([flags,flags2]));u.reg_write(UC_X86_REG_CS,0xf000)
 u.reg_write(UC_X86_REG_DS,0x40);u.reg_write(UC_X86_REG_SS,0x4000);u.reg_write(UC_X86_REG_SP,0x8000)
 u.reg_write(UC_X86_REG_AX,raw)
 def hook(u,address,size,data):
  if address==0xfab45: # mock return from charge-state helper
   sp=u.reg_read(UC_X86_REG_SP);ip=int.from_bytes(u.mem_read(0x40000+sp,2),'little')
   u.reg_write(UC_X86_REG_SP,sp+2);u.reg_write(UC_X86_REG_IP,ip)
 u.hook_add(UC_HOOK_CODE,hook)
 u.emu_start(0xfabc6 if backup else 0xfaae3,0xfabfa if backup else 0xfab43,count=500)
 return tuple(u.mem_read(0x4d4,2))
for typ,threshold in ((0,23),(0x80,27)):
 for raw in range(64):
  f,g=run(False,raw,typ,0)
  assert bool(f&1)==(raw<threshold),(typ,raw,f)
  assert bool(g&0x40)==(raw<15),(typ,raw,g)
  f,g=run(False,raw,typ|1,0x40)
  assert bool(f&1)==(raw<32)
for raw in range(64):
 f,g=run(True,raw,0,0);assert bool(f&2)==(raw<41)
 f,g=run(True,raw,2,2);assert bool(f&2)==(raw<48)
print('PASS: actual ROM 2.14 low/critical comparisons and warning hysteresis, all 64 ADC codes')
