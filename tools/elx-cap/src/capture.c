/* SPDX-License-Identifier: GPL-2.0-or-later
 * Original native ELKS implementation; no GEMCAP/CAPLX code or ROM bundled.
 * HP Developer's Guide: BDA video state, F000:FA6E, 9000:FC00, CGA banks.
 * Pixel representation: top-down, MSB first, 1 = ink, 0 = paper.
 */
#include "capture.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifdef __ia16__
#include <sys/utsname.h>
#endif
unsigned char cap_pixels[CAP_BYTES];
const char *cap_error = "capture failed";
static unsigned char cells[4000], font[2048];
static unsigned word(const unsigned char *p) { return p[0] | ((unsigned)p[1]<<8); }
static int at(int fd, unsigned long addr, void *buf, unsigned n) {
    unsigned char *p=buf;
    int got;
    if(lseek(fd,(off_t)addr,SEEK_SET)==(off_t)-1) return -1;
    while(n) {
        got=read(fd,p,n);
        if(got<0 && errno==EINTR) continue;
        if(got<=0) { if(!got) errno=EIO; return -1; }
        p+=got; n-=got;
    }
    return 0;
}
int cap_open(const char *image) {
    int fd, found=0;
    unsigned off,i;
    unsigned char b[256];
    static const unsigned char sig[]={0xbb,0x50,0x48,0xb9,0x02,0x01,0xba};
    cap_error="cannot open HP physical memory";
    if(!image) {
#ifdef __ia16__
        struct utsname u;
        if(uname(&u)<0) return -1;
        if(!strstr(u.version,"hp200lx") && !strstr(u.release,"hp200lx")) {
            cap_error="requires an HP 200LX ELKS kernel"; errno=ENODEV; return -1;
        }
#else
        cap_error="host builds require --image (offline fixture)";
        errno=ENODEV; return -1;
#endif
    }
    fd=open(image?image:"/dev/kmem",O_RDONLY);
    if(fd<0) return -1;
    cap_error="HP 200LX ROM identity not found";
    for(off=0xd000;off<0xe000 && !found;off+=240) {
        if(at(fd,0xf0000UL+off,b,sizeof(b))<0) goto fail;
        for(i=0;i+10<=sizeof(b);i++)
            if(!memcmp(b+i,sig,sizeof(sig)) && b[i+8]==2 && b[i+9]==0xcf) { found=1; break; }
    }
    if(!found) { errno=ENODEV; goto fail; }
    return fd;
fail:
    { int e=errno; close(fd); errno=e; return -1; }
}
int cap_read(int fd) {
    unsigned char b[256], after[256];
    unsigned mode, page, row, col, scan, offset;
    unsigned long base;
    cap_error="could not read screen memory";
    if(at(fd,0x400UL,b,256)<0) return -1;
    mode=b[0x49]&0x7f; page=word(b+0x4e);
    if(mode==6) {
        if(page || b[0x62]) { cap_error="unsupported graphics page"; errno=EINVAL; return -1; }
        /* Read both banks before any disk output. Bank gaps are not pixels. */
        if(at(fd,0xb8000UL,cap_pixels,8000)<0 ||
           at(fd,0xba000UL,cap_pixels+8000,8000)<0) return -1;
        /* Unshuffle banks in place using bounded row cycles, not another
         * 16 KB buffer; each row is 80 bytes. */
        {
            unsigned char done[200], temp[80], swap[80];
            unsigned start, current, next;
            memset(done,0,sizeof(done));
            for(start=0;start<200;start++) {
                if(done[start]) continue;
                memcpy(temp,cap_pixels+start*80,80); current=start;
                do {
                    next=current<100?current*2:(current-100)*2+1;
                    memcpy(swap,cap_pixels+next*80,80);
                    memcpy(cap_pixels+next*80,temp,80);
                    memcpy(temp,swap,80); done[current]=1; current=next;
                } while(current!=start);
            }
        }
    } else if(mode==2 || mode==3) {
        if(word(b+0x4a)!=80 || b[0x84]!=24 || page>12384 || (page&1)) {
            cap_error="text capture requires an 80x25 page"; errno=EINVAL; return -1;
        }
        base=0xb8000UL+page;
        if(at(fd,base,cells,sizeof(cells))<0 ||
           at(fd,0xffa6eUL,font,1024)<0 || at(fd,0x9fc00UL,font+1024,1024)<0) return -1;
        /* Full logical text page, rendered at 8x8 irrespective of LCD zoom.
         * Preserve reverse/hidden text; intensity and blink are flattened.
         * Read the active upper-half code page from firmware RAM. */
        for(row=0;row<25;row++) for(scan=0;scan<8;scan++) for(col=0;col<80;col++) {
            unsigned ch,attr,ink,paper;
            offset=(row*80+col)*2; ch=cells[offset]; attr=cells[offset+1];
            ink=(attr&7)?255:0; paper=(attr&0x70)?255:0;
            cap_pixels[(row*8+scan)*80+col]=(font[ch*8+scan]&ink)|((unsigned char)~font[ch*8+scan]&paper);
        }
    } else {
        cap_error="unsupported mode (use CGA 6 or 80x25 text 2/3)"; errno=EINVAL; return -1;
    }
    if(at(fd,0x400UL,after,256)<0) return -1;
    if(b[0x49]!=after[0x49] || word(b+0x4a)!=word(after+0x4a) ||
       page!=word(after+0x4e) || b[0x62]!=after[0x62] || b[0x84]!=after[0x84]) {
        cap_error="video mode/page changed during capture; retry"; errno=EAGAIN; return -1;
    }
    return 0;
}
static int putall(int fd, const unsigned char *p, unsigned n) {
    int done;
    while(n) {
        done=write(fd,p,n);
        if(done<0 && errno==EINTR) continue;
        if(done<=0) { if(!done) errno=EIO; return -1; }
        n-=done; p+=done;
    }
    return 0;
}
static void le16(unsigned char *p,unsigned n) { p[0]=n; p[1]=n>>8; }
static void le32(unsigned char *p,unsigned long n) { le16(p,(unsigned)n); le16(p+2,(unsigned)(n>>16)); }
int cap_save(const char *path,int pbm,int invert) {
    unsigned char header[62],line[80];
    int fd,result=0,saved;
    unsigned y,x,sy;
    cap_error="cannot create screenshot (existing files are never overwritten)";
    fd=open(path,O_WRONLY|O_CREAT|O_EXCL,0644);
    if(fd<0) return -1;
    cap_error="screenshot write failed";
    if(pbm) result=putall(fd,(const unsigned char *)"P4\n640 200\n",11);
    else {
        memset(header,0,sizeof(header)); header[0]='B'; header[1]='M';
        le32(header+2,16062UL); le32(header+10,62); le32(header+14,40);
        le32(header+18,640); le32(header+22,200); le16(header+26,1); le16(header+28,1);
        le32(header+34,16000); le32(header+46,2);
        header[54]=header[55]=header[56]=255; /* white then black */
        result=putall(fd,header,sizeof(header));
    }
    for(y=0;y<200 && !result;y++) {
        sy=pbm?y:199-y;
        for(x=0;x<80;x++) line[x]=cap_pixels[sy*80+x]^(invert?255:0);
        result=putall(fd,line,80);
    }
    saved=errno;
    if(close(fd)<0 && !result) { result=-1; saved=errno; }
    if(result) unlink(path); /* Only the file successfully created by us. */
    errno=saved; return result;
}
int cap_numbered(const char *dir,int pbm,int invert,char *path) {
    unsigned n;
    cap_error="output directory path too long";
    if(strlen(dir)>220) { errno=ENAMETOOLONG; return -1; }
    for(n=0;n<10000;n++) {
        snprintf(path,240,"%s%sCAP%04u.%s",dir,dir[0] && dir[strlen(dir)-1]=='/'?"":"/",n,pbm?"PBM":"BMP");
        if(cap_save(path,pbm,invert)==0) return 0;
        if(errno!=EEXIST) return -1;
    }
    cap_error="all CAP0000 through CAP9999 filenames already exist";
    errno=EEXIST; return -1;
}
