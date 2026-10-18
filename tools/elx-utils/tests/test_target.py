#!/usr/bin/env python3
import subprocess,sys,os,hashlib
from pathlib import Path
emu=sys.argv[1]
subprocess.run([sys.executable,'tests/make_fixtures.py'],check=True)
img='tests/fixtures/dos-reference.img'
def run(binary,args=(),code=0,env=None):
 p=subprocess.run([emu,binary,*args],capture_output=True,text=True,env={**os.environ,**(env or {})},timeout=15)
 assert p.returncode==code,(binary,args,p.returncode,p.stdout,p.stderr)
 return p.stdout+p.stderr
reference=run('build-elks/elxstat',['-v','--image',img])
assert '2.558 V' in reference and '2.860 V' in reference and '14.850' in reference
Path('build-tests/target-elxstat.txt').write_text(reference)
assert 'OFFLINE IMAGE' in run('build-elks/elxbat',['--text','--image',img])
assert '0.1' in run('build-elks/elxbat',['--version'])
for bin in ('build-elks/elxstat','build-elks/elxbat'):
 run(bin,['--help'])
 run(bin,['--bad'],2)
 for name in ('busy','wrong-machine','short'):
  run(bin,['--image','tests/fixtures/'+name+'.img'],1)
for name in ('dos-reference','invalid','below-scale','maximum'):
 run('build-tests/elxbat-render',['--image','tests/fixtures/'+name+'.img','--preview','build-tests/'+name+'.pbm'])
 assert Path('build-tests/'+name+'.pbm').stat().st_size==16011
print('PASS: real 16-bit ELXstat/ELXbat text and failures; shared CGA renderer handles reference, invalid, below-scale and maximum readings')
