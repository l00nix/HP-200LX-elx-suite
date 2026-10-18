/* SPDX-License-Identifier: GPL-2.0-or-later
 * Read-only HP BIOS status. No BIOS calls, I/O writes, UART opens or writes
 * to /dev/kmem. HP Developer's Guide: BDA 40:9A/B, D0/D4/D5; 9000:F848.
 * ROM 2.14 inspection confirms BATsys/BATbkup store adjusted sample averages
 * (F000:AAE3 / ABC6), not uncalibrated DAC inputs. No ROM code is bundled.
 */
#include "elx.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifdef __ia16__
#include <sys/utsname.h>
#endif

unsigned elx_word(const unsigned char *p) { return p[0] | ((unsigned)p[1] << 8); }
static int read_at(int fd,unsigned long addr,void *buf,unsigned size) {
    if (lseek(fd,(off_t)addr,SEEK_SET) == (off_t)-1) return -1;
    if (read(fd,buf,size) != (int)size) { errno=EIO; return -1; }
    return 0;
}
static void clean_string(char *s,unsigned n) {
    unsigned i;
    for(i=0;i<n;i++) {
        if ((unsigned char)s[i]<32 || (unsigned char)s[i]>126) break;
    }
    s[i]=0;
}
int elx_open(struct elx_reader *r,const char *image) {
    unsigned char block[256];
    unsigned off,i;
    int found=0;
    static const unsigned char signature[]={0xbb,0x50,0x48,0xb9,0x02,0x01,0xba};
    memset(r,0,sizeof(*r));
    r->fd=-1;
    if(!image) {
#ifdef __ia16__
        struct utsname u;
        if(uname(&u)<0) return -1;
        if(!strstr(u.version,"hp200lx") && !strstr(u.release,"hp200lx")) {
            errno=ENODEV; return -1;
        }
#else
        /* Host builds are offline test tools, never hardware readers. */
        errno=ENODEV; return -1;
#endif
    }
    r->fd=open(image ? image : "/dev/kmem",O_RDONLY);
    if(r->fd<0) return -1;
    r->offline=(image!=0);
    /* Identify the HP detection service's return signature in the ROM.
     * Scan with overlap; require model 02xx and IRET. Refuse unrelated PCs.
     * These are data reads, never execution of firmware from userspace. */
    for(off=0xd000; off<0xe000; off+=240) {
        if(read_at(r->fd,0xf0000UL+off,block,sizeof(block))<0) goto fail;
        for(i=0;i+10<=sizeof(block);i++) {
            if(!memcmp(block+i,signature,sizeof(signature)) &&
               block[i+8]==2 && block[i+9]==0xcf) {
                r->model=elx_word(block+i+7); found=1; break;
            }
        }
        if(found) break;
    }
    if(!found) { errno=ENODEV; goto fail; }
    if(read_at(r->fd,0xfff36UL,r->rom,32)<0 ||
       read_at(r->fd,0xffff5UL,r->date,8)<0 ||
       read_at(r->fd,0xfec5cUL,r->version,24)<0) goto fail;
    clean_string(r->rom,32); clean_string(r->date,8); clean_string(r->version,24);
    if(strncmp(r->rom,"ROM BIOS Ver ",13)) { errno=ENODEV; goto fail; }
    return 0;
fail:
    { int saved=errno; close(r->fd); r->fd=-1; errno=saved; }
    return -1;
}
void elx_close(struct elx_reader *r) { if(r->fd>=0) close(r->fd); r->fd=-1; }
