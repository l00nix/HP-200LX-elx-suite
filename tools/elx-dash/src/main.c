/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include "cga.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
static volatile int running=1;
static void stop(int sig) {(void)sig;running=0;}
int main(int argc,char **argv) {
    struct dashboard d;int i,demo=0,rc=0,normal=0;
    unsigned long last,now;const char *preview=0,*script="";
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--demo"))demo=1;
        else if(!strcmp(argv[i],"--help")) {
            puts(DASH_NAME " " DASH_VERSION " - HP 200LX battery and control dashboard\n"
                 "Usage: " DASH_NAME " [--demo]\n"
                 "Tab/Up/Down select; Left/Right or +/- adjust; R rereads; Q/Esc quits.\n"
                 "Contrast 0..31; volume 0..3; zoom on exit; icons left/right/off.\n"
                 "BIOS timeout: 30-second steps, Off..3604s. Not ELKS auto-sleep.\n"
                 "Battery triangles: nominal low / solid critical; firmware flags shown.\n"
                 "--demo uses synthetic values and never changes HP settings.");
#ifdef __WATCOMC__
            puts("Open Watcom runtime source is available under the Sybase Open Watcom\n"
                 "Public License: https://github.com/open-watcom/open-watcom-v2\n"
                 "See WATCOM-LICENSE.txt in the distribution.");
#endif
            return 0;
        } else if(!strcmp(argv[i],"--version")) {puts(DASH_NAME " " DASH_VERSION);return 0;}
#if (!defined(__ia16__) && !defined(__WATCOMC__)) || defined(DASH_TEST)
        else if(!strcmp(argv[i],"--preview") && i+1<argc)preview=argv[++i];
        else if(!strcmp(argv[i],"--script") && i+1<argc)script=argv[++i];
#endif
        else {fprintf(stderr,"Unknown argument: %s\n",argv[i]);return 2;}
    }
#if (!defined(__ia16__) && !defined(__WATCOMC__)) || defined(DASH_TEST)
    if(!demo||!preview){fputs("Host build: --demo --preview FILE.pbm [--script KEYS]\n",stderr);return 2;}
#else
    (void)preview;(void)script;
#endif
    if(dash_open(demo)<0){fprintf(stderr,"%s: %s\n",DASH_NAME,ctl_error);return 1;}
    if(dash_init(&d,demo)<0){fprintf(stderr,"%s: %s\n",DASH_NAME,ctl_error);dash_close();return 1;}
#if (!defined(__ia16__) && !defined(__WATCOMC__)) || defined(DASH_TEST)
    for(i=0;script[i];i++)if(dash_key_event(&d,(unsigned char)script[i]))break;
    dash_draw(&d);rc=elx_cga_save(preview)<0;
    printf("DEMO contrast=%u volume=%u zoom=%u pending=%d icons=%u ticks=%u\n",d.state.contrast,d.state.volume,d.zoom,d.zoom_pending,d.state.ann,d.state.ticks);
    dash_close();return rc;
#endif
    signal(SIGINT,stop);signal(SIGTERM,stop);
#ifndef __WATCOMC__
    signal(SIGHUP,stop);signal(SIGQUIT,stop);
#ifdef SIGTSTP
    signal(SIGTSTP,SIG_IGN);
#endif
#endif
    if(dash_enter()<0){fprintf(stderr,"%s: cannot acquire graphics console\n",DASH_NAME);dash_close();return 1;}
    last=dash_clock();dash_draw(&d);
    while(running) {
        int k=dash_key(),changed=0;
        if(k<0){rc=1;break;}
        if(k) {if(dash_key_event(&d,k)){normal=1;break;}changed=1;}
        now=dash_clock();
        if(now<last || now-last>=5){dash_refresh(&d);last=now;changed=1;}
        if(changed)dash_draw(&d);
        dash_wait();
    }
    if(dash_leave()<0){rc=1;fputs("Text console restore failed\n",stderr);}
    else if(dash_finish(&d,normal)<0){rc=1;fprintf(stderr,"Exit: %s\n",ctl_error);}
    dash_close();return rc;
}
