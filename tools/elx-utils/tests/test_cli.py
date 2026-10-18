#!/usr/bin/env python3
import hashlib,subprocess,sys
from pathlib import Path
root=Path(__file__).resolve().parent.parent
build=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else root/'build-host'
fixtures=root/'tests/fixtures'
subprocess.run([sys.executable,str(root/'tests/make_fixtures.py')],check=True)
def run(prog,*args,code=0):
 p=subprocess.run([str(build/prog),*map(str,args)],capture_output=True,text=True,timeout=10)
 assert p.returncode==code,(prog,args,p.returncode,p.stdout,p.stderr)
 return p.stdout+p.stderr
img=fixtures/'dos-reference.img'
before=hashlib.sha256(img.read_bytes()).hexdigest()
s=run('elxstat','-v','--image',img)
assert '2.558 V' in s and '2.860 V' in s and 'backup=LOW' in s
assert '14.850 seconds' in s and 'OFFLINE IMAGE' in s and 'sample age unknown' in s
assert '0x0201' in s and 'ROM BIOS Ver 2.14' in s
for section in ('battery','power','display','serial','system'):run('elxstat',section,'--image',img)
for program in ('elxstat','elxbat'):
 assert '0.1' in run(program,'--version')
 run(program,'--help')
 run(program,'--does-not-exist',code=2)
 for invalid in ('wrong-machine','short','busy'):
  run(program,'--image',fixtures/(invalid+'.img'),code=1)
run('elxbat','--interval','0',code=2)
run('elxbat','--interval','99999999999999999999999',code=2)
run('elxbat','--text','--graphics',code=2)
assert 'unavailable' in run('elxstat','--image',fixtures/'invalid.img')
assert '?' in run('elxbat','--text','--image',fixtures/'invalid.img')
assert 'below scale' in run('elxbat','--text','--image',fixtures/'below-scale.img')
assert '(disabled)' in run('elxstat','--image',fixtures/'disabled-poll.img')
assert hashlib.sha256(img.read_bytes()).hexdigest()==before
(build/'reference-output.txt').write_text(s)
print('PASS: CLI sections, offline labelling, reference voltages, invalid/busy/truncated data, bounds, read-only image')
