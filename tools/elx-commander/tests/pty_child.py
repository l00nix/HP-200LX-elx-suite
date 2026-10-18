#!/usr/bin/env python3
"""Keep a controlling-terminal session alive while checking child cleanup."""
import subprocess,sys,termios,signal
saved=termios.tcgetattr(0)
p=subprocess.Popen(sys.argv[1:])
def forward(sig,frame):
 if p.poll() is None:p.send_signal(sig)
for sig in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP):signal.signal(sig,forward)
rc=p.wait()
assert termios.tcgetattr(0)==saved,'child did not restore its terminal settings'
sys.exit(rc)
