/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "ctl.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
static void help(void) {
    puts("ELXctl 0.1 - HP 200LX controls for ELKS\n"
         "Usage: elxctl [--dry-run] [--simulate] [command [value]]\n"
         "  status                    Show settings (also the default)\n"
         "  contrast [0..31]          LCD contrast\n"
         "  volume [0..3]             Speaker: off, soft, medium, loud\n"
         "  zoom [80x25|64x18|40x25|40x16]\n"
         "  annunciators [left|right|off]\n"
         "  firmware-timeout [0..3604|off]  Nominal seconds, NOT ELKS auto-sleep\n"
         "No value queries the setting. One setting per command.\n"
         "--dry-run: show planned setting call without applying it.\n"
         "--simulate: isolated demo, no hardware access or persistent changes.\n"
         "Settings are for the current session; no startup file is modified.\n"
         "Serial power, battery charging and LCD power-off are not in 0.1.");
}
static const char *zoom(unsigned z) {
    switch(z) {
    case 2:case 3:case 0x21:return "80x25";
    case 0x80:case 0x81:return "64x18";
    case 0x82:case 0x83:return "40x25";
    case 7:case 0x84:case 0x85:return "40x16";
    default:return "unknown";
    }
}
static void show(int cmd,const struct ctl_state *s) {
    static const char *vol[]={"off","soft","medium","loud"};
    unsigned long ms;
    if(cmd==CT_STATUS || cmd==CT_CONTRAST) printf("Contrast          %u / 31\n",s->contrast);
    if(cmd==CT_STATUS || cmd==CT_VOLUME) printf("Speaker volume    %u (%s)\n",s->volume,vol[s->volume&3]);
    if(cmd==CT_STATUS || cmd==CT_ZOOM) {
        if(s->mode==2 || s->mode==3 || s->mode==7)
            printf("Text zoom         %s (firmware 0x%02X)\n",zoom(s->zoom),s->zoom);
        else puts("Text zoom         not applicable in this video mode");
        printf("Video mode        0x%02X; logical page %u x %u\n",s->mode,s->cols,s->rows);
    }
    if(cmd==CT_STATUS || cmd==CT_ANN) {
        printf("Annunciators      %s (firmware preference)\n",s->ann&0x80?"off":s->ann&0x40?"right":"left");
        puts("                  Visibility depends on firmware sleep/display handling.");
    }
    if(cmd==CT_STATUS || cmd==CT_TIMEOUT) {
        ms=(unsigned long)s->ticks*55;
        if(!s->ticks) puts("Firmware timeout  disabled");
        else printf("Firmware timeout  %lu.%03lu seconds (%u nominal 55 ms ticks)\n",ms/1000,ms%1000,s->ticks);
        puts("                  This does not configure ELKS auto-sleep.");
    }
}
int main(int argc,char **argv) {
    struct ctl_request r;struct ctl_state before,after;struct ctl_plan p;
    if(argc==2 && (!strcmp(argv[1],"--help") || !strcmp(argv[1],"-h"))) { help();return 0; }
    if(argc==2 && !strcmp(argv[1],"--version")) { puts("ELXctl 0.1");return 0; }
    if(ctl_parse(argc,argv,&r)<0) { fputs("ELXctl: invalid command/value; use --help\n",stderr);return 2; }
    if(ctl_open(r.simulate)<0) goto ioerror;
    if(ctl_get(&before)<0) goto ioerror;
    if(r.simulate) puts("ELXctl 0.1 - SIMULATION (no hardware changes)");
    if(!r.set) { show(r.command,&before);ctl_close();return 0; }
    if(ctl_plan(&r,&before,&p)<0) { fprintf(stderr,"ELXctl: %s\n",ctl_error);ctl_close();return 1; }
    if(r.dry) {
        printf("Would call INT %02X AX=%04X BX=%04X; no setting applied.\n",p.vector,p.ax,p.bx);
        ctl_close();return 0;
    }
    ctl_call(&p);
    if(ctl_get(&after)<0) { ctl_error="setting attempted, but read-back failed";goto ioerror; }
    if(!ctl_verified(r.command,&p,&after) || before.mode!=after.mode || before.cols!=after.cols || before.rows!=after.rows) {
        fputs("ELXctl: firmware read-back differs from requested state; inspect current settings.\n",stderr);
        show(CT_STATUS,&after);ctl_close();return 1;
    }
    show(r.command,&after);ctl_close();return 0;
ioerror:
    { int e=errno;ctl_close();fprintf(stderr,"ELXctl: %s: %s\n",ctl_error,strerror(e)); }
    return 1;
}
