/* SPDX-License-Identifier: GPL-2.0-or-later
 * Standalone CGA display primitives. Clipped horizontal-line algorithm and
 * interleaved framebuffer addressing adapted from ELKSWeb/platform/elks_platform.c.
 * HP 200LX default LCD polarity: set bits are dark, clear bits are light.
 * Drawing callers use logical black/white; map them here without changing
 * the system-wide ON+/ inversion setting. No hardware I/O register accesses.
 */
#include "cga.h"
#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__WATCOMC__)
#include <dos.h>
static unsigned char __far *video=(unsigned char __far *)0xb8000000UL;
#elif defined(__ia16__) && !defined(ELX_CGA_TEST)
static unsigned char __far *video=(unsigned char __far *)0xb8000000UL;
#else
static unsigned char video[16384];
#endif
static unsigned scan(int x,int y) {return (y&1)*8192U+(unsigned)(y>>1)*80U+((unsigned)x>>3);}
static void pixel(int x,int y,int white) {
    unsigned off;unsigned char mask;
    if(x<0 || x>=640 || y<0 || y>=200)return;
    off=scan(x,y);mask=0x80U>>(x&7);
    if(white)video[off]&=~mask;else video[off]|=mask;
}
static void hline(int x,int y,int width,int white) {
    unsigned off;unsigned char mask,value=white?0:255;
    if(width<=0 || y<0 || y>=200 || x>=640)return;
    if(x<0) {if(x<=-width)return;width+=x;x=0;}
    if(width>640-x)width=640-x;
    off=scan(x,y);
    if(x&7) {
        int n=8-(x&7);if(n>width)n=width;
        mask=(unsigned char)((255U>>(x&7))&(255U<<(8-(x&7)-n)));
        if(white)video[off]&=~mask;else video[off]|=mask;
        off++;width-=n;
    }
    while(width>=8) {video[off++]=value;width-=8;}
    if(width) {
        mask=(unsigned char)(255U<<(8-width));
        if(white)video[off]&=~mask;else video[off]|=mask;
    }
}
static void rect(int x,int y,int w,int h,int black) {
    if(h<=0 || y>=200)return;
    if(y<0) {if(y<=-h)return;h+=y;y=0;}
    if(h>200-y)h=200-y;
    while(h--)hline(x,y++,w,!black);
}
static void line(int x,int y,int x2,int y2) {
    int dx,dy,sx,sy,err,e;
    /* Callers draw inside the viewport; reject wildly offscreen endpoints
     * before 16-bit Bresenham arithmetic. All public geometry is bounded. */
    if(x<0 || x>=640 || x2<0 || x2>=640 || y<0 || y>=200 || y2<0 || y2>=200)return;
    dx=abs(x2-x);dy=-abs(y2-y);sx=x<x2?1:-1;sy=y<y2?1:-1;err=dx+dy;
    for(;;) {
        pixel(x,y,0);if(x==x2 && y==y2)break;
        e=2*err;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}
    }
}
static void poly(const int *xy,unsigned n) {
    int ymin=200,ymax=0,y;unsigned i,j;
    if(n<3 || n>8)return;
    for(i=0;i<n;i++) {
        if(xy[2*i]<0 || xy[2*i]>=640 || xy[2*i+1]<0 || xy[2*i+1]>=200)return;
        if(xy[2*i+1]<ymin)ymin=xy[2*i+1];
        if(xy[2*i+1]>ymax)ymax=xy[2*i+1];
    }
    for(y=ymin;y<=ymax;y++) {
        int left=640,right=-1;
        for(i=0;i<n;i++) {
            int x,ya,yb;j=(i+1)%n;ya=xy[2*i+1];yb=xy[2*j+1];
            if(ya==yb || y<(ya<yb?ya:yb) || y>(ya>yb?ya:yb))continue;
            x=xy[2*i]+(int)((long)(y-ya)*(xy[2*j]-xy[2*i])/(yb-ya));
            if(x<left)left=x;
            if(x>right)right=x;
        }
        if(left<=right)hline(left,y,right-left+1,0);
    }
}
static const struct elx_glyph *glyph(unsigned char ch) {
    return regular+(ch>=32 && ch<=126?ch-32:'?'-32);
}
int elx_text_width(const char *s) {int w=0;while(*s)w+=2*glyph((unsigned char)*s++)->advance;return w;}
static void text(int x,int y,const char *s,int white) {
    while(*s) {
        const struct elx_glyph *g=glyph((unsigned char)*s++);int r,col;
        for(r=0;r<10;r++)for(col=0;col<8;col++)if(g->rows[r]&(0x80U>>col))
            hline(x+2*(g->bearing+col),y+r,2,white);
        x+=2*g->advance;
    }
}
const struct elx_canvas elx_cga_canvas={rect,line,poly,text};
int elx_cga_save(const char *path) {
#if (!defined(__ia16__) && !defined(__WATCOMC__)) || defined(ELX_CGA_TEST)
    FILE *f=fopen(path,"wb");unsigned y,x;unsigned char row[80];
    if(!f)return -1;
    fputs("P4\n640 200\n",f);
    for(y=0;y<200;y++) {
        /* PBM also uses set bits for dark pixels, matching the HP LCD. */
        for(x=0;x<80;x++)row[x]=video[scan(0,y)+x];
        if(fwrite((const char *)row,1,80,f)!=80){fclose(f);return -1;}
    }
    return fclose(f);
#else
    (void)path;return -1;
#endif
}
