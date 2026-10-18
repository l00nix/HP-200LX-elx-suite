/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "cga.h"
static const int arc[61][2] = {
    {-1000,0},
    {-999,-52},
    {-995,-105},
    {-988,-156},
    {-978,-208},
    {-966,-259},
    {-951,-309},
    {-934,-358},
    {-914,-407},
    {-891,-454},
    {-866,-500},
    {-839,-545},
    {-809,-588},
    {-777,-629},
    {-743,-669},
    {-707,-707},
    {-669,-743},
    {-629,-777},
    {-588,-809},
    {-545,-839},
    {-500,-866},
    {-454,-891},
    {-407,-914},
    {-358,-934},
    {-309,-951},
    {-259,-966},
    {-208,-978},
    {-156,-988},
    {-105,-995},
    {-52,-999},
    {0,-1000},
    {52,-999},
    {105,-995},
    {156,-988},
    {208,-978},
    {259,-966},
    {309,-951},
    {358,-934},
    {407,-914},
    {454,-891},
    {500,-866},
    {545,-839},
    {588,-809},
    {629,-777},
    {669,-743},
    {707,-707},
    {743,-669},
    {777,-629},
    {809,-588},
    {839,-545},
    {866,-500},
    {891,-454},
    {914,-407},
    {934,-358},
    {951,-309},
    {966,-259},
    {978,-208},
    {988,-156},
    {995,-105},
    {999,-52},
    {1000,0},
};
static int px(int cx,unsigned a,int r) { return cx+(int)((long)arc[a][0]*r/1000); }
static int py(int cy,unsigned a,int r) { return cy+(int)((long)arc[a][1]*r/1000); }
/* Large, allocation-free seven-segment voltage readout. */
static void digit(const struct elx_canvas *c,int x,int y,unsigned n) {
    static const unsigned char masks[10]={0x3f,6,0x5b,0x4f,0x66,0x6d,0x7d,7,0x7f,0x6f};
    unsigned m=masks[n];
    if(m&1)c->rect(x+2,y,9,2,1);
    if(m&2)c->rect(x+11,y+2,2,8,1);
    if(m&4)c->rect(x+11,y+12,2,8,1);
    if(m&8)c->rect(x+2,y+20,9,2,1);
    if(m&16)c->rect(x,y+12,2,8,1);
    if(m&32)c->rect(x,y+2,2,8,1);
    if(m&64)c->rect(x+2,y+10,9,2,1);
}
void elx_digits(const struct elx_canvas *c,int cx,int y,int mv) {
    int x=cx-39;
    if(mv<0) {c->text(cx-33,y+10,"Unavailable",0);return;}
    digit(c,x,y,(unsigned)(mv/1000));c->rect(x+15,y+20,3,2,1);
    digit(c,x+21,y,(unsigned)(mv/100%10));
    digit(c,x+37,y,(unsigned)(mv/10%10));
    digit(c,x+53,y,(unsigned)(mv%10));
    c->text(x+71,y+9,"V",0);
}

void elx_meter(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned fill) {
    unsigned a;int p[8],ix=rx-14,iy=ry-7;
    if(fill>60)fill=60;
    for(a=0;a<60;a++) {
        if(a<fill) {
            p[0]=px(cx,a,rx);p[1]=py(cy,a,ry);
            p[2]=px(cx,a+1,rx);p[3]=py(cy,a+1,ry);
            p[4]=px(cx,a+1,ix);p[5]=py(cy,a+1,iy);
            p[6]=px(cx,a,ix);p[7]=py(cy,a,iy);c->poly(p,4);
        }
        c->line(px(cx,a,rx),py(cy,a,ry),px(cx,a+1,rx),py(cy,a+1,ry));
        c->line(px(cx,a,ix),py(cy,a,iy),px(cx,a+1,ix),py(cy,a+1,iy));
    }
    c->line(cx-rx,cy,cx-ix,cy);c->line(cx+ix,cy,cx+rx,cy);
}
void elx_marker(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned a,int solid) {
    int p[6];unsigned lo=a<2?0:a-2,hi=a>58?60:a+2;
    if(a>60)return;
    p[0]=px(cx,a,rx+2);p[1]=py(cy,a,ry+2);
    p[2]=px(cx,lo,rx+16);p[3]=py(cy,lo,ry+9);
    p[4]=px(cx,hi,rx+16);p[5]=py(cy,hi,ry+9);
    /* Endpoint triangles must remain non-degenerate. */
    if(a==0){p[3]=cy-4;p[5]=cy+4;}
    if(a==60){p[3]=cy-4;p[5]=cy+4;}
    if(solid)c->poly(p,3);
    else {c->line(p[0],p[1],p[2],p[3]);c->line(p[2],p[3],p[4],p[5]);c->line(p[4],p[5],p[0],p[1]);}
}

void elx_tick(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned a) {
    if(a<=60)c->line(px(cx,a,rx+2),py(cy,a,ry+2),px(cx,a,rx+9),py(cy,a,ry+5));
}
