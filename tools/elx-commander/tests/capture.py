#!/usr/bin/env python3
"""Capture actual program output against clearly artificial sample directories."""
import sys,os,pty,termios,fcntl,struct,subprocess,select,time
from pathlib import Path
cmd=[str(Path(x).resolve()) for x in sys.argv[1:]]
root=Path('build-elks/demo').resolve();left=root/'work';right=root/'backup'
left.mkdir(parents=True,exist_ok=True);right.mkdir(parents=True,exist_ok=True)
(left/'docs').mkdir(exist_ok=True)
(left/'README.TXT').write_text('ELX Commander sample directory.\nNo real user files.\n')
(left/'battery.csv').write_text('main_mV,backup_mV\n2640,3135\n')
(left/'notes.txt').write_text('Try Tab, F3 to view, F5 to copy.\n')
(right/'archive').mkdir(exist_ok=True)
(right/'README.TXT').write_text('Sample backup destination.\n')
m,s=pty.openpty();fcntl.ioctl(s,termios.TIOCSWINSZ,struct.pack('HHHH',25,80,0,0))
def session():
 os.setsid();fcntl.ioctl(0,termios.TIOCSCTTY,0)
p=subprocess.Popen(cmd+[str(left),str(right)],stdin=s,stdout=s,stderr=s,preexec_fn=session)
data=b''
def until(token):
 global data
 end=time.monotonic()+5
 while time.monotonic()<end:
  if select.select([m],[],[],.05)[0]:data+=os.read(m,65536)
  if token in data:return
 raise RuntimeError(data[-1000:])
until(b'F10/Q Quit');os.write(m,b'\x1b[B\x1b[B');time.sleep(.1)
end=time.monotonic()+.3
while time.monotonic()<end:
 if select.select([m],[],[],.05)[0]:data+=os.read(m,65536)
Path('build-elks/preview.ansi').write_bytes(data)
os.write(m,b'q');end=time.monotonic()+3
while p.poll() is None and time.monotonic()<end:
 if select.select([m],[],[],.05)[0]:os.read(m,65536)
if p.poll() is None:p.kill();p.wait();raise RuntimeError('exit timeout')
os.close(m);os.close(s);assert p.returncode==0
print('Captured native application terminal output in build-elks/preview.ansi')
