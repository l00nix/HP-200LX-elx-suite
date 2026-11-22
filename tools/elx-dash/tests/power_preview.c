#include "dash.h"
int dash_charge_test;
int dash_read(struct ctl_state *s,struct elx_status *b){(void)s;(void)b;return -1;}
int dash_read_power(unsigned char *f){(void)f;return -1;}
void dash_call(const struct ctl_plan *p){(void)p;}
/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Exercise shared icon and both real screen renderers, including partial redraw. */
#include "dash.h"
#include "cga.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
int main(void) {
    struct dashboard d;struct elx_reader reader;
    unsigned char flags,raw=0x58;int i;char path[64];
    static const unsigned states[]={0x40,0,8,0x18,0x38,0x40,0};
    memset(&d,0,sizeof(d));d.state.contrast=16;d.state.volume=2;
    d.state.ticks=3273;d.state.ann=0x40;
    d.battery.bda[0x9a]=39;d.battery.bda[0x9b]=57;d.battery.offline=d.demo=1;
    strcpy(d.battery.rom,"ROM BIOS Ver 2.14");
    /* The fast reader must use the documented byte, remain read-only, and fail on EOF. */
    i=open("power-cache.bin",O_CREAT|O_TRUNC|O_WRONLY,0600);assert(i>=0);
    assert(lseek(i,0x4d4L,SEEK_SET)==0x4d4L);assert(write(i,&raw,1)==1);close(i);
    reader.fd=open("power-cache.bin",O_RDONLY);assert(reader.fd>=0);
    assert(elx_read_power(&reader,&flags)==0&&flags==raw);close(reader.fd);
    reader.fd=open("power-cache.bin",O_WRONLY|O_TRUNC);close(reader.fd);
    reader.fd=open("power-cache.bin",O_RDONLY);
    assert(elx_read_power(&reader,&flags)<0);close(reader.fd);unlink("power-cache.bin");
    for(i=0;i<7;i++) {
        d.battery.bda[0xd4]=states[i];d.power_failed=(i==6);
        if(!i)elx_draw(&elx_cga_canvas,&d.battery,"OFFLINE PREVIEW");
        elx_draw_power(&elx_cga_canvas,&d.battery,d.power_failed);
        sprintf(path,"bat-%d.pbm",i);assert(elx_cga_save(path)==0);
    }
    for(i=0;i<7;i++) {
        d.battery.bda[0xd4]=states[i];d.power_failed=(i==6);
        if(!i)dash_draw(&d);
        dash_draw_power(&d);
        sprintf(path,"dash-%d.pbm",i);assert(elx_cga_save(path)==0);
    }
    puts("PASS: power cache byte, EOF handling and actual partial redraw frames");return 0;
}
