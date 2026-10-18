/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "elx.h"
#include <stdio.h>
#include <string.h>
unsigned elx_word(const unsigned char *p) { return p[0] | ((unsigned)p[1] << 8); }
int elx_mv(unsigned raw) { return raw > 63 ? -1 : 1570 + (int)(1730UL * raw / 63); }
unsigned elx_bar(int mv,unsigned width) {
    if (mv <= 2000) return 0;
    if (mv >= 3500) return width;
    return (unsigned)((unsigned long)(mv-2000)*width/1500);
}
/* HP documents Timer1 units as 55 ms. Avoid 32-bit multiplication overflow. */
unsigned long elx_seconds(unsigned long ticks) {
    return ticks / 200 * 11 + ticks % 200 * 11 / 200;
}
const char *elx_type(const struct elx_status *s) { return s->bda[0xd4]&0x80 ? "NiCad (configured)" : "Alkaline (configured)"; }
const char *elx_source(const struct elx_status *s) { return s->bda[0xd4]&0x40 ? "battery" : "external"; }
const char *elx_charging(const struct elx_status *s) {
    unsigned f=s->bda[0xd4];
    if(f&0x10) return f&0x20 ? "active (fast)" : "active";
    return f&8 ? "enabled, inactive" : "disabled";
}
void elx_voltage(char *buf,unsigned raw) {
    int mv=elx_mv(raw);
    if(mv<0) strcpy(buf,"unavailable");
    else sprintf(buf,"%d.%03d V",mv/1000,mv%1000);
}
void elx_battery(const struct elx_status *s) {
    char mainv[20],backupv[20];
    unsigned long secs=elx_seconds(s->charge_ticks);
    unsigned ms=elx_word(s->bda+0xd0);
    elx_voltage(mainv,s->bda[0x9a]); elx_voltage(backupv,s->bda[0x9b]);
    printf("Main battery      %s  %s\n",mainv,elx_type(s));
    printf("Backup battery    %s\n",backupv);
    printf("Firmware alerts   main=%s backup=%s card=%s\n",s->bda[0xd4]&1?"LOW":"clear",
        s->bda[0xd4]&2?"LOW":"clear",s->bda[0xd4]&4?"LOW":"clear");
    printf("Power source      %s\nCharging status   %s\n",elx_source(s),elx_charging(s));
    printf("Charging timer    %02lu:%02lu:%02lu (nominal 55 ms ticks)\n",secs/3600,secs/60%60,secs%60);
    printf("Battery interval  %lu.%03lu seconds%s\n",ms*55UL/1000,ms*55UL%1000,ms?"":" (disabled)");
    printf("Data source       %sfirmware cache; sample age unknown\n",s->offline?"OFFLINE IMAGE / ":"");
}
void elx_text_bars(const struct elx_status *s) {
    int row,mv; unsigned i,n; char v[20];
    printf("ELXbat %s  %s\n",ELX_BAT_VERSION,s->offline?"OFFLINE IMAGE":"HP 200LX");
    printf("%s | %s | charging %s\n",elx_type(s),elx_source(s),elx_charging(s));
    puts("Voltage scale: 2.0 V to 3.5 V (not charge percentage)");
    for(row=0;row<2;row++) {
        unsigned raw=s->bda[0x9a+row];
        mv=elx_mv(raw); elx_voltage(v,raw); n=elx_bar(mv,40);
        printf("%-6s [",row?"Backup":"Main");
        for(i=0;i<40;i++) putchar(mv<0?'?':(i<n?'#':' '));
        printf("] %s%s\n",v,mv>=0 && mv<2000?" (below scale)":"");
    }
    printf("Firmware flags: main=%s backup=%s\n",elx_alert(s,0),elx_alert(s,1));
    printf("Nominal BIOS markers: main low %d mV; critical 2000 mV; ",elx_low_mv(s,0));
    if(elx_low_mv(s,1)>=0)printf("backup low %d mV\n",elx_low_mv(s,1));
    else puts("backup threshold unverified");
    puts("Firmware-cached readings; age unknown. Ctrl-C stops --watch.");
}

/* Nominal markers, not a BIOS threshold getter. ROM 2.14's comparisons
 * use adjusted ADC codes <23 alkaline, <27 NiCad, <15 critical, <41 backup.
 * Clear codes are >=32 main and >=48 backup. See THRESHOLDS.md. */
int elx_low_mv(const struct elx_status *s,int backup) {
    if(!backup) return (s->bda[0xd4]&0x80)?2300:2200;
    return !strcmp(s->rom,"ROM BIOS Ver 2.14")?2700:-1;
}
const char *elx_alert(const struct elx_status *s,int backup) {
    if(!backup && (s->bda[0xd5]&0x40))return "VERY LOW";
    return (s->bda[0xd4]&(backup?2:1))?"LOW":"clear";
}
