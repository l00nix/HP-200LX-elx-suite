/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "elx.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <termios.h>

static void help(void) {
    puts("ELXstat " ELX_VERSION " - HP 200LX system status for ELKS\n"
         "Usage: elxstat [-v] [battery|power|display|serial|system] [--image FILE]\n"
         "       elxstat --help | --version\n"
         "Default: battery/power summary. -v: all supported sections.\n"
         "--image reads an OFFLINE 1 MiB physical-memory image, never live hardware.\n"
         "0.1 reports firmware-cached data. Sample age is unknown. No settings changed.");
}
static void display(const struct elx_status *s) {
    printf("Video mode        0x%02x (firmware)\nZoom mode         0x%02x\n",
        s->bda[0x49],s->bda[0x9f]);
    printf("BIOS text geometry %u columns x %u rows\nDisplay page      %u\n",
        elx_word(s->bda+0x4a),(unsigned)s->bda[0x84]+1,s->bda[0x62]);
    printf("Annunciators      %s, %s corner\n",s->bda[0xeb]&0x80?"disabled":"enabled",
        s->bda[0xeb]&0x40?"right":"left");
    puts("Contrast/frame rate: unavailable in 0.1 (no indexed-I/O writes).");
}
static void power(const struct elx_status *s) {
    unsigned n=elx_word(s->bda+0xa3);
    printf("Firmware timeout  %lu.%03lu seconds (not ELKS sleep policy)\n",n*55UL/1000,n*55UL%1000);
    printf("Firmware light sleep %s in BIOS keyboard polling\n",s->bda[0xd6]&1?"avoided":"allowed");
    printf("Power-on alarm    %s (firmware flag)\n",s->bda[0xc3]&2?"enabled":"disabled");
    printf("Power error flags very-low=%s overvoltage=%s\n",s->bda[0xd5]&0x40?"yes":"no",
        s->bda[0xd5]&0x80?"yes":"no");
}
static unsigned long baud_value(speed_t code) {
    switch(code) {
    case B0:return 0;case B50:return 50;case B75:return 75;
    case B110:return 110;case B134:return 134;case B150:return 150;
    case B200:return 200;case B300:return 300;case B600:return 600;
    case B1200:return 1200;case B1800:return 1800;case B2400:return 2400;
    case B4800:return 4800;case B9600:return 9600;case B19200:return 19200;
    case B38400:return 38400;
#ifdef B57600
    case B57600:return 57600;
#endif
#ifdef B115200
    case B115200:return 115200;
#endif
    default:return 0;
    }
}
static void serial(const struct elx_status *s) {
    struct termios t;
    char *name;
    printf("COM1 BIOS address 0x%04x\n",elx_word(s->bda));
    if(!s->offline && (name=ttyname(0))!=0 && strstr(name,"ttyS") && tcgetattr(0,&t)==0) {
        const char *parity=t.c_cflag&PARENB ? (t.c_cflag&PARODD?"O":"E") : "N";
        unsigned bits=5;
        switch(t.c_cflag&CSIZE) { case CS6:bits=6;break; case CS7:bits=7;break; case CS8:bits=8;break; }
        printf("Current serial tty %s: %lu,%s,%u,%u\n",name,
            baud_value(cfgetospeed(&t)),parity,bits,t.c_cflag&CSTOPB?2:1);
        puts("Protocol obtained from the existing stdin descriptor; no UART was opened.");
    } else puts("Live serial protocol unavailable (run on an existing serial shell).");
    puts("UART power/modem lines: unavailable in 0.1; no port reads with side effects.");
}
static void system_info(const struct elx_status *s) {
    struct utsname u;
    printf("HP model/revision 0x%04x (ROM identity signature)\n%s\n%s\nROM date          %s\n",
        s->model,s->version,s->rom,s->date);
    if(!s->offline && uname(&u)==0) printf("Operating system  %s %s %s %s\n",u.sysname,u.release,u.version,u.machine);
    printf("Keyboard delay    %u ms (firmware)\nKeyboard repeat   %u ms (firmware)\n",
        (unsigned)s->bda[0xfc]*33,(unsigned)s->bda[0xfd]*33);
    printf("Raw battery codes main=%u backup=%u reference=%u\n",s->bda[0x9a],s->bda[0x9b],s->bda[0x9c]);
    printf("Raw battery flags %02x %02x; poll countdown=%u\n",s->bda[0xd4],s->bda[0xd5],elx_word(s->bda+0xd2));
}
int main(int argc,char **argv) {
    const char *image=0,*section="battery";
    int i,all=0,selected=0;
    struct elx_reader r; struct elx_status s;
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h") || !strcmp(argv[i],"/?")) { help(); return 0; }
        if(!strcmp(argv[i],"--version")) { puts("ELXstat " ELX_VERSION); return 0; }
        if(!strcmp(argv[i],"-v")) { all=1;continue; }
        if(!strcmp(argv[i],"--image") && i+1<argc && !image) { image=argv[++i];continue; }
        if(!selected && (!strcmp(argv[i],"battery") || !strcmp(argv[i],"power") ||
            !strcmp(argv[i],"display") || !strcmp(argv[i],"serial") || !strcmp(argv[i],"system"))) {
            section=argv[i];selected=1;continue;
        }
        fprintf(stderr,"Unknown/incomplete argument: %s\n",argv[i]); help();return 2;
    }
    if(elx_open(&r,image)<0) { perror("ELXstat: open/identify HP 200LX");return 1; }
    if(elx_read(&r,&s)<0) { perror("ELXstat: read stable firmware cache");elx_close(&r);return 1; }
    elx_close(&r);
    printf("ELXstat %s - %s\n",ELX_VERSION,image?"OFFLINE IMAGE":"HP 200LX");
    if(all || !strcmp(section,"battery") || !strcmp(section,"power")) elx_battery(&s);
    if(all || !strcmp(section,"power")) power(&s);
    if(all || !strcmp(section,"display")) display(&s);
    if(all || !strcmp(section,"serial")) serial(&s);
    if(all || !strcmp(section,"system")) system_info(&s);
    if(strcmp(section,"battery") && strcmp(section,"power") && !all)
        puts("Firmware fields are cached; ELKS may manage these resources independently.");
    return 0;
}
