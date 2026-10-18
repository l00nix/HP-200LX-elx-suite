#!/usr/bin/env python3
import subprocess,sys
from pathlib import Path
cmd=[str(Path(p).resolve()) for p in sys.argv[1:]]
def run(args,code=0):
    p=subprocess.run(cmd+args,capture_output=True,text=True,timeout=10)
    assert p.returncode==code,(args,p.returncode,p.stdout,p.stderr)
    return p.stdout+p.stderr
assert 'ELXctl 0.1' in run(['--version'])
assert 'firmware-timeout' in run(['--help'])
out=run(['--simulate']);assert 'SIMULATION' in out and 'Contrast' in out and 'not configure ELKS' in out
for args,call,value in [
 (['contrast','0'],'INT 15 AX=6200 BX=0000','0 / 31'),
 (['contrast','31'],'INT 15 AX=6200 BX=001F','31 / 31'),
 (['volume','0'],'INT 15 AX=4800 BX=0000','0 (off)'),
 (['volume','3'],'INT 15 AX=4803 BX=0000','3 (loud)'),
 (['zoom','80x25'],'INT 10 AX=D003 BX=0000','80x25'),
 (['zoom','64x18'],'INT 10 AX=D081 BX=0000','64x18'),
 (['zoom','40x25'],'INT 10 AX=D083 BX=0000','40x25'),
 (['zoom','40x16'],'INT 10 AX=D085 BX=0000','40x16'),
 (['annunciators','left'],'INT 15 AX=6120 BX=0000','left'),
 (['annunciators','right'],'INT 15 AX=6160 BX=0000','right'),
 (['annunciators','off'],'INT 15 AX=61E0 BX=0000','off'),
 (['firmware-timeout','off'],'INT 15 AX=4600 BX=0000','disabled'),
 (['firmware-timeout','1'],'INT 15 AX=4600 BX=0013','1.045 seconds'),
 (['firmware-timeout','180'],'INT 15 AX=4600 BX=0CC9','180.015 seconds'),
]:
    out=run(['--simulate']+args);assert call in out and value in out,out
    out=run(['--simulate','--dry-run']+args);assert 'Would call '+call in out and 'SIMULATED INT' not in out,out
for command in ['contrast','volume','zoom','annunciators','firmware-timeout']:
    assert 'SIMULATED INT' not in run(['--simulate',command])
for args in [['contrast','-1'],['contrast','32'],['volume','4'],['volume','1.5'],['contrast',''],['contrast','0x10'],['contrast','9999999999999999999'],['zoom','81'],['zoom','800x600'],['annunciators','on'],['firmware-timeout','3605'],['firmware-timeout','-1'],['status','0'],['contrast','15','volume','2'],['serial','off'],['charge','on'],['display','off'],['--unknown'],['--help','contrast','0']]:
    out=run(['--simulate']+args,2);assert 'SIMULATED INT' not in out
print('PASS CLI getters/setters, dry-run, simulation, exact BIOS arguments and rejection before hardware access')
