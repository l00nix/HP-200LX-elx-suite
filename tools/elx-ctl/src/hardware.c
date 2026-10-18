/* SPDX-License-Identifier: GPL-2.0-or-later
 * Bounded, documented firmware services only. Never invoke power/sleep/ADC,
 * serial services, change mode, or write directly to BIOS RAM.
 */
#include "ctl.h"
#include "elx.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
static struct elx_reader reader;
static int simulated;
static struct ctl_state mock={16,2,3,3,80,25,0x40,3273};
int ctl_open(int simulate) {
    simulated=simulate;reader.fd=-1;
    if(simulated) return 0;
    ctl_error="HP firmware-aware ELKS and readable /dev/kmem required";
    return elx_open(&reader,0);
}
int ctl_get(struct ctl_state *s) {
    unsigned char b[256];
    if(simulated) { *s=mock;return 0; }
    ctl_error="could not read firmware settings";
    if(lseek(reader.fd,0x400UL,SEEK_SET)==(off_t)-1) return -1;
    { int n=read(reader.fd,b,256); if(n!=256) { if(n>=0) errno=EIO;return -1; } }
    s->mode=b[0x49]&0x7f;s->zoom=b[0x9f];s->cols=elx_word(b+0x4a);
    s->rows=(unsigned)b[0x84]+1;s->ann=b[0xeb];s->ticks=elx_word(b+0xa3);
#ifdef __ia16__
    s->contrast=ctl_int15(0x6201,0)&0xff;
    s->volume=(ctl_volume_register()>>6)&3;
#else
    errno=ENODEV;return -1;
#endif
    if(s->contrast>31) { ctl_error="firmware contrast query returned an invalid value";errno=EIO;return -1; }
    return 0;
}
void ctl_call(const struct ctl_plan *p) {
    if(simulated) {
        printf("SIMULATED INT %02X AX=%04X BX=%04X\n",p->vector,p->ax,p->bx);
        switch(p->ax>>8) {
        case 0x62:mock.contrast=p->expected;break;
        case 0x48:mock.volume=p->expected;break;
        case 0x46:mock.ticks=p->expected;break;
        case 0x61:mock.ann=(mock.ann&0x3f)|p->expected;break;
        case 0xd0:mock.zoom=p->expected;break;
        }
        return;
    }
#ifdef __ia16__
    if(p->vector==0x15) ctl_int15(p->ax,p->bx);
    else ctl_int10(p->ax);
#endif
}
void ctl_close(void) { if(reader.fd>=0) elx_close(&reader); }

#ifdef __ia16__
unsigned CT_ABI ctl_int15(unsigned ax,unsigned bx) {
    __asm__ volatile(
        "pushf\n\tcli\n\tpush %%ds\n\tpush %%es\n\tpush %%bp\n\tpush %%si\n\tpush %%di\n\t"
        "int $0x15\n\tcli\n\t"
        "pop %%di\n\tpop %%si\n\tpop %%bp\n\tpop %%es\n\tpop %%ds\n\tpopf"
        : "+a"(ax), "+b"(bx) : : "cx","dx","cc","memory");
    return ax;
}
unsigned CT_ABI ctl_int10(unsigned ax) {
    unsigned bx=0;
    __asm__ volatile(
        "pushf\n\tcli\n\tpush %%ds\n\tpush %%es\n\tpush %%bp\n\tpush %%si\n\tpush %%di\n\t"
        "int $0x10\n\tcli\n\t"
        "pop %%di\n\tpop %%si\n\tpop %%bp\n\tpop %%es\n\tpop %%ds\n\tpopf"
        : "+a"(ax), "+b"(bx) : : "cx","dx","cc","memory");
    return ax;
}
unsigned CT_ABI ctl_volume_register(void) {
    unsigned ax;
    /* Hornet 1Eh: read volume bits without touching clock or power bits.
     * Preserve the index selector across the complete, IRQ-masked read. */
    __asm__ volatile(
        "pushf\n\tcli\n\tpush %%bx\n\t"
        "inb $0x22,%%al\n\tmovb %%al,%%bl\n\tmovb $0x1e,%%al\n\toutb %%al,$0x22\n\t"
        "inb $0x23,%%al\n\tmovb %%al,%%ah\n\tmovb %%bl,%%al\n\toutb %%al,$0x22\n\t"
        "movb %%ah,%%al\n\txorb %%ah,%%ah\n\tpop %%bx\n\tpopf"
        : "=a"(ax) : : "cc","memory");
    return ax;
}
#endif
