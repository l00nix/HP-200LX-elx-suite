from pathlib import Path
import subprocess,sys,tempfile
exe=str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory() as td:
 subprocess.run([exe],cwd=td,check=True)
 p=Path(td);frames=sorted(p.glob('update-*.pbm'))
 for f in frames:assert f.read_bytes()==f.with_name(f.name.replace('update-','full-')).read_bytes(),f.name
 a=frames[0].read_bytes();b=frames[1].read_bytes();offset=a.index(b'200\n')+4
 for y in range(200):
  for x in range(80):
   if not(42<=y<179 and x<40):assert a[offset+y*80+x]==b[offset+y*80+x]
 print('PASS: 10 incremental ELXbat frames equal full renders; main update leaves other regions unchanged')
