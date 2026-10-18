/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include "cga.h"
#include <time.h>
#include <string.h>
#include <unistd.h>
static int simulated;
static struct elx_reader reader;
void demo_init(void);
int demo_read(struct ctl_state *,struct elx_status *);
void demo_call(const struct ctl_plan *);
int dash_open(int demo) {
    char *tty=ttyname(0);reader.fd=-1;simulated=demo;
    if(!tty || strstr(tty,"ttyS")) {ctl_error="Run the dashboard on the local HP text console";return -1;}
    if(demo){demo_init();return 0;}
    if(ctl_open(0)<0)return -1;
    if(elx_open(&reader,0)<0){ctl_close();return -1;}return 0;
}
int dash_read(struct ctl_state *s,struct elx_status *b) {
    if(simulated)return demo_read(s,b);
    if(ctl_get(s)<0)return -1;
    return elx_read(&reader,b);
}
void dash_call(const struct ctl_plan *p){if(simulated)demo_call(p);else ctl_call(p);}
int dash_read_power(unsigned char *flags) {
    if(simulated) {*flags=0x40;return 0;}
    return elx_read_power(&reader,flags);
}
void dash_close(void){if(reader.fd>=0)elx_close(&reader);if(!simulated)ctl_close();}
int dash_enter(void){return elx_console_open();}
int dash_leave(void){return elx_console_close();}
int dash_key(void) {
    static int esc=0,idle=0;int k=elx_console_key();
    if(k<0)return k;
    if(!k) {if(esc && ++idle>=4){int single=esc==1;esc=0;return single?27:0;}return 0;}
    idle=0;
    if(!esc){if(k==27){esc=1;return 0;}return k;}
    if(esc==1){if(k=='['||k=='O'){esc=2;return 0;}esc=0;return k;}
    esc=0;
    switch(k){case 'A':return DK_UP;case 'B':return DK_DOWN;case 'C':return DK_RIGHT;case 'D':return DK_LEFT;}
    return 0;
}
void dash_wait(void){elx_console_wait();}
unsigned long dash_clock(void){return (unsigned long)time(0);}
