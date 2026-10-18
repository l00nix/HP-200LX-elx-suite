/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct ctl_state current;
static int calls,fail_read,ignore_write;
static unsigned char power_flags;
int dash_read_power(unsigned char *flags) {
    if(fail_read)return -1;*flags=power_flags;return 0;
}
int dash_read(struct ctl_state *s,struct elx_status *b) {
    if(fail_read)return -1;*s=current;memset(b,0,sizeof(*b));
    b->bda[0x9a]=39;b->bda[0x9b]=57;strcpy(b->rom,"ROM BIOS Ver 2.14");return 0;
}
void dash_call(const struct ctl_plan *p) {
    calls++;if(ignore_write)return;
    switch(p->ax>>8){case 0x62:current.contrast=p->expected;break;
    case 0x48:current.volume=p->expected;break;case 0x46:current.ticks=p->expected;break;
    case 0x61:current.ann=(current.ann&0x3f)|p->expected;break;
    case 0xd0:assert(current.mode!=6);current.zoom=p->expected;break;default:assert(0);}
}
static void reset(struct dashboard *d) {
    struct ctl_state initial={16,2,3,0x85,80,25,0x45,3273};current=initial;
    fail_read=ignore_write=calls=0;assert(dash_init(d,0)==0);current.mode=6;
}
int main(void) {
    struct dashboard d;int i,n;
    reset(&d);assert(d.zoom==3);assert(d.selected==0);
    for(i=0;i<40;i++)assert(dash_adjust(&d,1)==0);
    assert(d.state.contrast==31);n=calls;assert(dash_adjust(&d,1)==0&&calls==n);
    for(i=0;i<40;i++)assert(dash_adjust(&d,-1)==0);assert(d.state.contrast==0);
    current.contrast=17;assert(dash_adjust(&d,1)==0&&d.state.contrast==18);
    dash_key_event(&d,'\t');assert(d.selected==1);
    for(i=0;i<8;i++)dash_adjust(&d,-1);assert(d.state.volume==0);
    for(i=0;i<8;i++)dash_adjust(&d,1);assert(d.state.volume==3);
    d.selected=3;dash_adjust(&d,1);assert((current.ann&0xc0)==0xc0&&(current.ann&0x3f)==5);
    dash_adjust(&d,1);assert(current.ann==5);
    d.selected=4;for(i=0;i<150;i++)dash_adjust(&d,-1);assert(current.ticks==0);
    for(i=0;i<150;i++)dash_adjust(&d,1);assert(current.ticks==65528);
    reset(&d);d.selected=2;n=calls;dash_adjust(&d,1);assert(calls==n&&d.zoom_pending&&d.zoom==0);
    current.mode=3;current.zoom=3;assert(dash_finish(&d,1)==0&&current.zoom==3);
    reset(&d);d.selected=2;dash_adjust(&d,1);current.mode=3;current.zoom=3;
    assert(dash_finish(&d,0)==0&&current.zoom==0x85); /* interrupted exit cancels queued zoom */
    reset(&d);ignore_write=1;assert(dash_adjust(&d,1)<0&&d.state.contrast==16);
    assert(strstr(d.message,"mismatch"));ignore_write=0;
    fail_read=1;n=calls;assert(dash_adjust(&d,1)<0&&calls==n&&d.failed);
    fail_read=0;assert(dash_adjust(&d,1)<0&&calls==n);dash_key_event(&d,'r');assert(!d.failed);
    assert(dash_key_event(&d,27)==1);assert(dash_key_event(&d,'q')==1);
    reset(&d);current.mode=6;assert(dash_init(&d,0)<0);
    current.mode=7;current.zoom=7;assert(dash_init(&d,0)==0);
    d.selected=2;dash_adjust(&d,1);assert(d.zoom==0);dash_adjust(&d,1);assert(d.zoom==3);
    assert(elx_low_mv(&d.battery,0)==2200);d.battery.bda[0xd4]=0x80;
    assert(elx_low_mv(&d.battery,0)==2300&&elx_low_mv(&d.battery,1)==2700);
    strcpy(d.battery.rom,"unknown");assert(elx_low_mv(&d.battery,1)==-1);
    d.battery.bda[0xd4]|=3;assert(!strcmp(elx_alert(&d.battery,1),"LOW"));
    d.battery.bda[0xd5]=0x40;assert(!strcmp(elx_alert(&d.battery,0),"VERY LOW"));
    reset(&d);d.battery.bda[0xd4]=0x83;power_flags=0x40;n=calls;
    assert(dash_poll_power(&d)&&d.battery.bda[0xd4]==0xc3);
    assert(!dash_poll_power(&d)); /* unchanged: no redraw */
    power_flags=0;assert(dash_poll_power(&d)&&d.battery.bda[0xd4]==0x83);
    power_flags=8;assert(dash_poll_power(&d)); /* enabled, inactive */
    power_flags=0x18;assert(dash_poll_power(&d)); /* charging */
    power_flags=0x38;assert(dash_poll_power(&d)); /* fast charging */
    fail_read=1;assert(dash_poll_power(&d)&&d.power_failed);
    assert(!dash_poll_power(&d));
    d.failed=1;fail_read=0;assert(dash_poll_power(&d)&&!d.power_failed&&d.failed);
    assert(calls==n); /* polling never writes settings or clears control failure */
    power_flags=0x40;assert(dash_poll_power(&d)); /* unplugged */
    puts("PASS: power transitions, stale/recovery, no writes; bounds, live read-back, failures, queued/restored zoom, discrete choices, timeout, battery profiles");return 0;
}
