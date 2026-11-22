/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "charge.h"
#include <string.h>
int charge_limits_valid(const struct charge_limits *p) {
    return p->mv>=2600 && p->mv<=3000 && p->minutes>=1 && p->minutes<=60 &&
        p->stall>=1 && p->stall<=p->minutes;
}
static const char *unsafe(int mv,unsigned f,int valid) {
    if(!valid || mv<2000 || mv>3300)return "Invalid fresh voltage";
    if(f&0x8000)return "Firmware overvoltage";
    if(f&0x4000)return "Firmware very-low battery";
    if(!(f&0x80))return "NiCd configuration lost";
    if(f&0x40)return "External power disconnected";
    return 0;
}
int charge_begin(struct charge_cycle *c,const struct charge_limits *p,unsigned long now,int mv,unsigned f) {
    const char *why;memset(c,0,sizeof(*c));
    c->reason="Invalid limits";
    if(!charge_limits_valid(p))return -1;
    why=unsafe(mv,f,1);if(why){c->reason=why;return -1;}
    if(f&0x38){c->reason="Charging already enabled";return -1;}
    if(mv>=(int)p->mv){c->reason="Voltage already at limit";return -1;}
    c->limits=*p;c->start=c->last=c->rise=now;c->peak=mv;
    c->active=1;c->reason="Test cycle running";return 0;
}
int charge_step(struct charge_cycle *c,unsigned long now,int mv,unsigned f,int valid) {
    const char *why;if(!c->active)return 0;
    why=unsafe(mv,f,valid);
    if(!why && (now<c->last || now-c->last>15))why="Clock / polling gap";
    if(!why && !(f&8))why="Firmware disabled charging";
    if(!why && now-c->start>=c->limits.minutes*60UL)why="Time limit reached";
    if(!why && mv>=(int)c->limits.mv)why="Voltage limit reached";
    if(!why && mv>c->peak){c->peak=mv;c->rise=now;}
    if(!why && now-c->rise>=c->limits.stall*60UL)why="Voltage stall limit reached";
    c->last=now;
    if(why){c->active=0;c->reason=why;return 1;}return 0;
}
