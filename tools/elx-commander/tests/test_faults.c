/* SPDX-License-Identifier: GPL-2.0-or-later
 * Deterministic I/O failures against the real copy implementation. */
#include "elxcmd.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
volatile int stopping;
static int fault,calls;
static const char *source;
static ssize_t faulty_write(int fd,const void *buf,size_t n) {
    if(fault==1 && ++calls==2){errno=ENOSPC;return -1;}
    if(fault==2 && n>7)n=7;
    return write(fd,buf,n);
}
static int faulty_unlink(const char *p) {
    if(fault==3 && !strcmp(p,source)){errno=EACCES;return -1;}
    return unlink(p);
}
#define write faulty_write
#define unlink faulty_unlink
#include "../src/files.c"
#undef write
#undef unlink
static void matches(const char *p,const char *buf,int len) {
    char b[4096];int fd=open(p,O_RDONLY);assert(fd>=0);
    assert(read(fd,b,sizeof(b))==len);assert(!memcmp(buf,b,len));close(fd);
}
int main(void) {
    char dir[]="/tmp/elxcmd-faults-XXXXXX",src[256],dst[256],buf[3072];int fd,i;
    assert(mkdtemp(dir));assert(joinpath(src,dir,"src")==0);assert(joinpath(dst,dir,"dst")==0);source=src;
    for(i=0;i<(int)sizeof(buf);i++)buf[i]=(char)i;
    fd=open(src,O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);assert(write(fd,buf,sizeof(buf))==sizeof(buf));close(fd);
    fault=1;assert(copyfile(src,dst,0)<0);assert(errno==ENOSPC);assert(access(dst,F_OK)<0);matches(src,buf,sizeof(buf));
    fault=2;assert(copyfile(src,dst,0)==0);matches(dst,buf,sizeof(buf));unlink(dst);
    fault=3;assert(copyfile(src,dst,1)<0);assert(errno==EACCES);matches(src,buf,sizeof(buf));matches(dst,buf,sizeof(buf));
    fault=0;unlink(src);unlink(dst);rmdir(dir);
    puts("PASS: disk-full rollback, short writes, move source-removal failure keeps both complete files");return 0;
}
