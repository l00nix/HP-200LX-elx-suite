#!/usr/bin/env python3
"""Run the real detached supervisor/lock/IPC against a fake BIOS, never hardware."""
import os,subprocess,tempfile,time
from pathlib import Path
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as temp:
 session=Path(temp)/'session';exe=Path(temp)/'service'
 subprocess.run(['cc','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',
  '-DDASH_SERVICE_TEST','-Dgeteuid=test_uid','-Dtime=test_time','-Dsleep=test_sleep',
  f'-DSESSION="{session}"','-Isrc','-I../elx-utils/src','-I../elx-ctl/src',
  'tests/service_fixture.c','src/charge_elks.c','src/charge.c','-o',str(exe)],cwd=root,check=True)
 for case,expected in [('locked',None),('existing','Existing charging'),('stop','Stopped by user'),
                       ('duplicate','Stopped by user'),('unplug','External power'),('fault','overvoltage'),
                       ('read','Invalid fresh'),('voltage','Voltage limit'),('veto','Firmware disabled'),
                       ('handshake','Dashboard handshake lost'),('normal','Time limit'),('parent-exit','Time limit')]:
  for suffix in ['-last','-trace']:Path(str(session)+suffix).unlink(missing_ok=True)
  result=subprocess.run([exe,case],timeout=8,capture_output=True)
  assert result.returncode==(-9 if case=='handshake' else 0),(case,result.returncode,result.stderr)
  for _ in range(200):
   if not session.exists():break
   time.sleep(.01)
  assert not session.exists(),case
  tracepath=Path(str(session)+'-trace')
  trace=tracepath.read_text().splitlines() if tracepath.exists() else []
  if case in ('locked','existing'):
   assert not trace,(case,trace) # refused before any BIOS write
  else:
   assert trace.count('charge 6004 0001')==1,(case,trace)
   assert trace[-3:]==['charge 6004 0000']*3,(case,trace) # wait for hardware-off
  if expected:assert expected in Path(str(session)+'-last').read_text(),case
 print('PASS: detached supervisor outlives parent, locks, stop IPC, faults, read failures, verified shutdown; mock BIOS only')
