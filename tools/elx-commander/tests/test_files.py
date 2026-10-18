#!/usr/bin/env python3
"""Run real operations in isolated temporary directories, host or elksemu."""
import os,subprocess,tempfile,sys
from pathlib import Path
command=sys.argv[1:]
def run(*args,ok=True):
 p=subprocess.run(command+list(map(str,args)),capture_output=True,text=True,timeout=20)
 assert (p.returncode==0)==ok,(args,p.returncode,p.stdout,p.stderr)
 return p.stdout
with tempfile.TemporaryDirectory(prefix='elxcmd-') as d:
 p=Path(d);a=p/'source';b=p/'target';data=bytes(range(256))*37+b'\x00\xffend'
 a.write_bytes(data)
 run('copy',a,b);assert a.read_bytes()==b.read_bytes()==data
 b.write_bytes(b'keep');run('copy',a,b,ok=False);assert b.read_bytes()==b'keep'
 run('move',a,b,ok=False);assert a.read_bytes()==data and b.read_bytes()==b'keep'
 run('copy',a,a,ok=False);assert a.read_bytes()==data
 run('cancel',a,p/'cancelled',ok=False);assert not (p/'cancelled').exists()
 os.symlink(a,p/'link');run('copy',p/'link',p/'linkcopy',ok=False)
 run('copy',a,p/'link',ok=False);assert a.read_bytes()==data
 run('copy',p,p/'dircopy',ok=False)
 os.mkfifo(p/'fifo');run('copy',p/'fifo',p/'fifo-copy',ok=False)
 run('delete',p/'fifo',ok=False)
 run('move',a,p/'moved');assert not a.exists() and (p/'moved').read_bytes()==data
 run('delete',p/'link');assert not (p/'link').is_symlink() and (p/'moved').exists()
 run('delete',p,ok=False);assert (p/'moved').read_bytes()==data
 (p/'empty').mkdir();run('delete',p/'empty');assert not (p/'empty').exists()
 run('delete',p/'moved');assert not (p/'moved').exists()
 for i in range(135):(p/('f%03d'%i)).touch()
 listing=run('list',p);assert 'limited=1' in listing and 'count=128' in listing
 run('list',p/'absent',ok=False)
 for seq,key in [('\x1b[A',256),('\x1b[B',257),('\x1b1',280),('\x1b0',289),('\x1bOP',280),('\x1b[15~',284),('\x1b[[A',280)]:
  assert int(run('key',seq))==key
print('PASS: binary copy, overwrite/self-copy refusal, cancellation cleanup, move, symlink/device handling, empty-only rmdir, directory limits and key mappings')
