/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include "cga.h"
#include <stdio.h>
static const struct elx_canvas *c=&elx_cga_canvas;
static void center(int x,int y,const char *s) {c->text(x-elx_text_width(s)/2,y,s,0);}
static void choice(int x,int y,int w,const char *s,int selected) {
    if(selected)c->rect(x-3,y-1,w,12,1);
    c->text(x,y,s,selected);
}
static void battery(const struct dashboard *d,int backup) {
    int cx=backup?480:160,low=elx_low_mv(&d->battery,backup);
    int mv=elx_mv(d->battery.bda[0x9a+backup]);char s[60];
    center(cx,26,backup?"BACKUP BATTERY":"MAIN BATTERIES");
    elx_meter(c,cx,81,103,31,elx_bar(mv,60));
    if(low>=0)elx_marker(c,cx,81,103,31,elx_bar(low,60),0);
    if(!backup)elx_marker(c,cx,81,103,31,0,1);
    elx_digits(c,cx,64,mv);
    c->text(cx-116,85,"2.0",0);c->text(cx+91,85,"3.5",0);
    if(low>=0)sprintf(s,"%s | low %d.%02dV%s",elx_alert(&d->battery,backup),low/1000,low%1000/10,backup?"":" crit 2.00V");
    else sprintf(s,"%s | low threshold unknown",elx_alert(&d->battery,backup));
    center(cx,98,s);
}
void dash_draw_power(const struct dashboard *d) {
    char s[100];
    c->rect(0,14,640,12,0);
    elx_power_icon(c,8,14,d->battery.bda[0xd4],d->power_failed);
    if(d->power_failed)c->text(56,14,"Power: unknown | Read failed; last status stale",0);
    else {
        sprintf(s,"%s | %s | Charge %s | cached volts, age unknown",(d->battery.bda[0xd4]&0x80)?"NiCad":"Alkaline",elx_source(&d->battery),elx_charging(&d->battery));
        c->text(56,14,s,0);
    }
}
void dash_draw(const struct dashboard *d) {
    char s[100];const char *ann,*zoom;unsigned seconds;
    static const char *zooms[]={"80x25","64x18","40x25","40x16"};
    static const char *volumes[]={"OFF","SOFT","MEDIUM","LOUD"};
    c->rect(0,0,640,200,0);c->rect(0,0,640,13,1);
    c->text(7,1,DASH_NAME " " DASH_VERSION,1);
    c->text(180,1,d->demo?"DEMO - NO HARDWARE CHANGES":"HP 200LX | nominal BIOS warning markers",1);
    dash_draw_power(d);
    battery(d,0);battery(d,1);c->line(320,26,320,160);
    choice(109,113,108,"CONTRAST",d->selected==0);
    choice(433,113,100,"VOLUME",d->selected==1);
    elx_meter(c,160,158,91,27,d->state.contrast*60/31);
    elx_meter(c,480,158,91,27,d->state.volume*20);
    sprintf(s,"%u / 31",d->state.contrast);center(160,145,s);
    center(480,145,volumes[d->state.volume<=3?d->state.volume:0]);
    c->text(53,149,"0",0);c->text(255,149,"31",0);
    c->text(355,149,"0",0);c->text(577,149,"3",0);
    /* Discrete volume levels have visible marks. */
    elx_tick(c,480,158,91,27,20);elx_tick(c,480,158,91,27,40);
    zoom=zooms[d->zoom];ann=(d->state.ann&0x80)?"Off":((d->state.ann&0x40)?"Right":"Left");
    sprintf(s,"Zoom %s%s",zoom,d->zoom_pending?"*":"");choice(8,164,190,s,d->selected==2);
    sprintf(s,"Icons %s",ann);choice(205,164,180,s,d->selected==3);
    seconds=(unsigned)((unsigned long)d->state.ticks*55/1000);
    if(d->state.ticks)sprintf(s,"BIOS timeout %us",seconds);else sprintf(s,"BIOS timeout Off");
    choice(397,164,238,s,d->selected==4);
    c->text(8,177,d->failed?"READ ERROR: STALE VALUES; controls disabled":d->message,0);
    c->rect(0,189,640,11,1);
    c->text(8,189,"Tab: select  Arrows/+/-: adjust  R: reread  Q/Esc: exit",1);
}
