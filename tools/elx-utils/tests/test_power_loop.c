/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Link with elxbat.c's production loop; fake time, console and firmware reads. */
#include "cga.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int tick,reads,polls,full,partial,stale,footer;
static int power_ticks[16];
time_t time(time_t *out){if(out)*out=tick;return tick;}
char *ttyname(int fd){(void)fd;return "/dev/tty1";}
int elx_open(struct elx_reader *r,const char *image){(void)image;r->fd=1;return 0;}
int elx_read(struct elx_reader *r,struct elx_status *s){
    (void)r;reads++;memset(s,0,sizeof(*s));s->bda[0x9a]=39;
    if(tick==5){s->bda[0x9a]=255;return -1;}
    s->bda[0xd4]=tick?0:0x40;return 0;
}
int elx_read_power(struct elx_reader *r,unsigned char *flags){
    (void)r;power_ticks[polls++]=tick;
    if(tick==6||tick==7)return -1;
    *flags=tick>=8?0x40:(tick==4?0x18:0);return 0;
}
void elx_close(struct elx_reader *r){(void)r;}
void elx_text_bars(const struct elx_status *s){(void)s;assert(0);}
int elx_console_open(void){return 0;}
int elx_console_key(void){tick++;return tick==12?'r':(tick==13?'q':0);}
void elx_console_wait(void){}
static void rect(int x,int y,int w,int h,int black){(void)x;(void)y;(void)w;(void)h;(void)black;}
static void text(int x,int y,const char *s,int white){(void)x;(void)y;(void)white;assert(strstr(s,"stale"));footer++;}
const struct elx_canvas elx_cga_canvas={rect,0,0,text};
void elx_draw(const struct elx_canvas *c,const struct elx_status *s,const char *stamp){
    (void)c;(void)stamp;assert(s->bda[0x9a]==39);full++;
}
void elx_draw_power(const struct elx_canvas *c,const struct elx_status *s,int failed){
    (void)c;assert(s->bda[0x9a]==39);partial++;if(failed)stale++;
}
int elx_console_close(void){
    static const int expected[]={1,2,3,4,6,7,8,9,11};
    int i;assert(reads==4&&full==3&&polls==9&&footer==1);
    for(i=0;i<polls;i++)assert(power_ticks[i]==expected[i]);
    assert(partial==5&&stale==1);
    puts("PASS: production ELXbat loop: one-second power checks, five-second gauges, change-only redraw, stale/recovery, R, Q");return 0;
}
