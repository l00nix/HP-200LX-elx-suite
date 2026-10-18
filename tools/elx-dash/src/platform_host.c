/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include <time.h>
void demo_init(void);
int demo_read(struct ctl_state *,struct elx_status *);
void demo_call(const struct ctl_plan *);
int dash_open(int demo) {if(!demo)return -1;demo_init();return 0;}
int dash_read(struct ctl_state *s,struct elx_status *b){return demo_read(s,b);}
int dash_read_power(unsigned char *flags){*flags=0x40;return 0;}
void dash_call(const struct ctl_plan *p){demo_call(p);}
void dash_close(void){}
int dash_enter(void){return -1;}
int dash_leave(void){return 0;}
int dash_key(void){return 'q';}
void dash_wait(void){}
unsigned long dash_clock(void){return (unsigned long)time(0);}
