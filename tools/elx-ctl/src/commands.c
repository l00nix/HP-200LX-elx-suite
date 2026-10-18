/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "ctl.h"
#include <string.h>
#include <stdlib.h>
const char *ctl_error="operation failed";
static int number(const char *s,unsigned max,unsigned *value) {
    unsigned n=0;
    if(!*s) return -1;
    while(*s) {
        unsigned d=(unsigned char)*s++-'0';
        if(d>9 || n>max/10 || (n==max/10 && d>max%10)) return -1;
        n=n*10+d;
    }
    *value=n;return 0;
}
int ctl_parse(int argc,char **argv,struct ctl_request *r) {
    int i,n=0;const char *arg[2];
    memset(r,0,sizeof(*r));
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--simulate")) r->simulate=1;
        else if(!strcmp(argv[i],"--dry-run")) r->dry=1;
        else if(argv[i][0]=='-' || n==2) return -1;
        else arg[n++]=argv[i];
    }
    if(!n) return 0;
    if(!strcmp(arg[0],"status")) return n==1?0:-1;
    if(!strcmp(arg[0],"contrast")) r->command=CT_CONTRAST;
    else if(!strcmp(arg[0],"volume")) r->command=CT_VOLUME;
    else if(!strcmp(arg[0],"zoom")) r->command=CT_ZOOM;
    else if(!strcmp(arg[0],"annunciators")) r->command=CT_ANN;
    else if(!strcmp(arg[0],"firmware-timeout")) r->command=CT_TIMEOUT;
    else return -1;
    if(n==1) return 0;
    r->set=1;
    switch(r->command) {
    case CT_CONTRAST:return number(arg[1],31,&r->value);
    case CT_VOLUME:return number(arg[1],3,&r->value);
    case CT_TIMEOUT:
        if(!strcmp(arg[1],"off")) { r->value=0;return 0; }
        return number(arg[1],3604,&r->value);
    case CT_ZOOM:
        if(!strcmp(arg[1],"80x25")) r->value=0;
        else if(!strcmp(arg[1],"64x18")) r->value=1;
        else if(!strcmp(arg[1],"40x25")) r->value=2;
        else if(!strcmp(arg[1],"40x16")) r->value=3;
        else return -1;
        return 0;
    case CT_ANN:
        if(!strcmp(arg[1],"left")) r->value=0;
        else if(!strcmp(arg[1],"right")) r->value=0x40;
        else if(!strcmp(arg[1],"off")) r->value=0x80;
        else return -1;
        return 0;
    }
    return -1;
}
int ctl_plan(const struct ctl_request *r,const struct ctl_state *s,struct ctl_plan *p) {
    memset(p,0,sizeof(*p));p->vector=0x15;p->expected=r->value;
    switch(r->command) {
    case CT_CONTRAST:p->ax=0x6200;p->bx=r->value;break;
    case CT_VOLUME:p->ax=0x4800|r->value;break;
    case CT_TIMEOUT:
        /* HP documents nominal 55 ms ticks. Round upward, never shorten. */
        p->expected=(unsigned)(((unsigned long)r->value*1000+54)/55);
        p->ax=0x4600;p->bx=p->expected;break;
    case CT_ANN:
        /* Preserve location when disabling; do not alter present-icon bits. */
        if(r->value==0x80) p->expected=0x80|(s->ann&0x40);
        p->ax=0x6120|p->expected;break;
    case CT_ZOOM:
        if(s->cols!=80 || s->rows!=25) { ctl_error="zoom requires a logical 80x25 text console";return -1; }
        if(s->mode==2 || s->mode==3)
            p->expected=r->value?0x80+(r->value-1)*2+(s->mode==3):s->mode;
        else if(s->mode==7 && (r->value==0 || r->value==3)) p->expected=r->value?7:0x21;
        else { ctl_error="zoom is unsupported in this video mode; no mode switch attempted";return -1; }
        p->vector=0x10;p->ax=0xd000|p->expected;break;
    default:ctl_error="no setting requested";return -1;
    }
    return 0;
}
int ctl_verified(int command,const struct ctl_plan *p,const struct ctl_state *s) {
    unsigned actual;
    switch(command) {
    case CT_CONTRAST:actual=s->contrast;break;
    case CT_VOLUME:actual=s->volume;break;
    case CT_TIMEOUT:actual=s->ticks;break;
    case CT_ANN:actual=s->ann&0xc0;break;
    case CT_ZOOM:actual=s->zoom;break;
    default:return 0;
    }
    return actual==p->expected;
}
