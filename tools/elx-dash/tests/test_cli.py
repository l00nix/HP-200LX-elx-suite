#!/usr/bin/env python3
import subprocess,sys,tempfile
from pathlib import Path
exe=[str(Path(p).resolve()) for p in sys.argv[1:]]
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp)/'preview.pbm'
 def run(*a,code=0):
  r=subprocess.run([*exe,*a],capture_output=True,text=True,timeout=10)
  assert r.returncode==code,(a,r.returncode,r.stdout,r.stderr)
  return r.stdout
 assert '0.1' in run('--version')
 assert 'Contrast' in run('--help')
 run('--bad',code=2);run(code=2)
 out=run('--demo','--preview',str(p))
 base=p.read_bytes();assert base.startswith(b'P4\n640 200\n') and len(base)==16011
 assert 'contrast=16 volume=2' in out
 out=run('--demo','--preview',str(p),'--script','++\t-\t+\t+\t-')
 assert 'contrast=18 volume=1 zoom=1 pending=1 icons=192 ticks=2728' in out,out
 assert p.read_bytes()!=base
 out=run('--demo','--preview',str(p),'--script','q++++')
 assert 'contrast=16' in out
 print('PASS: CLI validation, actual framebuffer, scripted selection/adjustment, exit handling')
