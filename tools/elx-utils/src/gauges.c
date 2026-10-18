/* SPDX-License-Identifier: GPL-2.0-or-later
 * 640x200 HP LCD: 2:1 pixel aspect correction, so the two arcs appear round
 * on the panel / a 640x400 aspect-correct preview. Integer-only rendering.
 */
#include "cga.h"
#include <stdio.h>
static void gauge(const struct elx_canvas *c,int cx,unsigned raw,const char *title,const struct elx_status *s,int backup) {
    int mv=elx_mv(raw);
    c->text(cx-elx_text_width(title)/2,48,title,0);
    elx_meter(c,cx,136,112,56,elx_bar(mv,60));
    c->text(cx-125,140,"2.0",0);c->text(cx+99,140,"3.5",0);
    c->text(cx-70,68,"2.5",0);c->text(cx+42,68,"3.0",0);
    elx_tick(c,cx,136,112,56,20);elx_tick(c,cx,136,112,56,40);
    elx_digits(c,cx,107,mv);
    {
        const char *note=elx_mv(raw)<0?"Sample unavailable":elx_alert(s,backup);
        c->text(cx-elx_text_width(note)/2,157,note,0);
    }
    {
        int low=elx_low_mv(s,backup);char msg[48];
        if(low>=0) {
            elx_marker(c,cx,136,112,56,elx_bar(low,60),0);
            sprintf(msg,"Low %d.%02d V%s",low/1000,(low%1000)/10,backup?"":" | critical 2.00 V");
        } else sprintf(msg,"Low threshold unverified");
        if(!backup)elx_marker(c,cx,136,112,56,0,1);
        c->text(cx-elx_text_width(msg)/2,169,msg,0);
    }
    if(elx_mv(raw)>=0 && elx_mv(raw)<2000) c->text(cx-elx_text_width("Below 2.0 V")/2,139,"Below 2.0 V",0);
}
void elx_draw_power(const struct elx_canvas *c,const struct elx_status *s,int stale) {
    char line[100];
    c->rect(0,21,640,19,0);
    elx_power_icon(c,8,23,s->bda[0xd4],stale);
    if(stale)c->text(56,25,"Power: unknown | Read failed; last status stale",0);
    else {
        sprintf(line,"%s | Power: %s | Charge: %s",elx_type(s),elx_source(s),elx_charging(s));
        c->text(56,25,line,0);
    }
}
void elx_draw(const struct elx_canvas *c,const struct elx_status *s,const char *stamp) {
    c->rect(0,0,640,200,0); c->rect(0,0,640,18,1);
    c->text(8,2,"ELXbat " ELX_BAT_VERSION " | Battery voltage",1);
    c->text(442,2,stamp,1);
    elx_draw_power(c,s,0);
    gauge(c,160,s->bda[0x9a],"MAIN BATTERIES",s,0);
    gauge(c,480,s->bda[0x9b],"BACKUP BATTERY",s,1);
    c->line(320,50,320,173);
    c->text(8,183,s->offline?"OFFLINE | Nominal BIOS markers | Q/Esc: quit":
        "Cached volts, age unknown | Nominal BIOS markers | Q: quit R: reread",0);
}
