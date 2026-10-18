/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "ctl.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    struct ctl_state s={16,2,3,3,80,25,0x45,3273};
    struct ctl_request r={CT_ZOOM,1,0,1,0};struct ctl_plan p;
    unsigned mode,v;
    for(mode=2;mode<=3;mode++) for(v=0;v<4;v++) {
        s.mode=mode;r.value=v;assert(!ctl_plan(&r,&s,&p));
        assert(p.vector==0x10 && p.bx==0);
        assert(p.ax==(v?0xd080+(v-1)*2+(mode==3):0xd000+mode));
        s.zoom=p.expected;assert(ctl_verified(CT_ZOOM,&p,&s));
        ++s.zoom;assert(!ctl_verified(CT_ZOOM,&p,&s));
    }
    s.mode=7;r.value=0;assert(!ctl_plan(&r,&s,&p) && p.ax==0xd021);
    r.value=3;assert(!ctl_plan(&r,&s,&p) && p.ax==0xd007);
    r.value=1;assert(ctl_plan(&r,&s,&p)==-1);
    s.mode=6;assert(ctl_plan(&r,&s,&p)==-1);
    s.mode=3;s.cols=40;assert(ctl_plan(&r,&s,&p)==-1);s.cols=80;
    s.rows=50;assert(ctl_plan(&r,&s,&p)==-1);s.rows=25;
    r.command=CT_TIMEOUT;
    for(v=0;v<=3604;v++) {
        unsigned long ms=(unsigned long)v*1000;
        r.value=v;assert(!ctl_plan(&r,&s,&p));
        assert(p.ax==0x4600 && p.expected==p.bx);
        assert(p.bx*55UL>=ms && p.bx*55UL<ms+55);
    }
    r.command=CT_ANN;r.value=0x80;assert(!ctl_plan(&r,&s,&p));
    assert(p.ax==0x61e0 && p.expected==0xc0);
    r.value=0;assert(!ctl_plan(&r,&s,&p) && p.ax==0x6120);
    r.value=0x40;assert(!ctl_plan(&r,&s,&p) && p.ax==0x6160);
    r.command=CT_CONTRAST;r.value=31;assert(!ctl_plan(&r,&s,&p));
    assert(p.ax==0x6200 && p.bx==31 && p.vector==0x15);
    s.contrast=30;assert(!ctl_verified(CT_CONTRAST,&p,&s));
    r.command=CT_VOLUME;r.value=0;assert(!ctl_plan(&r,&s,&p) && p.ax==0x4800);
    puts("PASS zoom modes/geometry guards, all timeout values, annunciator preservation, BIOS call plans and read-back mismatch");
    return 0;
}
