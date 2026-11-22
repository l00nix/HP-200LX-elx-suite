#include "cga.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    struct elx_status s,old;char name[64];int i;
    memset(&s,0,sizeof(s));s.bda[0x9a]=39;s.bda[0x9b]=57;
    strcpy(s.rom,"ROM BIOS Ver 2.14");
    elx_draw(&elx_cga_canvas,&s,"2000-01-01 12:00:00");
    for(i=0;i<10;i++) {
        old=s;
        if(i==1)s.bda[0x9a]=30;
        if(i==2)s.bda[0x9b]=45;
        if(i==3)s.bda[0xd4]=0x83;
        if(i==4)s.bda[0xd5]=0x40;
        if(i==5)s.bda[0x9a]=255;
        if(i==6){s.bda[0x9a]=0;s.bda[0x9b]=0;}
        if(i==7){s.bda[0x9a]=63;s.bda[0xd4]=0x58;s.bda[0xd5]=0;}
        if(i==8)strcpy(s.rom,"Unknown");
        if(i==9){elx_draw_error(&elx_cga_canvas,&s);s.offline=1;}
        elx_draw_update(&elx_cga_canvas,&old,&s,"2000-01-01 12:00:00",i==9);
        sprintf(name,"update-%02d.pbm",i);assert(!elx_cga_save(name));
        elx_draw(&elx_cga_canvas,&s,"2000-01-01 12:00:00");
        sprintf(name,"full-%02d.pbm",i);assert(!elx_cga_save(name));
    }
    return 0;
}
