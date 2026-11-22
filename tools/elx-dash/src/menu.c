/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include "cga.h"
#include <stdio.h>
#include <string.h>
#ifndef DASH_DOS_MENU
static const struct elx_canvas *c=&elx_cga_canvas;
#endif
int dash_menu_key(struct dashboard *d,int k) {
#ifdef DASH_DOS_MENU
    return dash_dos_menu_key(d,k);
#else
    int rc;
#ifndef __WATCOMC__
    int direction=0;
#endif
    if(d->confirm) {
        if(k=='y'||k=='Y') {
            int action=d->confirm;d->confirm=0;
            if(action==2) {
                if(d->demo)strcpy(d->message,"DEMO: ABC/LX handoff; no hardware changes");
                else {rc=dash_abc_open();strcpy(d->message,rc==0?"Returned from ABC/LX; resident policy remains active":ctl_error);dash_refresh(d);}
                return 0;
            }
            if(d->demo)strcpy(d->message,"DEMO: test requested; no hardware changes");
            else if(dash_refresh(d)==0) {
                rc=dash_charge_start(&d->limits);
                strcpy(d->message,rc==0?"Test supervisor started; stays active after Q":ctl_error);
            }
            dash_charge_status(d->charge_status,sizeof(d->charge_status));
        } else if(k==27||k=='n'||k=='N'){d->confirm=0;strcpy(d->message,"Charging request cancelled");}
        return 0;
    }
    if(k==27||k=='b'||k=='B'){d->menu=0;return 0;}
    if(k=='q'||k=='Q')return 1;
#ifdef __WATCOMC__
    /* DOS delegates all charging edits to ABC; no ELKS-only limits shown. */
    if(k==DK_DOWN||k==DK_UP||k=='\t')d->menu_item=(d->menu_item+1)%2;
    else if(k==13||k==10) {
        if(d->menu_item==0)d->confirm=2;
        else d->menu=0;
    }
    return 0;
#else
    if(k==DK_DOWN||k=='\t')d->menu_item=(d->menu_item+1)%5;
    else if(k==DK_UP)d->menu_item=(d->menu_item+4)%5;
    else if(k==DK_LEFT||k=='-')direction=-1;
    else if(k==DK_RIGHT||k=='+'||k=='=')direction=1;
    if(direction) {
        if(d->menu_item==0) {
            int v=(int)d->limits.mv+direction*25;
            if(v>=2600 && v<=3000)d->limits.mv=(unsigned)v;
        } else if(d->menu_item==1) {
            int v=(int)d->limits.minutes+direction;
            if(v>=1&&v<=60)d->limits.minutes=(unsigned)v;
            if(d->limits.stall>d->limits.minutes)d->limits.stall=d->limits.minutes;
        } else if(d->menu_item==2) {
            int v=(int)d->limits.stall+direction;
            if(v>=1&&v<=(int)d->limits.minutes)d->limits.stall=(unsigned)v;
        }
    }
    if(k==13||k==10) {
        if(d->menu_item==3) {
            if(d->demo||dash_charge_available())d->confirm=1;
            else strcpy(d->message,ctl_error);
        } else if(d->menu_item==4) {
            if(d->demo)strcpy(d->message,"DEMO: stop requested; no hardware changes");
            else {rc=dash_charge_stop();strcpy(d->message,rc==0?"Stop requested; supervisor verifies charging off":ctl_error);}
        }
    }
    return 0;
#endif
#endif
}
void dash_draw_menu(const struct dashboard *d) {
#ifdef DASH_DOS_MENU
    dash_dos_draw_menu(d);
#else
    char rows[5][70];
#ifndef __WATCOMC__
    int i;
#endif
    c->rect(42,26,556,147,1);c->rect(44,28,552,143,0);
    c->rect(44,28,552,12,1);c->text(52,29,"BATTERY | EXPERIMENTAL NiCd controls",1);
#ifdef __WATCOMC__
    c->rect(48,d->menu_item?144:42,544,13,1);
    c->text(53,43,"Open ABC/LX configuration and charging controls...",d->menu_item==0);
    c->text(52,62,"F2 in ABC/LX: voltage, time, stall and trickle limits",0);
    c->text(52,78,"Charging menu: Start / Stop / Force Trickle / Reason",0);
    c->text(52,94,"ABCTSR continues supervising after either UI closes.",0);
    c->text(52,110,"ABC's automatic policy may restart after Stop.",0);
    c->text(52,129,d->charge_status,0);
    c->text(53,145,"Back to dashboard",d->menu_item==1);
    c->text(52,160,"Up/Down/Tab: select  Enter: choose  Esc: close",0);
#else
    sprintf(rows[0],"Stop voltage: %u.%03u V",d->limits.mv/1000,d->limits.mv%1000);
    sprintf(rows[1],"Maximum time: %u minutes",d->limits.minutes);
    sprintf(rows[2],"Voltage stall: %u minutes",d->limits.stall);
    strcpy(rows[3],"Start bounded NiCd test...");
    strcpy(rows[4],"Stop test (no automatic restart)");
    for(i=0;i<5;i++) {
        int selected=d->menu_item==i;
        if(selected)c->rect(48,42+i*14,544,13,1);
        c->text(53,43+i*14,rows[i],selected);
    }
    c->text(52,119,d->charge_status,0);
    c->text(52,133,"Arrows: select/change  Enter: choose  Esc: close",0);
    c->text(52,149,"EXPERIMENTAL: not a full-charge battery profile.",0);
#endif
    if(d->confirm) {
        c->rect(38,51,564,105,1);c->rect(40,53,560,101,0);
        c->text(52,57,"EXPERIMENTAL: two NiCd AA cells installed?",0);
        c->text(52,73,"Never enable with alkaline, NiMH or lithium cells.",0);
        if(d->confirm==2) {
            c->text(52,89,"ABC/LX uses its own saved charging configuration.",0);
            c->text(52,105,"Review F2 limits before using its Charging menu.",0);
            c->text(52,121,"Its resident policy can restart after Stop Charging.",0);
            c->text(52,137,"Y: confirm NiCd and open ABC/LX   N/Esc: cancel",0);
            return;
        }
        sprintf(rows[0],"Stop at %u.%03uV / %umin / %umin without voltage rise.",d->limits.mv/1000,d->limits.mv%1000,d->limits.minutes,d->limits.stall);
        c->text(52,89,rows[0],0);
        c->text(52,105,"External power required. Remain present for this test.",0);
        c->text(52,121,"No restart / trickle. Supervisor continues after exit.",0);
        c->text(52,137,"Y: confirm NiCd and start   N/Esc: cancel",0);
    }
#endif
}
