/* SPDX-License-Identifier: GPL-2.0-or-later
 * Uses ELKS's existing ANSI text-console protocol, including ESC 1..0 Fn keys.
 * No BIOS calls, framebuffer access, global key hooks or graphics ownership.
 */
#include "elxcmd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
static struct termios saved;
static int active,inputfd=-1,have_saved;
static void stopped(int sig){(void)sig;stopping=1;}
void at(int row,int col){printf("\033[%d;%dH",row+1,col+1);}
void field(int row,int col,int width,const char *s,int reverse) {
    int i;at(row,col);fputs(reverse?"\033[0;7m":"\033[0m",stdout);
    for(i=0;i<width;i++) {
        unsigned char c=*s?(unsigned char)*s++:' ';
        putchar(c>=32 && c<127?c:'?');
    }
    fputs("\033[0m",stdout);
}
void terminal_close(void) {
    if(!active)return;
    fputs("\033[0m\033[?25h\033[H\033[2J",stdout);fflush(stdout);
    tcsetattr(0,TCSAFLUSH,&saved);close(inputfd);inputfd=-1;active=0;
}
int terminal_open(void) {
    struct termios t;
    if(!isatty(0)||!isatty(1)){errno=ENOTTY;return -1;}
#ifdef TIOCGWINSZ
    {struct winsize ws;if(ioctl(1,TIOCGWINSZ,&ws)==0 && ws.ws_col && ws.ws_row && (ws.ws_col<80 || ws.ws_row<25)){errno=EINVAL;return -1;}}
#endif
    if(!have_saved){if(tcgetattr(0,&saved)<0)return -1;have_saved=1;}
    /* A separate open description keeps stdout blocking even when stdin and
     * stdout originally share one terminal descriptor (as on a shell/PTY). */
    inputfd=open("/dev/tty",O_RDONLY|O_NONBLOCK);if(inputfd<0)return -1;
    t=saved;t.c_lflag&=~(ECHO|ECHONL|ICANON|IEXTEN);t.c_lflag|=ISIG;
    t.c_iflag&=~(ICRNL|INLCR|IGNCR|ISTRIP|IXON|IXOFF);
    t.c_cc[VMIN]=1;t.c_cc[VTIME]=0;
    if(tcsetattr(0,TCSAFLUSH,&t)<0){close(inputfd);inputfd=-1;return -1;}
    active=1;signal(SIGINT,stopped);signal(SIGTERM,stopped);signal(SIGHUP,stopped);signal(SIGQUIT,stopped);
    fputs("\033[0m\033[H\033[2J\033[?25l",stdout);return 0;
}
int decodekey(const char *s) {
    int n=(int)strlen(s),v;static const int vt[]={11,12,13,14,15,17,18,19,20,21};
    if(n==1)return (unsigned char)*s;
    if(s[0]!=27)return 0;
    if(n==2) {
        if(s[1]>='1' && s[1]<='9')return K_F1+s[1]-'1';
        if(s[1]=='0')return K_F1+9;
    }
    if(n==3 && (s[1]=='[' || s[1]=='O')) {
        switch(s[2]){case 'A':return K_UP;case 'B':return K_DOWN;case 'C':return K_RIGHT;case 'D':return K_LEFT;case 'H':return K_HOME;case 'F':return K_END;}
        if(s[1]=='O' && s[2]>='P' && s[2]<='S')return K_F1+s[2]-'P';
    }
    if(n==4 && s[1]=='[' && s[2]=='[' && s[3]>='A' && s[3]<='E')return K_F1+s[3]-'A';
    if(n>3 && s[1]=='[' && s[n-1]=='~') {
        v=atoi(s+2);
        if(v==1||v==7)return K_HOME;if(v==4||v==8)return K_END;
        if(v==5)return K_PGUP;if(v==6)return K_PGDN;
        for(n=0;n<10;n++)if(v==vt[n])return K_F1+n;
    }
    return 0;
}
int key(void) {
    char s[24];int n=0,idle=0;ssize_t r;unsigned char c;
    fflush(stdout);
    while(!stopping) {
        r=read(inputfd,&c,1);
        if(r<0 && errno!=EINTR && errno!=EAGAIN){stopping=1;return -1;}
        if(r<=0) {
            if(n && ++idle>=10)return n==1?27:0;
            usleep(20000UL);continue;
        }
        idle=0;
        if(!n && c!=27)return c;
        s[n++]=(char)c;s[n]=0;
        if(n==2 && c!='[' && c!='O')return decodekey(s);
        if(n>=3 && c>=0x40 && c<=0x7e && !(n==3 && c=='['))return decodekey(s);
        if(n>=22)return 0;
    }
    return -1;
}
int prompt(const char *label,char *buf,int cap) {
    int k,n=(int)strlen(buf);
    field(22,0,79,label,1);field(24,0,79,"Enter accepts | Esc cancels | Backspace edits",0);
    while(!stopping) {
        field(23,0,79,buf+n-(n>77?77:n),0);at(23,n>77?77:n);fputs("\033[?25h",stdout);
        k=key();
        if(k==27||k<0){fputs("\033[?25l",stdout);return 0;}
        if(k=='\r'||k=='\n'){fputs("\033[?25l",stdout);return 1;}
        if((k==8||k==127)&&n)buf[--n]=0;
        else if(k>=32 && k<127 && n<cap-1){buf[n++]=(char)k;buf[n]=0;}
    }
    return 0;
}
