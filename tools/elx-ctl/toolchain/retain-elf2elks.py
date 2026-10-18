#!/usr/bin/env python3
"""Preserve ELF before conversion; adapted from ELKSWeb toolchain helper (GPL-2.0-or-later)."""
import os,pathlib,shutil,sys
for argument in sys.argv[1:]:
 p=pathlib.Path(argument)
 if p.is_file():
  with p.open('rb') as f: magic=f.read(4)
  if magic==b'\x7fELF':shutil.copyfile(p,str(p)+'.elf');break
real=os.environ['ELX_REAL_ELF2ELKS']
os.execv(real,[real]+sys.argv[1:])
