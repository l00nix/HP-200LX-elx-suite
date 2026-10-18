/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "elxcmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

char message[160];
void error_message(const char *action) {
    snprintf(message,sizeof(message),"%s: %s",action,strerror(errno));
}
int joinpath(char *out,const char *dir,const char *name) {
    unsigned a=strlen(dir),b=strlen(name);
    int slash=a && dir[a-1]!='/';
    if(a+b+slash>=PATH_CAP){errno=ENAMETOOLONG;return -1;}
    memcpy(out,dir,a);if(slash)out[a++]='/';memcpy(out+a,name,b+1);return 0;
}
int canonical(char *out,const char *path) {
    char old[PATH_CAP],resolved[PATH_CAP];int e,ok;
    if(!getcwd(old,sizeof(old)))return -1;
    if(chdir(path)<0)return -1;
    ok=getcwd(resolved,sizeof(resolved))!=NULL;e=errno;
    if(chdir(old)<0)return -1;
    if(!ok){errno=e;return -1;}
    strcpy(out,resolved);return 0;
}
static int compare(const void *aa,const void *bb) {
    const struct entry *a=aa,*b=bb;
    if(!strcmp(a->name,".."))return -1;
    if(!strcmp(b->name,".."))return 1;
    if(S_ISDIR(a->mode)!=S_ISDIR(b->mode))return S_ISDIR(a->mode)?-1:1;
    return strcmp(a->name,b->name);
}
int loadpanel(struct panel *p) {
    DIR *d;struct dirent *de;struct stat st;char path[PATH_CAP],keep[NAME_CAP];int e=0;
    keep[0]=0;
    if(p->selected>=0 && p->selected<p->count)strcpy(keep,p->entries[p->selected].name);
    p->count=p->limited=p->failed=0;
    d=opendir(p->path);
    if(!d){p->failed=1;p->selected=p->top=0;return -1;}
    for(;;) {
        errno=0;de=readdir(d);if(!de){e=errno;break;}
        if(!strcmp(de->d_name,"."))continue;
        if(!strcmp(p->path,"/") && !strcmp(de->d_name,".."))continue;
        if(strlen(de->d_name)>=NAME_CAP || p->count>=ENTRY_CAP){p->limited=1;continue;}
        if(joinpath(path,p->path,de->d_name)<0){p->limited=1;continue;}
        if(lstat(path,&st)<0){p->limited=1;continue;}
        strcpy(p->entries[p->count].name,de->d_name);
        p->entries[p->count].mode=st.st_mode;
        p->entries[p->count++].size=(unsigned long)st.st_size;
    }
    if(closedir(d)<0 && !e)e=errno;
    if(e){p->count=p->selected=p->top=0;p->failed=1;errno=e;return -1;}
    qsort(p->entries,p->count,sizeof(struct entry),compare);
    if(p->selected>=p->count)p->selected=p->count?p->count-1:0;
    {int i;for(i=0;i<p->count;i++)if(!strcmp(keep,p->entries[i].name)){p->selected=i;break;}}
    p->top=0;return 0;
}
int validname(const char *s) {
    return *s && strcmp(s,".") && strcmp(s,"..") && !strchr(s,'/') && strlen(s)<NAME_CAP;
}
int copyfile(const char *src,const char *dst,int moving) {
    struct stat before,opened,after;int in=-1,out=-1,e=0,created=0;char buf[1024];ssize_t n,k,off;
    if(lstat(src,&before)<0)return -1;
    if(!S_ISREG(before.st_mode)){errno=EINVAL;return -1;}
    in=open(src,O_RDONLY);if(in<0)return -1;
    if(fstat(in,&opened)<0){e=errno;goto done;}
    if(!S_ISREG(opened.st_mode)||before.st_dev!=opened.st_dev||before.st_ino!=opened.st_ino){e=EAGAIN;goto done;}
    out=open(dst,O_WRONLY|O_CREAT|O_EXCL,before.st_mode&0777);
    if(out<0){e=errno;goto done;}created=1;
    while(!stopping) {
        n=read(in,buf,sizeof(buf));
        if(n<0){if(errno==EINTR)continue;e=errno;break;}
        if(!n)break;
        for(off=0;off<n;) {
            k=write(out,buf+off,n-off);
            if(k<0 && errno==EINTR && !stopping)continue;
            if(k<=0){e=k<0?errno:EIO;break;}off+=k;
        }
        if(e)break;
    }
    if(stopping)e=EINTR;
    if(!e && fstat(in,&after)<0)e=errno;
    if(!e && (after.st_size!=before.st_size || after.st_mtime!=before.st_mtime))e=EAGAIN;
 done:
    if(in>=0 && close(in)<0 && !e)e=errno;
    if(out>=0 && close(out)<0 && !e)e=errno;
    if(e) {
        if(created && unlink(dst)<0) {
            snprintf(message,sizeof(message),"Copy failed; partial destination remains. Check target.");
        }
        errno=e;return -1;
    }
    if(moving) {
        /* Source is removed only after a complete, closed destination exists. */
        if(lstat(src,&after)<0)return -1;
        if(after.st_dev!=before.st_dev || after.st_ino!=before.st_ino || after.st_size!=before.st_size || after.st_mtime!=before.st_mtime){errno=EAGAIN;return -1;}
        if(unlink(src)<0)return -1; /* Both files remain on removal failure. */
    }
    return 0;
}
int removeitem(const char *path) {
    struct stat st;if(lstat(path,&st)<0)return -1;
    if(S_ISDIR(st.st_mode))return rmdir(path);
    if(S_ISREG(st.st_mode)||S_ISLNK(st.st_mode))return unlink(path);
    errno=EINVAL;return -1;
}
