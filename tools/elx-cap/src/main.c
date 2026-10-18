/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "capture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
static volatile int stopped,requested;
static void stop(int sig) { (void)sig; stopped=1; }
static void request(int sig) { (void)sig; requested=1; }
static void help(void) {
    puts("ELXcap 0.1 - HP 200LX screenshots for ELKS\n"
         "Usage: elxcap [options] [new-file.bmp]\n"
         "  -f bmp|pbm       Output format (default BMP, regardless of suffix)\n"
         "  -i               Invert output; default dark ink on light paper\n"
         "  -d SECONDS       Delay capture, 0..3600\n"
         "  -q               Quiet success\n"
         "  --watch DIR      Wait for SIGUSR1; save numbered files in DIR\n"
         "  --hotkey DIR     Alt+period listener (requires optional kernel patch)\n"
         "  --image FILE     Offline physical-memory fixture, not live hardware\n"
         "  --help / --version\n"
         "No filename: create CAP0000.BMP etc. in current directory.\n"
         "Supports CGA mode 6 and full logical 80x25 text pages (modes 2/3).\n"
         "No display mode changes. LCD zoom/inversion/cursor are not reproduced.\n"
         "Background listeners should redirect stdout/stderr to a log file.");
}
int main(int argc,char **argv) {
    const char *image=0,*path=0,*dir=0;
    int i,fd,pbm=0,invert=0,quiet=0,hotkey=0,result=0;
    unsigned delay=0,event=0,last=0;
    char output[240];
    struct stat st;
    for(i=1;i<argc;i++) {
        const char *s=argv[i];
        if(!strcmp(s,"--help") || !strcmp(s,"-h")) { help(); return 0; }
        if(!strcmp(s,"--version")) { puts("ELXcap 0.1"); return 0; }
        if(!strcmp(s,"-i")) invert=1;
        else if(!strcmp(s,"-q")) quiet=1;
        else if(!strcmp(s,"-f") && i+1<argc) {
            s=argv[++i]; if(!strcmp(s,"pbm")) pbm=1; else if(strcmp(s,"bmp")) goto usage;
        } else if(!strcmp(s,"-d") && i+1<argc) {
            unsigned long value; char *end;
            s=argv[++i]; if(!*s || *s<'0' || *s>'9') goto usage;
            value=strtoul(s,&end,10); if(*end || value>3600) goto usage; delay=(unsigned)value;
        } else if(!strcmp(s,"--image") && i+1<argc) image=argv[++i];
        else if((!strcmp(s,"--watch") || !strcmp(s,"--hotkey")) && i+1<argc) {
            if(dir) goto usage;
            hotkey=!strcmp(s,"--hotkey"); dir=argv[++i]; if(!*dir) goto usage;
        } else if(s[0]=='-' || path) goto usage;
        else path=s;
    }
    if((dir && (path || delay)) || (hotkey && image) || (path && strlen(path)>230)) goto usage;
    if(dir) {
        cap_error="output directory must already exist";
        if(strlen(dir)>220) { errno=ENAMETOOLONG; goto error; }
        if(stat(dir,&st)<0) goto error;
        if(!S_ISDIR(st.st_mode)) { errno=ENOTDIR; goto error; }
    }
    signal(SIGINT,stop); signal(SIGTERM,stop); signal(SIGHUP,stop); signal(SIGUSR1,request);
    fd=cap_open(image); if(fd<0) goto error;
    while(delay-- && !stopped) sleep(1);
    if(stopped) { close(fd); return 0; }
    if(hotkey && ioctl(fd,CAP_LEASE,&last)<0) {
        cap_error="Alt+period unavailable: kernel patch required (use --watch or -d)";
        { int e=errno; close(fd); errno=e; } goto error;
    }
    if(dir && !quiet) { printf("ELXcap PID %d: %s, saving %s in %s\n",getpid(),hotkey?"Alt+period or SIGUSR1":"SIGUSR1",pbm?"PBM":"BMP",dir); fflush(stdout); }
    if(!dir) requested=1;
    do {
        if(hotkey) {
            if(ioctl(fd,CAP_LEASE,&event)<0) {
                fprintf(stderr,"ELXcap: capture hotkey lease lost: %s\n",strerror(errno)); result=-1; break;
            }
            if(event!=last) { last=event; requested=1; }
        }
        if(requested) {
            requested=0;
            result=cap_read(fd);
            if(!result) result=path?cap_save(path,pbm,invert):cap_numbered(dir?dir:".",pbm,invert,output);
            if(result) {
                if(dir) fprintf(stderr,"ELXcap: %s: %s\n",cap_error,strerror(errno));
                else break;
            } else if(!quiet) { printf("Saved %s\n",path?path:output); fflush(stdout); }
        }
        if(dir && !stopped) usleep(100000UL);
    } while(dir && !stopped);
    if(hotkey) ioctl(fd,CAP_RELEASE,&event);
    { int e=errno; close(fd); errno=e; }
    if(result && !dir) goto error;
    return result?1:0;
error:
    fprintf(stderr,"ELXcap: %s: %s\n",cap_error,strerror(errno)); return 1;
usage:
    fputs("ELXcap: invalid arguments; use --help\n",stderr); return 2;
}
