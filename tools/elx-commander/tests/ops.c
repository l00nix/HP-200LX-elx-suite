/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "elxcmd.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
volatile int stopping;
static struct panel panel;
int main(int argc,char **argv) {
    int rc=-1;
    if(argc<3)return 2;
    if(!strcmp(argv[1],"copy") && argc==4)rc=copyfile(argv[2],argv[3],0);
    else if(!strcmp(argv[1],"move") && argc==4)rc=copyfile(argv[2],argv[3],1);
    else if(!strcmp(argv[1],"cancel") && argc==4){stopping=1;rc=copyfile(argv[2],argv[3],0);}
    else if(!strcmp(argv[1],"delete"))rc=removeitem(argv[2]);
    else if(!strcmp(argv[1],"list")) {
        if(canonical(panel.path,argv[2])<0)return 1;
        rc=loadpanel(&panel);
        printf("count=%d limited=%d failed=%d\n",panel.count,panel.limited,panel.failed);
        if(!rc){int i;for(i=0;i<panel.count;i++)puts(panel.entries[i].name);}
    } else if(!strcmp(argv[1],"key")){printf("%d\n",decodekey(argv[2]));return 0;}
    else return 2;
    if(rc<0){perror("operation");return 1;}return 0;
}
