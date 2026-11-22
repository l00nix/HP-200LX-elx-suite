/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include "cga.h"
#include <stdio.h>
#include <string.h>
static struct dashboard painted;
static int have_painted;
static const struct elx_canvas *c=&elx_cga_canvas;
static void center(int x,int y,const char *s) {c->text(x-elx_text_width(s)/2,y,s,0);}
static void choice(int x,int y,int w,const char *s,int selected) {
    if(selected)c->rect(x-3,y-1,w,12,1);
    c->text(x,y,s,selected);
}
static void battery(const struct dashboard *d,int backup) {
    int cx=backup?480:160,low=elx_low_mv(&d->battery,backup);
    int mv=elx_mv(d->battery.bda[0x9a+backup]);char s[60];

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
static void control(const struct dashboard *d,int volume) {
    char s[24];int cx=volume?480:160;
    static const char *volumes[]={"OFF","SOFT","MEDIUM","LOUD"};
    if(volume)choice(433,113,100,"VOLUME",d->selected==1);
    else choice(109,113,108,"CONTRAST",d->selected==0);
    elx_meter(c,cx,158,91,27,volume?d->state.volume*20:d->state.contrast*60/31);
    if(volume)center(cx,145,volumes[d->state.volume<=3?d->state.volume:0]);
    else {sprintf(s,"%u / 31",d->state.contrast);center(cx,145,s);}
    c->text(cx-107,149,"0",0);c->text(cx+(volume?97:95),149,volume?"3":"31",0);
    if(volume){elx_tick(c,cx,158,91,27,20);elx_tick(c,cx,158,91,27,40);}
}
static void settings(const struct dashboard *d) {
    char s[48];const char *ann;unsigned seconds;
    static const char *zooms[]={"80x25","64x18","40x25","40x16"};
    ann=(d->state.ann&0x80)?"Off":((d->state.ann&0x40)?"Right":"Left");
    sprintf(s,"Zoom %s%s",zooms[d->zoom],d->zoom_pending?"*":"");choice(8,164,190,s,d->selected==2);
    sprintf(s,"Icons %s",ann);choice(205,164,180,s,d->selected==3);
    seconds=(unsigned)((unsigned long)d->state.ticks*55/1000);
    if(d->state.ticks)sprintf(s,"BIOS timeout %us",seconds);else sprintf(s,"BIOS timeout Off");
    choice(397,164,238,s,d->selected==4);
}
static void message(const struct dashboard *d) {
    c->text(8,177,d->failed?"READ ERROR: STALE VALUES; controls disabled":d->message,0);
}
void dash_draw(const struct dashboard *d) {
    c->rect(0,0,640,200,0);c->rect(0,0,640,13,1);
    c->text(7,1,DASH_NAME " " DASH_VERSION,1);
    c->text(180,1,d->demo?"DEMO - NO HARDWARE CHANGES":"HP 200LX | nominal BIOS warning markers",1);
    dash_draw_power(d);
    center(160,26,"MAIN BATTERIES");center(480,26,"BACKUP BATTERY");
    battery(d,0);battery(d,1);c->line(320,26,320,160);
    control(d,0);control(d,1);settings(d);message(d);
    c->rect(0,189,640,11,1);
    c->text(8,189,"B: Battery  Tab: select  +/-: adjust  R: reread  Q: exit",1);
    if(d->menu)dash_draw_menu(d);
    painted=*d;have_painted=1;
}
/* Compare visible values, not the BIOS cache: its clock and countdown bytes
 * change even when nothing on screen does. Modal overlays own the middle of
 * the display; restore their background once when closing, never on a poll. */
int dash_draw_update(const struct dashboard *d) {
    int i,changed=0;
    const struct dashboard *p=&painted;
    if(!have_painted || (p->menu && !d->menu) ||
       (p->confirm && !d->confirm) || p->demo!=d->demo) {
        dash_draw(d);return 1;
    }
    if(((p->battery.bda[0xd4]^d->battery.bda[0xd4])&0xf8) || p->power_failed!=d->power_failed) {
        dash_draw_power(d);changed=1;
    }
    if(d->menu) {
        if(!p->menu || p->confirm!=d->confirm || p->menu_item!=d->menu_item ||
           p->limits.mv!=d->limits.mv || p->limits.minutes!=d->limits.minutes ||
           p->limits.stall!=d->limits.stall || strcmp(p->charge_status,d->charge_status) ||
           p->abc.valid!=d->abc.valid || p->abc_action!=d->abc_action ||
           memcmp(p->abc.value,d->abc.value,sizeof(d->abc.value)) ||
           memcmp(p->abc_edit,d->abc_edit,sizeof(d->abc_edit)) ||
           ((p->battery.bda[0xd4]^d->battery.bda[0xd4])&0xb8)) {
            dash_draw_menu(d);changed=1;
        }
    } else {
        for(i=0;i<2;i++) {
            if(p->battery.bda[0x9a+i]!=d->battery.bda[0x9a+i] ||
               elx_low_mv(&p->battery,i)!=elx_low_mv(&d->battery,i) ||
               strcmp(elx_alert(&p->battery,i),elx_alert(&d->battery,i))) {
                c->rect(i?321:0,38,i?319:320,72,0);battery(d,i);changed=1;
            }
            if((i?p->state.volume!=d->state.volume:p->state.contrast!=d->state.contrast) ||
               (p->selected==(unsigned)i)!=(d->selected==(unsigned)i)) {
                c->rect(i?321:0,112,i?319:320,49,0);control(d,i);changed=1;
            }
        }
        if(p->zoom!=d->zoom || p->zoom_pending!=d->zoom_pending ||
           p->state.ann!=d->state.ann || p->state.ticks!=d->state.ticks ||
           (p->selected!=d->selected && (p->selected>=2 || d->selected>=2))) {
            c->rect(0,163,640,13,0);settings(d);changed=1;
        }
    }
    if(p->failed!=d->failed || strcmp(p->message,d->message)) {
        c->rect(0,177,640,12,0);message(d);changed=1;
    }
    painted=*d;return changed;
}
