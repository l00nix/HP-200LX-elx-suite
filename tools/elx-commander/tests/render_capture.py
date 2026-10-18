# SPDX-License-Identifier: GPL-2.0-or-later
# Render the captured ANSI stream; does not invent interface content.
from pathlib import Path
import re
from PIL import Image,ImageDraw,ImageFont
import sys
s=Path(sys.argv[1]).read_bytes().decode('utf-8')
cols,rows=80,25
cells=[[(' ',False,False) for _ in range(cols)] for _ in range(rows)]
x=y=0;saved=(0,0);rev=blue=False
parts=re.findall(r'\x1b\[[0-?]*[ -/]*[@-~]|\x1b[78]|[^\x1b]+',s)
for t in parts:
 if t=='\x1b7':saved=(x,y)
 elif t=='\x1b8':x,y=saved
 elif t.startswith('\x1b['):
  raw=t[2:-1];cmd=t[-1]
  if raw.startswith('?'):continue
  ns=[int(n) if n else 0 for n in raw.split(';')]
  if cmd=='H':
   y=min(rows-1,max(0,(ns[0] or 1)-1));x=min(cols-1,max(0,(ns[1] if len(ns)>1 and ns[1] else 1)-1))
  elif cmd=='K':
   for j in range(x,cols):cells[y][j]=(' ',False,False)
  elif cmd=='J':
   for i in range(y,rows):
    for j in range(x if i==y else 0,cols):cells[i][j]=(' ',False,False)
  elif cmd=='m':
   for n in ns:
    if n==0:rev=blue=False
    elif n==7:rev=True
    elif n==94:blue=True
    elif n==39:blue=False
 else:
  for c in t:
   if c=='\r':x=0
   elif c=='\n':y=min(rows-1,y+1)
   else:
    if x>=cols:x=0;y=min(rows-1,y+1)
    cells[y][x]=(c,rev,blue);x+=1
font=ImageFont.truetype('/System/Library/Fonts/Menlo.ttc',20)
cw,ch=13,26;pad=18
im=Image.new('RGB',(cols*cw+2*pad,rows*ch+2*pad),'#f3f1e8');d=ImageDraw.Draw(im)
for i,row in enumerate(cells):
 for j,(c,r,b) in enumerate(row):
  px=pad+j*cw;py=pad+i*ch
  if r:d.rectangle((px,py,px+cw-1,py+ch-1),fill='#1b2024')
  d.text((px,py),c,font=font,fill='#f3f1e8' if r else ('#83b6ff' if b else '#1b2024'))
out=Path(sys.argv[2]);out.parent.mkdir(parents=True,exist_ok=True);im.save(out)
print(out)
