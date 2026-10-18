#!/usr/bin/env python3
"""Execute the released MZ EXE using Unicorn with mocked DOS/HP services.
This validates compiled control flow, BIOS arguments, CGA writes and cleanup;
it is not a real DOS or HP firmware compatibility test.
"""
import sys,struct
from pathlib import Path
from unicorn import *
from unicorn.x86_const import *
exe=Path(sys.argv[1]).read_bytes()
def run(args='',keys=b'q',hp=True,expect=0):
 u=Uc(UC_ARCH_X86,UC_MODE_16);u.mem_map(0,0x200000)
 w=lambda o:struct.unpack_from('<H',exe,o)[0]
 assert exe[:2]==b'MZ'
 seg=0x1010;psp=0x1000;hdr=w(8)*16;data=bytearray(exe[hdr:])
 for i in range(w(6)):
  off,sg=struct.unpack_from('<HH',exe,w(0x18)+4*i);p=sg*16+off
  struct.pack_into('<H',data,p,(struct.unpack_from('<H',data,p)[0]+seg)&65535)
 u.mem_write(seg*16,bytes(data));u.mem_write(psp*16,b'\xcd\x20')
 def mw(addr,val):u.mem_write(addr,struct.pack('<H',val&65535))
 mw(psp*16+2,0x9000);mw(psp*16+0x2c,0x900)
 u.mem_write(0x9000,b'PATH=C:\\\0\0\x01\0C:\\LXDASH.EXE\0')
 cmd=(' '+args).encode() if args else b''
 u.mem_write(psp*16+0x80,bytes([len(cmd)])+cmd+b'\r')
 for r,v in [(UC_X86_REG_CS,seg+w(0x16)),(UC_X86_REG_IP,w(0x14)),(UC_X86_REG_SS,seg+w(0xe)),(UC_X86_REG_SP,w(0x10)),(UC_X86_REG_DS,psp),(UC_X86_REG_ES,psp),(UC_X86_REG_EFLAGS,0x202)]:u.reg_write(r,v)
 b=bytearray(256);b[0x49]=3;b[0x4a]=80;b[0x84]=24;b[0x9f]=0x85;b[0x9a]=39;b[0x9b]=57;b[0xd4]=0x40;b[0xeb]=0x40
 struct.pack_into('<H',b,0xa3,3273);u.mem_write(0x400,bytes(b))
 if hp:
  u.mem_write(0xfd88e,bytes.fromhex('bb5048b90201ba0102cf'))
  u.mem_write(0xfff36,b'ROM BIOS Ver 2.14\0')
 text=bytes([65,7])*2000;u.mem_write(0xb8000,text)
 state={'exit':None,'out':b'','keys':list(keys),'contrast':16,'volume':2,'index':0x5a,'writes':[], 'frames':[], 'tick':0,'mode':3,'cursor':0x0304,'shape':0x0607}
 def reg(r):return u.reg_read(r)
 def setr(r,v):u.reg_write(r,v)
 def carry(v):setr(UC_X86_REG_EFLAGS,(reg(UC_X86_REG_EFLAGS)&~1)|bool(v))
 def intcall(u,n,user):
  ax=reg(UC_X86_REG_AX);ah=ax>>8;al=ax&255;bx=reg(UC_X86_REG_BX);cx=reg(UC_X86_REG_CX);dx=reg(UC_X86_REG_DX)
  if n==0x21:
   carry(0)
   if ah==0x30:setr(UC_X86_REG_AX,5);setr(UC_X86_REG_BX,0);setr(UC_X86_REG_CX,0)
   elif ah==0x4a:pass
   elif ah==0x4c:state['exit']=al;u.emu_stop()
   elif ah==0x40:
    state['out']+=bytes(u.mem_read(reg(UC_X86_REG_DS)*16+dx,cx));setr(UC_X86_REG_AX,cx)
   elif ah==0x09:
    addr=reg(UC_X86_REG_DS)*16+dx
    for j in range(4096):
     ch=bytes(u.mem_read(addr+j,1))
     if ch==b'$':break
     state['out']+=ch
   elif ah==0x44:setr(UC_X86_REG_DX,0x80d3) # character device
   elif ah==0x2a:setr(UC_X86_REG_CX,2026);setr(UC_X86_REG_DX,0x0a12);setr(UC_X86_REG_AX,0)
   elif ah==0x2c:
    state['tick']+=1;t=state['tick'];setr(UC_X86_REG_CX,0x0c00+(t//6000)%60);setr(UC_X86_REG_DX,((t//100)%60)*256+t%100)
   elif ah==0x0b:setr(UC_X86_REG_AX,255 if state['keys'] else 0)
   elif ah in (7,8):
    assert state['keys'],'no scripted keys remaining';setr(UC_X86_REG_AX,state['keys'].pop(0))
   elif ah==0x2f:setr(UC_X86_REG_ES,psp);setr(UC_X86_REG_BX,0x80)
   elif ah==0x35:setr(UC_X86_REG_ES,0);setr(UC_X86_REG_BX,0)
   elif ah in (0x25,0x1a):pass
   elif ah==0x48:setr(UC_X86_REG_AX,0x6000)
   elif ah==0x49:pass
   elif ax==0x6300:setr(UC_X86_REG_DS,0);setr(UC_X86_REG_SI,0x8000);setr(UC_X86_REG_AX,0)
   else:raise AssertionError(f'Unhandled DOS AX={ax:04x}')
  elif n==0x10:
   if ah==0x0f:setr(UC_X86_REG_AX,0x5000|state['mode']);setr(UC_X86_REG_BX,0)
   elif ah==0:
    if state['mode']==6:state['frames'].append(bytes(u.mem_read(0xb8000,16384)))
    state['mode']=al;u.mem_write(0x449,bytes([al]));u.mem_write(0x49f,bytes([al]));u.mem_write(0xb8000,bytes(16384))
   elif ah==3:setr(UC_X86_REG_CX,state['shape']);setr(UC_X86_REG_DX,state['cursor'])
   elif ah==1:state['shape']=cx
   elif ah==2:state['cursor']=dx
   elif ah==0xd0:
    assert state['mode'] in (2,3,7);u.mem_write(0x49f,bytes([al]));state['writes'].append(('zoom',al))
   else:raise AssertionError(f'Unhandled video AX={ax:04x}')
  elif n==0x15:
   assert hp and '--demo' not in args
   if ax==0x6201:setr(UC_X86_REG_AX,state['contrast'])
   elif ax==0x6200:assert bx<=31;state['contrast']=bx;state['writes'].append(('contrast',bx))
   elif ah==0x48:assert al<=3;state['volume']=al;state['writes'].append(('volume',al))
   elif ah==0x61:
    assert al&0x20;u.mem_write(0x4eb,bytes([(u.mem_read(0x4eb,1)[0]&0x3f)|(al&0xc0)]));state['writes'].append(('icons',al&0xc0))
   elif ah==0x46:mw(0x4a3,bx);state['writes'].append(('timeout',bx))
   else:raise AssertionError(f'Unhandled HP AX={ax:04x}')
  elif n==0x16:
   if ah==1:
    flags=reg(UC_X86_REG_EFLAGS);setr(UC_X86_REG_EFLAGS,(flags&~0x40)|(0 if state['keys'] else 0x40));setr(UC_X86_REG_AX,state['keys'][0] if state['keys'] else 0)
   elif ah==0:assert state['keys'];setr(UC_X86_REG_AX,state['keys'].pop(0))
   else:raise AssertionError(f'Unhandled keyboard {ax:04x}')
  elif n==0x1a:
   state['tick']+=1;setr(UC_X86_REG_CX,0);setr(UC_X86_REG_DX,state['tick']);setr(UC_X86_REG_AX,0)
  else:raise AssertionError(f'Unhandled interrupt {n:02x} AX={ax:04x}')
 def inp(u,port,size,user):
  assert '--demo' not in args and hp and size==1
  if port==0x22:return state['index']
  assert port==0x23 and state['index']==0x1e;return (state['volume']<<6)|0x24
 def outp(u,port,size,value,user):
  assert port==0x22 and size==1 and '--demo' not in args and hp
  state['index']=value
 u.hook_add(UC_HOOK_INTR,intcall)
 u.hook_add(UC_HOOK_INSN,inp,None,1,0,UC_X86_INS_IN)
 u.hook_add(UC_HOOK_INSN,outp,None,1,0,UC_X86_INS_OUT)
 u.emu_start((seg+w(0x16))*16+w(0x14),0,count=100000000)
 assert state['exit']==expect,(args,state['exit'],state['out'],hex(reg(UC_X86_REG_CS)),hex(reg(UC_X86_REG_IP)))
 if state['frames']:
  assert state['mode']==3 and bytes(u.mem_read(0xb8000,4000))==text,'text screen not restored'
  assert state['cursor']==0x0304 and state['shape']==0x0607
  assert state['index']==0x5a
  image=state['frames'][-1];pbm=b'P4\n640 200\n'+b''.join(image[(y&1)*8192+(y//2)*80:(y&1)*8192+(y//2)*80+80] for y in range(200))
  Path(sys.argv[1]).with_suffix('.pbm').write_bytes(pbm)
 return state
if __name__=='__main__':
 assert b'LXdash 0.1' in run('--help')['out']
 assert b'LXdash 0.1' in run('--version')['out']
 run('--bad',expect=2)
 run(hp=False,expect=1)
 s=run('--demo',b'++\t-\t+\t+\t-q',hp=False);assert not s['writes']
 s=run(keys=b'++\t-\t+\t+\t-q')
 assert s['contrast']==18 and s['volume']==1
 assert ('zoom',3) in s['writes'] and ('icons',0xc0) in s['writes'] and ('timeout',2728) in s['writes'],s['writes']
 s=run(keys=b'q');assert s['writes']==[('zoom',0x85)],s['writes']
 s=run(keys=bytes([0,77,0,80,0,75,27]));assert s['contrast']==17 and s['volume']==1
 print('PASS: compiled DOS EXE CLI, HP guard, demo, live controls, CGA framebuffer, text/cursor/zoom restoration')
