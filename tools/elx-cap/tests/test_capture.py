#!/usr/bin/env python3
"""Independent synthetic 1MB images. No HP ROM bytes bundled.
Pass binary alone for host, or emulator and binary for real 16-bit tests.
"""
import os,signal,struct,subprocess,sys,tempfile,time
from pathlib import Path
cmd=[str(Path(p).resolve()) for p in sys.argv[1:]]
def memory(mode=6):
    b=bytearray(1048576)
    b[0xfd88e:0xfd898]=bytes.fromhex('bb5048b90201ba0102cf')
    b[0x449]=mode;struct.pack_into('<H',b,0x44a,80);b[0x484]=24
    return b
def run(args,code=0):
    p=subprocess.run(cmd+args,capture_output=True,timeout=20)
    assert p.returncode==code,(args,p.returncode,p.stdout,p.stderr)
    return p
def bmp(path):
    d=path.read_bytes()
    assert len(d)==16062 and d[:2]==b'BM'
    assert struct.unpack_from('<I',d,2)[0]==len(d)
    assert struct.unpack_from('<I',d,10)[0]==62
    assert struct.unpack_from('<IiiHHII',d,14)==(40,640,200,1,1,0,16000)
    assert d[54:62]==b'\xff\xff\xff\x00\x00\x00\x00\x00'
    return b''.join(d[62+y*80:62+(y+1)*80] for y in range(199,-1,-1))
with tempfile.TemporaryDirectory(prefix='elxcap-') as tmp:
    root=Path(tmp);os.chdir(root)
    expected=bytes((y*37+x*11+(y//7))%256 for y in range(200) for x in range(80))
    b=memory()
    for y in range(200):
        pos=0xb8000+(y%2)*8192+(y//2)*80
        b[pos:pos+80]=expected[y*80:(y+1)*80]
    image=root/'graphics.img';image.write_bytes(b)
    run(['--image',str(image)])
    assert bmp(root/'CAP0000.BMP')==expected
    run(['--image',str(image)])
    assert bmp(root/'CAP0001.BMP')==expected
    run(['--image',str(image),'-i','negative.bmp'])
    assert bmp(root/'negative.bmp')==bytes(v^255 for v in expected)
    run(['--image',str(image),'-f','pbm','screen.pbm'])
    assert (root/'screen.pbm').read_bytes()==b'P4\n640 200\n'+expected
    original=(root/'CAP0000.BMP').read_bytes()
    run(['--image',str(image),'CAP0000.BMP'],1)
    assert (root/'CAP0000.BMP').read_bytes()==original
    (root/'link.bmp').symlink_to(root/'CAP0000.BMP')
    run(['--image',str(image),'link.bmp'],1)
    assert (root/'CAP0000.BMP').read_bytes()==original
    # Text: a nonzero active page, non-ASCII code-page glyph, reverse and hidden.
    t=memory(3);struct.pack_into('<H',t,0x44e,4096);t[0x462]=1
    glyph=bytes([0x18,0x3c,0x66,0x7e,0x66,0x66,0,0]);high=bytes([0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81])
    t[0xffa6e+65*8:0xffa6e+66*8]=glyph;t[0x9fc00:0x9fc08]=high
    t[0xb9000:0xb9008]=bytes([65,7,65,0x70,128,7,65,0])
    text=root/'text.img';text.write_bytes(t)
    run(['--image',str(text),'text.bmp'])
    pixels=bmp(root/'text.bmp')
    for y in range(8):assert pixels[y*80:y*80+4]==bytes([glyph[y],glyph[y]^255,high[y],0])
    assert pixels[640:]==bytes(15360)
    for name,off,value in [('mode',0x449,7),('columns',0x44a,40),('rows',0x484,42),('page',0x44f,0xff),('identity',0xfd88e,0)]:
        bad=bytearray(t);bad[off]=value;(root/'bad.img').write_bytes(bad)
        run(['--image',str(root/'bad.img'),name+'.bmp'],1)
        assert not (root/(name+'.bmp')).exists()
    (root/'short.img').write_bytes(b[:8192])
    run(['--image',str(root/'short.img'),'short.bmp'],1)
    assert not (root/'short.bmp').exists()
    for args in [['-d','-1'],['-d','3601'],['-d','x'],['-d','999999999999999999999999'],['-f','png'],['--wat'],['--watch',''],['--watch',tmp,'a.bmp'],['--hotkey',tmp,'--image',str(image)]]:run(args,2)
    run(['--watch',str(root/'absent'),'--image',str(image)],1)
    run(['--help']);run(['--version'])
    run(['--image',str(image),'-q','-d','1','delay.bmp'])
    assert bmp(root/'delay.bmp')==expected
    # SIGUSR1 listener operates without a terminal and exits cleanly.
    watch=root/'watch';watch.mkdir()
    p=subprocess.Popen(cmd+['--image',str(image),'--watch',str(watch)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True)
    import atexit
    def cleanup():
        if p.poll() is None:
            os.killpg(p.pid,signal.SIGKILL);p.wait()
    atexit.register(cleanup)
    ready=p.stdout.readline();assert b'PID' in ready,ready
    # elksemu's process is also its emulated process for forwarded signals.
    time.sleep(.2)
    p.send_signal(signal.SIGUSR1)
    deadline=time.monotonic()+5
    while time.monotonic()<deadline:
        if (watch/'CAP0000.BMP').exists() and (watch/'CAP0000.BMP').stat().st_size==16062:break
        time.sleep(.02)
    assert bmp(watch/'CAP0000.BMP')==expected
    time.sleep(.2)
    p.send_signal(signal.SIGUSR1)
    deadline=time.monotonic()+5
    while time.monotonic()<deadline:
        if (watch/'CAP0001.BMP').exists() and (watch/'CAP0001.BMP').stat().st_size==16062:break
        time.sleep(.02)
    assert bmp(watch/'CAP0001.BMP')==expected
    p.terminate();out,err=p.communicate(timeout=5);assert p.returncode==0,(p.returncode,out,err)
print('PASS graphics/BMP/PBM/polarity, text/font/page/attributes, no-overwrite/symlinks, unsupported/short inputs, CLI, delay, repeated SIGUSR1 and clean exit')
