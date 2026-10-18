/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include <string.h>
#include <stdio.h>
static int zoom_index(unsigned z) {
    if(z==2 || z==3 || z==0x21)return 0;
    if(z==0x80 || z==0x81)return 1;
    if(z==0x82 || z==0x83)return 2;
    if(z==0x84 || z==0x85 || z==7)return 3;
    return -1;
}
int dash_init(struct dashboard *d,int demo) {
    int z;memset(d,0,sizeof(*d));d->demo=demo;
    if(dash_read(&d->state,&d->battery)<0)return -1;
    d->entry=d->state;z=zoom_index(d->state.zoom);
    if(z<0 || d->state.cols!=80 || d->state.rows!=25 ||
       (d->state.mode!=2 && d->state.mode!=3 && d->state.mode!=7)) {
        ctl_error="Start from an HP 80x25 logical text console";return -1;
    }
    d->zoom=(unsigned)z;
    strcpy(d->message,"Read-back controls | triangles: low / solid critical");return 0;
}
int dash_refresh(struct dashboard *d) {
    struct ctl_state s;struct elx_status b;
    if(dash_read(&s,&b)<0) {
        d->failed=d->power_failed=1;strcpy(d->message,"READ ERROR: values stale; editing disabled; Q to exit");return -1;
    }
    d->state=s;d->battery=b;
    if(d->failed)strcpy(d->message,"Read recovered; current firmware cache shown");
    d->failed=d->power_failed=0;return 0;
}
int dash_poll_power(struct dashboard *d) {
    unsigned char flags=0;int stale,changed;
    stale=dash_read_power(&flags)<0;
    changed=stale!=d->power_failed || (!stale && ((flags^d->battery.bda[0xd4])&ELX_POWER_MASK));
    if(!stale)d->battery.bda[0xd4]=(d->battery.bda[0xd4]&~ELX_POWER_MASK)|(flags&ELX_POWER_MASK);
    d->power_failed=stale;
    return changed;
}
static int apply(struct dashboard *d,struct ctl_request *r) {
    struct ctl_plan p;struct ctl_state before=d->state;
    if(ctl_plan(r,&d->state,&p)<0)return -1;
    dash_call(&p);
    if(dash_refresh(d)<0)return -1;
    if(!ctl_verified(r->command,&p,&d->state) || before.mode!=d->state.mode ||
        before.cols!=d->state.cols || before.rows!=d->state.rows) {
        strcpy(d->message,"Read-back mismatch: actual setting shown; R to reread");return -1;
    }
    strcpy(d->message,"Setting applied and verified");return 0;
}
int dash_adjust(struct dashboard *d,int direction) {
    struct ctl_request r;int v;
    if(d->failed)return -1;
    if(direction!=1 && direction!=-1)return -1;
    /* Refresh before modifying, so ON+contrast / another tool is respected. */
    if(dash_refresh(d)<0)return -1;
    memset(&r,0,sizeof(r));r.set=1;
    switch(d->selected) {
    case 0:
        r.command=CT_CONTRAST;v=(int)d->state.contrast+direction;
        if(v<0 || v>31)return 0;
        r.value=(unsigned)v;break;
    case 1:
        r.command=CT_VOLUME;v=(int)d->state.volume+direction;
        if(v<0 || v>3)return 0;
        r.value=(unsigned)v;break;
    case 2:
        v=(int)d->zoom;
        if(d->entry.mode==7)v=v?0:3;
        else v=(v+direction+4)%4;
        d->zoom=(unsigned)v;d->zoom_pending=(v!=zoom_index(d->entry.zoom));
        strcpy(d->message,d->zoom_pending?"Text zoom queued for exit; graphics size stays the same":"Original text zoom selected");return 0;
    case 3:
        r.command=CT_ANN;
        v=(d->state.ann&0x80)?2:((d->state.ann&0x40)?1:0);
        v=(v+direction+3)%3;r.value=v==2?0x80:(v?0x40:0);break;
    case 4:
        r.command=CT_TIMEOUT;
        v=(int)((unsigned long)d->state.ticks*55/1000);
        if(direction>0)v=v>=3574?3604:v+30;
        else v=v<=30?0:v-30;
        r.value=(unsigned)v;break;
    default:return -1;
    }
    return apply(d,&r);
}
int dash_finish(struct dashboard *d,int apply_pending) {
    struct ctl_request r;memset(&r,0,sizeof(r));r.command=CT_ZOOM;r.set=1;
    r.value=apply_pending?d->zoom:(unsigned)zoom_index(d->entry.zoom);
    /* Mode reset on exit can reset zoom; restore it even if never edited. */
    if(dash_refresh(d)<0)return -1;
    if(d->state.mode!=d->entry.mode) {ctl_error="Text mode restoration failed";return -1;}
    if(apply(d,&r)<0){ctl_error="Text zoom read-back failed";return -1;}
    return 0;
}
int dash_key_event(struct dashboard *d,int k) {
    if(k=='q'||k=='Q'||k==27||k==3)return 1;
    if(k=='\t'||k==DK_DOWN)d->selected=(d->selected+1)%5;
    else if(k==DK_UP)d->selected=(d->selected+4)%5;
    else if(k==DK_LEFT||k=='-'||k=='h')dash_adjust(d,-1);
    else if(k==DK_RIGHT||k=='+'||k=='='||k=='l')dash_adjust(d,1);
    else if(k=='r'||k=='R') {
        if(dash_refresh(d)==0)strcpy(d->message,"Reread firmware cache; battery sample age unknown");
    }
    return 0;
}
