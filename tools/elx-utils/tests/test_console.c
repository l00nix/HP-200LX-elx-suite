/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "cga.h"
#include "console_mock.h"
static int failure,opened,locked,mode,sets,releases,drains,clears,nextkey,mode_sets;
static struct termios original,active;
int mock_open(const char *path,int flags) {
    (void)flags;assert(!strcmp(path,"/dev/tty"));if(failure==1){errno=EIO;return -1;}opened=1;return 9;
}
int mock_close(int fd) {assert(fd==9);opened=0;return 0;}
int mock_ioctl(int fd,int request,void *arg) {
    (void)arg;assert(fd==9);
    if(request==DCGET_GRAPH) {if(failure==2){errno=EBUSY;return -1;}locked=1;return 0;}
    assert(request==DCREL_GRAPH && locked);locked=0;releases++;return 0;
}
int mock_tcgetattr(int fd,struct termios *t) {
    assert(fd==9);if(failure==3){errno=EIO;return -1;}*t=original;return 0;
}
int mock_tcsetattr(int fd,int action,const struct termios *t) {
    (void)action;assert(fd==9);sets++;
    if(failure==4 && sets==1){errno=EIO;return -1;}active=*t;return 0;
}
unsigned mock_bios(unsigned ax) {
    if(ax==0x0f00)return (unsigned)mode;
    mode_sets++;if(failure!=5 || ax!=6)mode=ax;return ax;
}
ssize_t mock_read(int fd,void *buf,size_t n) {
    assert(fd==9);
    if(n!=1){drains++;errno=EAGAIN;return -1;}
    if(nextkey<0){errno=EIO;return -1;}
    if(nextkey){*(unsigned char *)buf=nextkey;nextkey=0;return 1;}
    errno=EAGAIN;return -1;
}
ssize_t mock_write(int fd,const void *buf,size_t n) {assert(fd==9 && n==7 && !memcmp(buf,"\033[H\033[2J",7));clears++;return n;}
int mock_usleep(unsigned long usec) {assert(usec==50000);return 0;}
static void reset(int f) {
    failure=f;opened=locked=sets=releases=drains=clears=nextkey=mode_sets=0;mode=3;
    memset(&original,0,sizeof(original));original.c_lflag=ECHO|ICANON|ISIG|IEXTEN;
    original.c_iflag=ICRNL|IXON;original.c_cc[VMIN]=1;active=original;
}
int main(void) {
    int f;
    reset(0);assert(elx_console_open()==0 && opened && locked && mode==6);
    assert(!(active.c_lflag&(ECHO|ICANON)) && (active.c_lflag&ISIG));
    nextkey='q';assert(elx_console_key()=='q');assert(elx_console_key()==0);
    nextkey=-1;assert(elx_console_key()==-1);elx_console_wait();
    assert(elx_console_close()==0 && !opened && !locked && mode==3 && clears==1);
    assert(!memcmp(&active,&original,sizeof(active)));assert(elx_console_close()==0);
    for(f=1;f<=5;f++) {
        reset(f);assert(elx_console_open()==-1);assert(!opened && !locked && mode==3);
        assert(!memcmp(&active,&original,sizeof(active)));
        if(f==2)assert(sets==0 && drains==0 && mode_sets==0);
    }
    reset(0);mode=6;assert(elx_console_open()==-1);assert(!opened && !locked && mode_sets==0 && sets==0 && drains==0);
    puts("PASS: console acquisition, translated keys, nonblocking input, text/termios restore, busy owner untouched, all startup failure cleanup");
    return 0;
}
