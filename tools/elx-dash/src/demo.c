/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include <string.h>
static struct ctl_state state;
void demo_init(void) {
    struct ctl_state initial={16,2,3,3,80,25,0x40,3273};state=initial;
}
int demo_read(struct ctl_state *s,struct elx_status *b) {
    *s=state;memset(b,0,sizeof(*b));b->offline=1;b->model=0x201;
    b->bda[0x9a]=39;b->bda[0x9b]=57;b->bda[0xd4]=0x40;
    strcpy(b->rom,"ROM BIOS Ver 2.14");return 0;
}
void demo_call(const struct ctl_plan *p) {
    switch(p->ax>>8) {
    case 0x62:state.contrast=p->expected;break;
    case 0x48:state.volume=p->expected;break;
    case 0x46:state.ticks=p->expected;break;
    case 0x61:state.ann=(state.ann&0x3f)|p->expected;break;
    case 0xd0:state.zoom=p->expected;break;
    }
}
