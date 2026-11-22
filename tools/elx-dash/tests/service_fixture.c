/* SPDX-License-Identifier: GPL-2.0-or-later
 * Process/IPC fixture: same production supervisor, mocked firmware + fast clock.
 */
#include "dash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
const char *ctl_error;
int dash_charge_test=1;
static unsigned flags=0,seconds=100,off_delay;
static const char *scenario;
static pid_t driver;
static int samples;
static void record(const char *what,unsigned ax,unsigned bx){
    FILE *f=fopen(SESSION "-trace","a");if(f){fprintf(f,"%s %04x %04x\n",what,ax,bx);fclose(f);}
}
unsigned ctl_int15(unsigned ax,unsigned bx) {
    if(ax==0x6003){flags|=0x80;record("type",ax,bx);}
    if(ax==0x6004) {
        record("charge",ax,bx);
        if(bx){flags|=0x38;off_delay=3;}
        else {flags&=~8;if(off_delay){off_delay--;if(!off_delay)flags&=~0x30;}else flags&=~0x30;}
    }
    return flags;
}
int elx_open(struct elx_reader *r,const char *s){(void)s;r->fd=-1;return 0;}
void elx_close(struct elx_reader *r){(void)r;}
int test_snapshot(struct elx_reader *r,int *mv,unsigned *f){
    (void)r;*mv=2500;
    if(++samples==2 && !strcmp(scenario,"handshake")) {
        kill(driver,SIGKILL);usleep(50000); /* only this disposable test driver */
    }
    if(seconds>=104) {
        if(!strcmp(scenario,"unplug"))flags|=0x40;
        if(!strcmp(scenario,"fault"))flags|=0x8000;
        if(!strcmp(scenario,"read"))return -1;
        if(!strcmp(scenario,"voltage"))*mv=3000;
        if(!strcmp(scenario,"veto"))flags&=~8;
    }
    *f=flags;return 0;
}
uid_t test_uid(void){return 0;}
time_t test_time(time_t *p){time_t t=seconds;if(p)*p=t;return t;}
unsigned test_sleep(unsigned n){seconds+=n;usleep(10000);return 0;}
int main(int argc,char **argv) {
    struct charge_limits p={2900,1,1};char line[80];int i,rc;
    if(argc<2)return 2;driver=getpid();scenario=argv[1];
    if(!strcmp(scenario,"existing"))flags=0xb8;
    if(!strcmp(scenario,"locked"))dash_charge_test=0;
    rc=dash_charge_start(&p);
    if(!strcmp(scenario,"locked")||!strcmp(scenario,"existing"))return rc<0?0:1;
    if(rc<0){fprintf(stderr,"%s\n",ctl_error);return 1;}
    if(!strcmp(scenario,"parent-exit"))return 0;
    if(!strcmp(scenario,"duplicate")) {
        if(dash_charge_start(&p)==0)return 3;
        dash_charge_stop();
    }
    if(!strcmp(scenario,"stop"))dash_charge_stop();
    for(i=0;i<300;i++) {
        usleep(10000);
        if(access(SESSION,F_OK)!=0)break;
    }
    if(i==300)return 4;
    dash_charge_status(line,sizeof(line));puts(line);return 0;
}
