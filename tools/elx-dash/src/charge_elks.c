/* SPDX-License-Identifier: GPL-2.0-or-later
 * Experimental native supervisor. No automatic start or restart. The child
 * owns the session lock and remains alive until firmware reports charging off.
 */
#include "dash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#ifndef SESSION
#define SESSION "/var/run/elxchg"
#endif
#define LAST SESSION "-last"
#define STATUS SESSION "/status"
#define STOP SESSION "/stop"
static volatile int stopping;
static void interrupted(int sig){(void)sig;stopping=1;}
static int publish(const char *s) {
    int fd=open(SESSION "/new",O_WRONLY|O_CREAT|O_TRUNC,0600),n;
    if(fd<0)return -1;
    n=write(fd,s,strlen(s));close(fd);
    if(n!=(int)strlen(s))return -1;
    return rename(SESSION "/new",STATUS);
}
#ifdef DASH_SERVICE_TEST
extern int test_snapshot(struct elx_reader *,int *,unsigned *);
#define snapshot test_snapshot
#else
static int snapshot(struct elx_reader *r,int *mv,unsigned *flags) {
    struct elx_status b;unsigned ax=0x6000,bx=0;
    if(elx_read(r,&b)<0)return -1;
    /* Capture CF before restoring caller flags; preserve all firmware-clobbered
     * segment/index registers. No cached voltage used for charge termination. */
    __asm__ volatile(
        "pushf\n\tcli\n\tpush %%ds\n\tpush %%es\n\tpush %%bp\n\tpush %%si\n\tpush %%di\n\t"
        "int $0x15\n\tsbb %%bx,%%bx\n\tcli\n\t"
        "pop %%di\n\tpop %%si\n\tpop %%bp\n\tpop %%es\n\tpop %%ds\n\tpopf"
        : "+a"(ax),"+b"(bx) : : "cx","dx","cc","memory");
    if(bx || ax>63)return -1;
    *mv=elx_mv(ax);*flags=ctl_int15(0x6002,0);return 0;
}
#endif
int dash_charge_available(void) {
    ctl_error="Start locked: launch with --charge-test for supervised testing";
    return dash_charge_test;
}
int dash_abc_open(void){ctl_error="ABC/LX is DOS only; use the ELKS test controls below";return -1;}
int dash_charge_status(char *s,unsigned size) {
    int fd,n,active=1;struct stat st;time_t now;
    if(!size)return -1;
    fd=open(STATUS,O_RDONLY);
    if(fd<0){active=0;fd=open(LAST,O_RDONLY);}
    if(fd<0){strncpy(s,"No ELKS test session",size);s[size-1]=0;return 0;}
    now=time(0);
    if(active && (fstat(fd,&st)<0 || now==(time_t)-1 || now<st.st_mtime || now-st.st_mtime>10)) {
        close(fd);strncpy(s,"STALE supervisor: disconnect external power",size);s[size-1]=0;return -1;
    }
    n=read(fd,s,size-1);close(fd);
    if(n<0){strncpy(s,"Supervisor status read error",size);s[size-1]=0;return -1;}
    s[n]=0;return 0;
}
int dash_charge_stop(void) {
    int fd;struct stat st;
    ctl_error="No active supervisor session; charging unchanged";
    if(stat(SESSION,&st)<0)return -1;
    ctl_error="Cannot request supervisor stop";
    fd=open(STOP,O_WRONLY|O_CREAT|O_EXCL,0600);
    if(fd<0){if(access(STOP,F_OK)==0)return 0;return -1;}
    close(fd);return 0;
}
static void supervise(int reply,const struct charge_limits *p) {
    struct elx_reader r;struct charge_cycle c;int mv=0,ok=0,owned=0;
    unsigned flags=0;char answer='E',line[80];const char *why="Supervisor startup failed";
    unsigned long now;
    r.fd=-1;
    signal(SIGPIPE,SIG_IGN);signal(SIGHUP,SIG_IGN);signal(SIGINT,interrupted);signal(SIGTERM,interrupted);signal(SIGQUIT,interrupted);
    if(setsid()==(pid_t)-1 || elx_open(&r,0)<0)goto done;
    if(snapshot(&r,&mv,&flags)<0)goto done;
    if(flags&0x38){why="Existing charging must be stopped first";goto done;}
    if(flags&(0x40|0xc000)){why="External power / firmware fault blocks start";goto done;}
    /* The UI's explicit NiCd confirmation authorizes this chemistry change.
     * Do not restore alkaline on exit: that would misdescribe installed cells. */
    ctl_int15(0x6004,0);ctl_int15(0x6003,1);
    if(snapshot(&r,&mv,&flags)<0)goto done;
    now=(unsigned long)time(0);
    if(now==(unsigned long)(time_t)-1){why="Clock unavailable";goto done;}
    if(charge_begin(&c,p,now,mv,flags)<0){why=c.reason;goto done;}
    if(stopping || access(STOP,F_OK)==0){why="Cancelled before enable";goto done;}
    if(publish("Starting bounded NiCd test")<0)goto done;
    owned=1;ctl_int15(0x6004,1);
    if(!(ctl_int15(0x6002,0)&8)){why="Charging enable read-back failed";goto done;}
    answer='O';if(write(reply,&answer,1)!=1){why="Dashboard handshake lost";goto done;}close(reply);reply=-1;
    while(!stopping) {
        if(access(STOP,F_OK)==0){why="Stopped by user";break;}
        now=(unsigned long)time(0);
        ok=snapshot(&r,&mv,&flags)==0 && now!=(unsigned long)(time_t)-1;
        if(charge_step(&c,now,mv,flags,ok)){why=c.reason;break;}
        sprintf(line,"TEST %lum/%um | %d.%03dV | limit %u.%03uV",(now-c.start)/60,p->minutes,mv/1000,mv%1000,p->mv/1000,p->mv%1000);
        if(publish(line)<0){why="Status write failed";break;}
        sleep(1);
    }
    if(stopping)why="Supervisor interrupted";
done:
    if(owned) {
        /* UCHG can clear before hardware ECHG/FCHG clears. Keep requesting off
         * until firmware confirms all three bits off; never release lock early. */
        do {
            ctl_int15(0x6004,0);flags=ctl_int15(0x6002,0);
            if(flags&0x38){publish("STOPPING: waiting for firmware charging off");sleep(1);}
        } while(flags&0x38);
    }
    publish(why);
    if(reply>=0){write(reply,&answer,1);close(reply);}
    if(r.fd>=0)elx_close(&r);
    /* Keep a final result separately; remove active lock only after off. */
    unlink(LAST);rename(STATUS,LAST);
    unlink(STOP);unlink(SESSION "/new");rmdir(SESSION);
    _exit(0);
}
int dash_charge_start(const struct charge_limits *p) {
    int pipes[2],pid,status,got;char reply='E';
    if(!dash_charge_available())return -1;
    ctl_error="Invalid test limits";if(!charge_limits_valid(p))return -1;
    ctl_error="Root required for the charging supervisor";if(geteuid()!=0)return -1;
    ctl_error="Session exists or /var/run is unavailable; inspect before retry";
    if(mkdir(SESSION,0700)<0)return -1;
    ctl_error="Unable to start supervisor";
    if(pipe(pipes)<0){rmdir(SESSION);return -1;}
    pid=fork();
    if(pid==0) {
        int child;close(pipes[0]);child=fork();
        if(child<0)_exit(1);
        if(child>0)_exit(0);
        close(0);close(1);close(2);supervise(pipes[1],p);
    }
    close(pipes[1]);
    if(pid<0){close(pipes[0]);rmdir(SESSION);return -1;}
    got=read(pipes[0],&reply,1);close(pipes[0]);waitpid(pid,&status,0);
    if(got==1 && reply=='O')return 0;
    ctl_error="Start refused; check /var/run/elxchg-last";return -1;
}
