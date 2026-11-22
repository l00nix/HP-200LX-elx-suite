/* Exercise the real incremental renderer against a full reference frame. */
#include "dash.h"
#include "cga.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int dash_charge_test;
int dash_read(struct ctl_state *s,struct elx_status *b){(void)s;(void)b;return -1;}
int dash_read_power(unsigned char *f){(void)f;return -1;}
void dash_call(const struct ctl_plan *p){(void)p;}
static void check(struct dashboard *d,int n) {
    char path[60];
    assert(dash_draw_update(d)==1);
    sprintf(path,"update-%02d.pbm",n);assert(elx_cga_save(path)==0);
    dash_draw(d);
    sprintf(path,"full-%02d.pbm",n);assert(elx_cga_save(path)==0);
    assert(dash_draw_update(d)==0);
}
int main(void) {
    struct dashboard d;int n=0,i;
    memset(&d,0,sizeof(d));d.state.contrast=16;d.state.volume=2;
    d.state.ticks=3273;d.state.ann=0x40;
    d.battery.bda[0x9a]=39;d.battery.bda[0x9b]=57;d.demo=1;
    strcpy(d.battery.rom,"ROM BIOS Ver 2.14");
    check(&d,n++);
    /* Ordinary BIOS clock/countdown churn must cause no display writes. */
    d.battery.bda[0x6c]++;d.battery.bda[0xd2]++;assert(dash_draw_update(&d)==0);
    d.battery.bda[0x9a]=40;check(&d,n++);
    d.battery.bda[0x9b]=63;check(&d,n++);
    d.battery.bda[0xd4]=0x83;check(&d,n++);
    d.battery.bda[0xd5]=0x40;check(&d,n++);
    d.battery.bda[0x9a]=255;check(&d,n++);
    d.battery.bda[0x9a]=0;check(&d,n++);
    for(i=0;i<32;i++){d.state.contrast=i;check(&d,n++);}
    for(i=0;i<4;i++){d.state.volume=i;check(&d,n++);}
    for(i=1;i<5;i++){d.selected=i;check(&d,n++);}
    d.selected=0;check(&d,n++);
    d.zoom=3;d.zoom_pending=1;d.state.ann=0x80;d.state.ticks=0;check(&d,n++);
    strcpy(d.message,"Setting applied and verified");check(&d,n++);
    strcpy(d.message,"Short");check(&d,n++);
    d.failed=d.power_failed=1;check(&d,n++);
    d.failed=d.power_failed=0;check(&d,n++);
    d.menu=1;check(&d,n++);
    d.menu_item=1;check(&d,n++);
    d.confirm=2;check(&d,n++);
    d.confirm=0;check(&d,n++);
    /* Polls beneath a modal must not overwrite it or cause it to flash. */
    d.battery.bda[0x9a]=48;d.state.contrast=20;
    assert(dash_draw_update(&d)==0);
    d.menu=0;check(&d,n++);
    puts("PASS: incremental redraw, idle polling, controls, modal stability and restoration");
    return 0;
}
