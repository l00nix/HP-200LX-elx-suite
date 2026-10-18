/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "cga.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
static volatile int running=1;
static void stop(int sig) {(void)sig;running=0;}
static void help(void) {
    puts("ELXbat " ELX_BAT_VERSION " - HP 200LX battery voltage gauges\n"
         "Usage: elxbat [--graphics | --text] [--watch] [--interval SECONDS]\n"
         "              [--image FILE]\n"
         "       elxbat --help | --version\n"
         "Graphics: light theme, standalone CGA; no Nano-X. Q/Esc quits; R rereads.\n"
         "Text: one snapshot; --watch repeats. Default interval: 5 seconds.\n"
         "Graphics power status: polls firmware cache every second; bolt = active charging.\n"
         "--image reads an OFFLINE 1 MiB physical-memory image.\n"
         "Readings are cached volts, NOT charge percentages. Battery settings unchanged.");
#if !defined(__ia16__) || defined(ELX_CGA_TEST)
    puts("Test build: --preview FILE.pbm renders the actual 640x200 framebuffer.");
#endif
}
#if defined(__ia16__) && !defined(ELX_CGA_TEST)
static void stamp(char *buf) {
    time_t t=time(0);struct tm *tm=localtime(&t);
    if(tm)strftime(buf,24,"%Y-%m-%d %H:%M:%S",tm);else strcpy(buf,"Clock unavailable");
}
#endif
int main(int argc,char **argv) {
    int i,textmode=0,watch=0,explicit_graphics=0,result=0;
    unsigned interval=5;
    const char *image=0;
    struct elx_reader reader;struct elx_status s;
#if !defined(__ia16__) || defined(ELX_CGA_TEST)
    const char *preview=0;
#else
    time_t last,last_power,cur;char now[24];int power_stale=0;
    struct elx_status next;
#endif
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")) {help();return 0;}
        if(!strcmp(argv[i],"--version")) {puts("ELXbat " ELX_BAT_VERSION);return 0;}
        if(!strcmp(argv[i],"--text")) {textmode=1;continue;}
        if(!strcmp(argv[i],"--graphics")) {explicit_graphics=1;continue;}
        if(!strcmp(argv[i],"--watch")) {watch=1;continue;}
        if(!strcmp(argv[i],"--image") && i+1<argc && !image) {image=argv[++i];continue;}
        if(!strcmp(argv[i],"--interval") && i+1<argc) {
            char *end;long n=strtol(argv[++i],&end,10);
            if(!*argv[i] || *end || n<1 || n>3600) {fputs("Interval must be 1..3600 seconds.\n",stderr);return 2;}
            interval=(unsigned)n;continue;
        }
#if !defined(__ia16__) || defined(ELX_CGA_TEST)
        if(!strcmp(argv[i],"--preview") && i+1<argc && !preview) {preview=argv[++i];continue;}
#endif
        fprintf(stderr,"Unknown/incomplete argument: %s\n",argv[i]);help();return 2;
    }
    if(textmode && explicit_graphics) {fputs("Choose --text or --graphics.\n",stderr);return 2;}
    if(elx_open(&reader,image)<0) {perror("ELXbat: open/identify HP 200LX");return 1;}
    if(elx_read(&reader,&s)<0) {perror("ELXbat: read stable firmware cache");elx_close(&reader);return 1;}
    signal(SIGINT,stop);signal(SIGTERM,stop);signal(SIGHUP,stop);signal(SIGQUIT,stop);
    if(textmode) {
        do {
            elx_text_bars(&s);fflush(stdout);
            if(!watch)break;
            for(i=0;i<(int)interval && running;i++)sleep(1);
            if(!running)break;
            if(elx_read(&reader,&s)<0) {perror("ELXbat: cache read failed");result=1;break;}
        } while(running);
        elx_close(&reader);return result;
    }
#if !defined(__ia16__) || defined(ELX_CGA_TEST)
    if(!preview || !image) {fputs("Test graphics require --image FILE --preview FILE.pbm, or use --text.\n",stderr);elx_close(&reader);return 2;}
    elx_draw(&elx_cga_canvas,&s,"OFFLINE PREVIEW");
    if(elx_cga_save(preview)<0) {perror(preview);result=1;}
#else
    if(!explicit_graphics) {
        char *tty=ttyname(0);
        if(!tty || strstr(tty,"ttyS")) {
            fputs("Use --text on serial; run graphics from the local HP console.\n",stderr);
            elx_close(&reader);return 2;
        }
    }
    if(elx_console_open()<0) {perror("ELXbat: acquire CGA console");elx_close(&reader);return 1;}
    last=last_power=time(0);stamp(now);elx_draw(&elx_cga_canvas,&s,now);
    while(running) {
        int key=elx_console_key(),refresh=0;
        if(key<0) {result=1;break;}
        if(key=='q' || key=='Q' || key==27 || key==3)break;
        if(key=='r' || key=='R')refresh=1;
        cur=time(0);
        if(cur<last || cur-last>=(time_t)interval)refresh=1;
        if(refresh) {
            last=last_power=cur;
            if(elx_read(&reader,&next)<0) {
                power_stale=1;elx_draw_power(&elx_cga_canvas,&s,1);
                elx_cga_canvas.rect(0,181,640,19,0);
                elx_cga_canvas.text(8,183,"READ ERROR: battery values stale | Retrying | Q: quit R: reread",0);
            } else {
                s=next;power_stale=0;stamp(now);elx_draw(&elx_cga_canvas,&s,now);
            }
        } else if(cur<last_power || cur-last_power>=1) {
            unsigned char flags=0;int stale,changed;
            last_power=cur;stale=elx_read_power(&reader,&flags)<0;
            changed=stale!=power_stale || (!stale && ((flags^s.bda[0xd4])&ELX_POWER_MASK));
            if(!stale)s.bda[0xd4]=(s.bda[0xd4]&~ELX_POWER_MASK)|(flags&ELX_POWER_MASK);
            power_stale=stale;
            if(changed)elx_draw_power(&elx_cga_canvas,&s,power_stale);
        }
        elx_console_wait();
    }
    if(elx_console_close()<0) {perror("ELXbat: console restore");result=1;}
    if(result)fputs("ELXbat stopped after a read/console error.\n",stderr);
#endif
    elx_close(&reader);return result;
}
