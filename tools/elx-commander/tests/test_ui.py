#!/usr/bin/env python3
"""PTY integration tests against the real application (optionally through elksemu)."""
import os,pty,fcntl,termios,struct,subprocess,select,time,tempfile,sys,signal
from pathlib import Path
command=[str(Path(x).resolve()) for x in sys.argv[1:]]
class UI:
 def __init__(self,left,right,env=None):
  self.master,self.slave=pty.openpty()
  fcntl.ioctl(self.slave,termios.TIOCSWINSZ,struct.pack('HHHH',25,80,0,0))
  self.saved=termios.tcgetattr(self.slave)
  def session():
   os.setsid();fcntl.ioctl(0,termios.TIOCSCTTY,0)
  self.p=subprocess.Popen([sys.executable,str(Path(__file__).with_name('pty_child.py'))]+command+[str(left),str(right)],stdin=self.slave,stdout=self.slave,stderr=self.slave,preexec_fn=session,env={**os.environ,**(env or {})})
  self.data=b'';self.mark=0
 def wait(self,text,timeout=6):
  text=text.encode();end=time.monotonic()+timeout
  while time.monotonic()<end:
   if text in self.data[self.mark:]:
    self.mark=len(self.data);return
   if select.select([self.master],[],[],.05)[0]:
    try:self.data+=os.read(self.master,65536)
    except OSError:break
  raise AssertionError((text,self.p.poll(),self.data[-2000:]))
 def send(self,s):
  self.mark=len(self.data);os.write(self.master,s.encode())
 def finish(self):
  try:
   end=time.monotonic()+4
   while self.p.poll() is None and time.monotonic()<end:
    if select.select([self.master],[],[],.05)[0]:self.data+=os.read(self.master,65536)
   self.p.wait(timeout=.1)
   assert self.p.returncode==0,self.p.returncode
   # The session-holder asserts restoration before the PTY is hung up.
  finally:
   if self.p.poll() is None:self.p.kill();self.p.wait()
   os.close(self.master);os.close(self.slave)
with tempfile.TemporaryDirectory(prefix='elxcmd-ui-') as d:
 base=Path(d);left=base/'left';right=base/'right';left.mkdir();right.mkdir()
 content=('UNIQUE VIEWER TEXT\n'+'line\n'*50).encode();(left/'alpha.txt').write_bytes(content)
 ui=UI(left,right)
 try:
  ui.wait('Ready.');ui.send('\x1b[B');ui.wait('F10/Q Quit')
  ui.send('\x1bOR');ui.wait('UNIQUE VIEWER TEXT');ui.send(' ');ui.wait('Page 2');ui.send('q');ui.wait('F10/Q Quit')
  ui.send('c');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('n');ui.wait('F10/Q Quit');assert not (right/'alpha.txt').exists()
  ui.send('\x1b[15~');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('y');ui.wait('File copied.');assert (right/'alpha.txt').read_bytes()==content
  ui.send('c');ui.wait('destination filename');ui.send('\r');ui.wait('Copy file?');ui.send('y');ui.wait('Copy failed');assert (right/'alpha.txt').read_bytes()==content
  ui.send('m');ui.wait('destination filename');ui.send('\x7f'*9+'moved.txt\r');ui.wait('Move file');ui.send('y');ui.wait('File moved.');assert not (left/'alpha.txt').exists() and (right/'moved.txt').read_bytes()==content
  ui.send('n');ui.wait('Create folder');ui.send('newdir\r');ui.wait('Folder created.');assert (left/'newdir').is_dir()
  ui.send('\x1b[F');ui.wait('F10/Q Quit');ui.send('\r');ui.wait('/newdir');ui.send('\x7f');ui.wait('F10/Q Quit')
  ui.send('\x1b[F');ui.wait('F10/Q Quit');ui.send('d');ui.wait('Permanently delete');ui.send('n');ui.wait('F10/Q Quit');assert (left/'newdir').exists()
  ui.send('d');ui.wait('Permanently delete');ui.send('y');ui.wait('Deleted.');assert not (left/'newdir').exists()
  ui.send('\t');ui.wait('F10/Q Quit');ui.send('\x1b[F');ui.wait('F10/Q Quit')
  ui.send('p');ui.wait('Directory path');ui.send('\x7f'*len(str(right))+'/nonexistent-elxcmd\r');ui.wait('Directory:');assert (right/'moved.txt').exists()
  ui.send('?');ui.wait('keyboard guide');ui.send(' ');ui.wait('F10/Q Quit')
  ui.send('q');ui.finish()
 except BaseException:
  if ui.p.poll() is None:ui.p.kill();ui.p.wait()
  raise
 # Signal-driven cleanup during input.
 ui=UI(left,right);ui.wait('Ready.');ui.p.send_signal(signal.SIGTERM);ui.finish()
print('PASS: real UI navigation, viewer pagination, cancel/confirm copy and deletion, overwrite refusal, move, mkdir, path errors, help, quit and terminal restoration')
