#!/usr/bin/env python3
"""Make an isolated elksemu copy fixing the LP64 select timeval conversion."""
from pathlib import Path
import shutil,subprocess,sys
src=Path(sys.argv[1]).resolve();dst=Path(sys.argv[2]).resolve()
assert src!=dst and not dst.is_relative_to(src)
shutil.copytree(src,dst,dirs_exist_ok=True)
p=dst/'elks_sys.c';s=p.read_text();start=s.index('#define sys_select elks_select');end=s.index('#define sys_socket',start)
block=s[start:end]
assert block.count('ELKS_PEEK(long,')==2 and block.count('ELKS_POKE(long,')==2
block=block.replace('ELKS_PEEK(long,','ELKS_PEEK(int32_t,').replace('ELKS_POKE(long,','ELKS_POKE(int32_t,')
p.write_text(s[:start]+block+s[end:])
subprocess.run(['make','-C',str(dst),'TOPDIR='+str(src.parent),'PREFIX='+str(src.parent/'cross'),'elksemu'],check=True)
print('Isolated emulator: '+str(dst/'elksemu'))
