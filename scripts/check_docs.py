#!/usr/bin/env python3
"""Validate local Markdown links/images and self-contained keyboard SVGs."""
from pathlib import Path
from urllib.parse import unquote
import re,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[1]
errors=[];links=0;icons=0
for p in [root/'README.md',root/'CONTRIBUTING.md',*sorted((root/'docs').rglob('*.md'))]:
 for target in re.findall(r'\]\(([^)]+)\)',p.read_text()):
  target=target.strip('<>').split('#',1)[0]
  if not target or re.match(r'^[a-zA-Z]+:',target):continue
  links+=1
  if not (p.parent/unquote(target)).exists():errors.append(f'{p.relative_to(root)}: missing {target}')
for p in (root/'docs/assets/keys').glob('*.svg'):
 icons+=1;s=p.read_text();tree=ET.fromstring(s)
 if 'http' in s.replace('http://www.w3.org/2000/svg','') or '<script' in s or '<foreignObject' in s:
  errors.append(f'{p.name}: external or active content')
 if tree.find('{http://www.w3.org/2000/svg}title') is None:errors.append(f'{p.name}: missing accessible title')
assert not errors,'\n'.join(errors)
print(f'PASS: {links} local links/images and {icons} self-contained SVG key icons')
