#!/usr/bin/env python3
"""Apply optional hotkey support to an ISOLATED HP firmware-aware ELKS tree.
Fails closed on unexpected source; does not build or install a kernel.
"""
import pathlib,sys,shutil
root=pathlib.Path(sys.argv[1]).resolve()
base=root/'elks/arch/i86/drivers/char'
mem=base/'mem.c';kbd=base/'kbd-scancode.c'
m=mem.read_text();k=kbd.read_text()
if 'elxcap_ioctl' in m or 'elxcap_key' in k:
    raise SystemExit('Already patched or partially patched; use a clean isolated tree')
replacements=[(m,'static int kmem_ioctl(', '#ifdef CONFIG_HP200LX_FW\n#include "elxcap-impl.h"\n#endif\n\nstatic int kmem_ioctl('),
              (m,'    switch (cmd) {\n', '    switch (cmd) {\n#ifdef CONFIG_HP200LX_FW\n    case 0x4501: /* ELXcap acquire/renew and read event counter */\n    case 0x4502: /* ELXcap release */\n        return elxcap_ioctl(cmd,arg);\n#endif\n'),
              (k,'static void keyboard_irq(int irq, struct pt_regs *regs);', '#ifdef CONFIG_HP200LX_FW\nextern int elxcap_key(unsigned, unsigned, unsigned);\n#endif\nstatic void keyboard_irq(int irq, struct pt_regs *regs);'),
              (k,'    /* high bit set when key released */', '#ifdef CONFIG_HP200LX_FW\n    if (elxcap_key((unsigned)code, ModeState, (unsigned)E0key)) return;\n#endif\n\n    /* high bit set when key released */')]
for content,old,new in replacements:
    if content.count(old)!=1: raise SystemExit('Unexpected source: '+old)
for _,old,new in replacements[:2]:m=m.replace(old,new)
for _,old,new in replacements[2:]:k=k.replace(old,new)
mem.write_text(m);kbd.write_text(k)
for name in ('elxcap-state.h','elxcap-impl.h'):shutil.copyfile(pathlib.Path(__file__).parent/name,base/name)
print('Patched isolated tree:',root)
