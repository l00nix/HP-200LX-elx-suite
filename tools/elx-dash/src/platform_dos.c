/* SPDX-License-Identifier: GPL-2.0-or-later
 * Real-mode DOS backend. Documented HP services, no TSR, no battery writes.
 * Never call HP services before ROM identification (demo skips them entirely).
 */
#include "dash.h"
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <time.h>
static int simulated,entered;
static unsigned oldmode,oldcursor,oldshape;
static unsigned char textsave[4000];
static volatile unsigned char __far *bda=(unsigned char __far *)0x00400000UL;
static volatile unsigned char __far *rom=(unsigned char __far *)0xf0000000UL;
void demo_init(void);
int demo_read(struct ctl_state *,struct elx_status *);
void demo_call(const struct ctl_plan *);
static unsigned bios15(unsigned ax,unsigned bx);
#pragma aux bios15 = "pushf" "cli" "push ds" "push es" "push bp" "push si" "push di" \
 "int 15h" "cli" "pop di" "pop si" "pop bp" "pop es" "pop ds" "popf" \
 parm [ax] [bx] value [ax] modify [ax bx cx dx];
static unsigned bios10(unsigned ax);
#pragma aux bios10 = "pushf" "cli" "push ds" "push es" "push bp" "push si" "push di" \
 "int 10h" "cli" "pop di" "pop si" "pop bp" "pop es" "pop ds" "popf" \
 parm [ax] value [ax] modify [ax bx cx dx];
static unsigned volume_register(void);
#pragma aux volume_register = "pushf" "cli" "push bx" "in al,22h" "mov bl,al" \
 "mov al,1eh" "out 22h,al" "in al,23h" "mov ah,al" "mov al,bl" \
 "out 22h,al" "mov al,ah" "xor ah,ah" "pop bx" "popf" value [ax] modify [ax];
int dash_open(int demo) {
    unsigned off,i;int found=0;
    static const unsigned char signature[]={0xbb,0x50,0x48,0xb9,2,1,0xba};
    simulated=demo;
    if(demo){demo_init();return 0;}
    ctl_error="LXdash needs an HP 200LX; use --demo on other PCs";
    for(off=0xd000;off<0xe000-10;off++) {
        for(i=0;i<7 && rom[off+i]==signature[i];i++){}
        if(i==7 && rom[off+8]==2 && rom[off+9]==0xcf){found=1;break;}
    }
    if(!found)return -1;
    for(i=0;i<13;i++)if(rom[0xff36+i]!=(unsigned char)"ROM BIOS Ver "[i])return -1;
    return 0;
}
int dash_read(struct ctl_state *s,struct elx_status *b) {
    unsigned i,tries;unsigned char second[256];
    if(simulated)return demo_read(s,b);
    ctl_error="Unable to read stable firmware cache";
    for(tries=0;tries<8;tries++) {
        for(i=0;i<256;i++)b->bda[i]=bda[i];
        for(i=0;i<256;i++)second[i]=bda[i];
        if((b->bda[0xcd]|second[0xcd])&0x18)continue;
        if(memcmp(b->bda+0x9a,second+0x9a,2)||memcmp(b->bda+0xd4,second+0xd4,2))continue;
        break;
    }
    if(tries==8)return -1;
    b->offline=0;b->model=0x200;b->charge_ticks=0;
    for(i=0;i<32;i++){unsigned char ch=rom[0xff36+i];if(ch<32||ch>126)break;b->rom[i]=(char)ch;}b->rom[i]=0;
    s->mode=b->bda[0x49]&0x7f;s->zoom=b->bda[0x9f];
    s->cols=elx_word(b->bda+0x4a);s->rows=(unsigned)b->bda[0x84]+1;
    s->ann=b->bda[0xeb];s->ticks=elx_word(b->bda+0xa3);
    s->contrast=bios15(0x6201,0)&255;s->volume=(volume_register()>>6)&3;
    if(s->contrast>31)return -1;return 0;
}
void dash_call(const struct ctl_plan *p) {
    if(simulated){demo_call(p);return;}
    if(p->vector==0x15)bios15(p->ax,p->bx);else bios10(p->ax);
}
void dash_close(void){}
int dash_enter(void) {
    union REGS r;unsigned i;volatile unsigned char __far *v;
    oldmode=bios10(0x0f00)&0x7f;
    if((oldmode!=2 && oldmode!=3 && oldmode!=7) || bda[0x62]!=0) {
        /* Only page zero is supported. */
        return -1;
    }
    v=(unsigned char __far *)MK_FP(oldmode==7?0xb000:0xb800,0);
    for(i=0;i<4000;i++)textsave[i]=v[i];
    memset(&r,0,sizeof(r));r.h.ah=3;int86(0x10,&r,&r);oldcursor=r.x.dx;oldshape=r.x.cx;
    bios10(6);entered=1;
    if((bios10(0x0f00)&0x7f)!=6){dash_leave();return -1;}return 0;
}
int dash_leave(void) {
    union REGS r;unsigned i;volatile unsigned char __far *v;
    if(!entered)return 0;entered=0;bios10(oldmode);
    if((bios10(0x0f00)&0x7f)!=oldmode)return -1;
    v=(unsigned char __far *)MK_FP(oldmode==7?0xb000:0xb800,0);
    for(i=0;i<4000;i++)v[i]=textsave[i];
    memset(&r,0,sizeof(r));r.h.ah=1;r.x.cx=oldshape;int86(0x10,&r,&r);
    memset(&r,0,sizeof(r));r.h.ah=2;r.x.dx=oldcursor;int86(0x10,&r,&r);
    return 0;
}
int dash_key(void) {
    int k;if(!kbhit())return 0;k=getch();
    if(k==0 || k==0xe0) {
        k=getch();switch(k){case 72:return DK_UP;case 80:return DK_DOWN;case 75:return DK_LEFT;case 77:return DK_RIGHT;}
        return 0;
    }
    return k;
}
void dash_wait(void){delay(50);}
unsigned long dash_clock(void){return (unsigned long)time(0);}
