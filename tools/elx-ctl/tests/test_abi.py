#!/usr/bin/env python3
"""Execute actual linked 16-bit hardware bridges with mocked BIOS and ports.
Requires Unicorn. Does not claim to emulate the HP firmware itself.
"""
from pathlib import Path
import struct,sys
from unicorn import Uc,UC_ARCH_X86,UC_MODE_16,UC_HOOK_INTR,UC_HOOK_INSN
from unicorn.x86_const import *
raw=Path(sys.argv[1]).read_bytes()
hdr=struct.unpack_from('<16sHHIIIIIHHHHHH',raw)
sections=[struct.unpack_from('<IIIIIIIIII',raw,hdr[6]+i*hdr[11]) for i in range(hdr[12])]
strings=sections[hdr[13]];names=raw[strings[4]:strings[4]+strings[5]]
def name(data,off):return data[off:].split(b'\0',1)[0].decode()
code=None;symbols={}
for h in sections:
    if name(names,h[0])=='.text':code=raw[h[4]:h[4]+h[5]];codeaddr=h[3]
    if h[1]==2:
        st=sections[h[6]];sn=raw[st[4]:st[4]+st[5]]
        for pos in range(h[4],h[4]+h[5],h[9]):
            n,v,size,info,other,idx=struct.unpack_from('<IIIBBH',raw,pos)
            symbols[name(sn,n)]=v
assert code is not None
for function,args,interrupt in [('ctl_int15',[0x6200,17],0x15),('ctl_int10',[0xd083],0x10),('ctl_volume_register',[],None)]:
 for flags in (2,0x202):
    u=Uc(UC_ARCH_X86,UC_MODE_16);u.mem_map(0,1048576);u.mem_write(0x10000,code)
    u.mem_write(0x1f000,b'\xf4')
    saved={UC_X86_REG_BX:0xbeef,UC_X86_REG_BP:0x7777,UC_X86_REG_SI:0x1357,UC_X86_REG_DI:0x2468,UC_X86_REG_DS:0x4000,UC_X86_REG_ES:0x3500}
    for reg,val in saved.items():u.reg_write(reg,val)
    u.reg_write(UC_X86_REG_CS,0x1000);u.reg_write(UC_X86_REG_SS,0x4000);u.reg_write(UC_X86_REG_SP,0x8000);u.reg_write(UC_X86_REG_EFLAGS,flags)
    u.mem_write(0x48000,struct.pack('<'+'H'*(1+len(args)),0xf000,*args))
    calls=[];index=[0x5a];ports=[]
    def bios(uc,num,data):
        assert num==interrupt
        assert not uc.reg_read(UC_X86_REG_EFLAGS)&0x200
        calls.append((num,uc.reg_read(UC_X86_REG_AX),uc.reg_read(UC_X86_REG_BX)))
        for reg in saved:uc.reg_write(reg,0x1111)
        uc.reg_write(UC_X86_REG_CX,0x2222);uc.reg_write(UC_X86_REG_DX,0x3333)
        uc.reg_write(UC_X86_REG_AX,0x17);uc.reg_write(UC_X86_REG_EFLAGS,0x202)
    def portin(uc,port,size,data):
        assert size==1 and not uc.reg_read(UC_X86_REG_EFLAGS)&0x200
        ports.append(('in',port));assert port in (0x22,0x23)
        if port==0x22:return index[0]
        assert index[0]==0x1e;return 0xa4
    def portout(uc,port,size,value,data):
        assert size==1 and port==0x22 and not uc.reg_read(UC_X86_REG_EFLAGS)&0x200
        ports.append(('out',port,value));index[0]=value
    u.hook_add(UC_HOOK_INTR,bios)
    u.hook_add(UC_HOOK_INSN,portin,None,1,0,UC_X86_INS_IN)
    u.hook_add(UC_HOOK_INSN,portout,None,1,0,UC_X86_INS_OUT)
    u.emu_start(0x10000+symbols[function]-codeaddr,0x1f000,count=200)
    assert u.reg_read(UC_X86_REG_IP)==0xf000,(function,'did not return')
    assert u.reg_read(UC_X86_REG_SP)==0x8002
    for reg,val in saved.items():
        # IA16 cdecl BX is caller-saved; the volume helper explicitly preserves it.
        if reg==UC_X86_REG_BX and interrupt:continue
        assert u.reg_read(reg)==val,(function,reg)
    assert u.reg_read(UC_X86_REG_EFLAGS)&0x200==flags&0x200
    if interrupt:
        assert calls==[(interrupt,args[0],args[1] if len(args)>1 else 0)]
        assert u.reg_read(UC_X86_REG_AX)==0x17 and not ports
    else:
        assert not calls and index[0]==0x5a and u.reg_read(UC_X86_REG_AX)==0xa4
        assert ports==[('in',0x22),('out',0x22,0x1e),('in',0x23),('out',0x22,0x5a)]
print('PASS linked 16-bit BIOS argument ABI, callee/segment/stack/IF preservation, and atomic selector-preserving volume read')
