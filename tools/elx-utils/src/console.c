/* SPDX-License-Identifier: GPL-2.0-or-later
 * HP console lifecycle adapted from ELKSWeb/platform/elks_platform.c.
 * Uses translated HP keys (as ELKSWEB_COOKED_KEYS=1), never raw scancodes.
 */
#include "cga.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#ifdef ELX_CONSOLE_TEST
#include "console_mock.h"
#else
#include <linuxmt/ntty.h>
static unsigned bios_video(unsigned ax) {
    unsigned bx=0;
    __asm__ volatile (
        "push %%ds\n\tpush %%es\n\tpush %%bp\n\tpush %%si\n\tpush %%di\n\t"
        "int $0x10\n\t"
        "pop %%di\n\tpop %%si\n\tpop %%bp\n\tpop %%es\n\tpop %%ds"
        : "+a"(ax), "+b"(bx) : : "cx","dx","cc","memory");
    return ax;
}
#endif
static int fd=-1,locked,term_saved,old_mode=-1;
static struct termios saved;
static void drain(void) {unsigned char buf[16];int n=16;while(n-- && read(fd,buf,sizeof(buf))>0) {}}
int elx_console_close(void) {
    int result=0,had_video=old_mode>=0;
    if(fd<0)return 0;
    if(term_saved && locked)drain();
    if(had_video) {
        bios_video((unsigned)old_mode);
        if((bios_video(0x0f00)&0x7f)!=(unsigned)old_mode)result=-1;
        old_mode=-1;
    }
    if(locked) {if(ioctl(fd,DCREL_GRAPH,(void *)0)<0)result=-1;locked=0;}
    if(term_saved) {if(tcsetattr(fd,TCSAFLUSH,&saved)<0)result=-1;term_saved=0;}
    /* Mode reset clears pixels; put the kernel cursor at the text origin. */
    if(had_video && write(fd,"\033[H\033[2J",7)!=7)result=-1;
    if(close(fd)<0)result=-1;
    fd=-1;return result;
}
int elx_console_open(void) {
    struct termios term;int mode;
    fd=open("/dev/tty",O_RDWR|O_NONBLOCK);
    if(fd<0)return -1;
    /* Acquire ownership before altering input, so a competing GUI is untouched. */
    if(ioctl(fd,DCGET_GRAPH,(void *)0)<0)goto fail;
    locked=1;
    mode=bios_video(0x0f00)&0x7f;
    if(mode!=2 && mode!=3 && mode!=7 && mode!=0x21) {errno=EBUSY;goto fail;}
    if(tcgetattr(fd,&saved)<0)goto fail;
    term_saved=1;term=saved;
    /* Keep ISIG: Ctrl-C follows the normal cleanup path via our handler. */
    term.c_lflag&=~(ECHO|ECHONL|ICANON|IEXTEN);
    term.c_iflag&=~(ICRNL|INLCR|IGNCR|ISTRIP|IXON|IXOFF|BRKINT);
    term.c_cc[VMIN]=0;term.c_cc[VTIME]=0;
    if(tcsetattr(fd,TCSAFLUSH,&term)<0)goto fail;
    drain();old_mode=mode;
    bios_video(6);
    if((bios_video(0x0f00)&0x7f)!=6) {errno=EIO;goto fail;}
    return 0;
fail:
    {int err=errno;elx_console_close();errno=err;return -1;}
}
int elx_console_key(void) {
    unsigned char ch;int n=read(fd,&ch,1);
    if(n==1)return ch;
    if(n<0 && errno!=EAGAIN && errno!=EINTR)return -1;
    return 0;
}
void elx_console_wait(void) {usleep(50000UL);}
