/* SPDX-License-Identifier: GPL-2.0-or-later
 * Exercise writer handling of short writes, EINTR, disk-full and close failure.
 */
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
static int failure,calls;
static ssize_t test_write(int fd,const void *p,size_t n) {
    ++calls;
    if(calls==1) { errno=EINTR; return -1; }
    if(failure==1 && calls==8) { errno=ENOSPC; return -1; }
    if(failure==2 && calls==8) return 0;
    return write(fd,p,n>7?7:n);
}
static int test_close(int fd) {
    int result=close(fd);
    if(failure==3) { errno=EIO; return -1; }
    return result;
}
#define write test_write
#define close test_close
#include "../src/capture.c"
#undef write
#undef close
int main(void) {
    char dir[]="/tmp/elxcap-faults-XXXXXX",path[240];
    struct stat st;
    assert(mkdtemp(dir));snprintf(path,sizeof(path),"%s/test.bmp",dir);
    assert(!cap_save(path,0,0));assert(!stat(path,&st) && st.st_size==16062);
    assert(!unlink(path));
    for(failure=1;failure<=3;failure++) {
        calls=0;assert(cap_save(path,0,0)==-1);assert(stat(path,&st)==-1 && errno==ENOENT);
    }
    assert(!rmdir(dir));puts("PASS short writes, EINTR, ENOSPC, zero writes, close errors and partial-file cleanup");
    return 0;
}
