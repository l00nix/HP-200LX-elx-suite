/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "elx.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    unsigned i,last=0;
    assert(elx_mv(0)==1570);assert(elx_mv(63)==3300);
    assert(elx_mv(36)==2558);assert(elx_mv(47)==2860);
    for(i=0;i<256;i++) {
        int mv=elx_mv(i);
        assert(i<=63 ? mv>=1570 && mv<=3300 : mv==-1);
        if(i && i<=63) assert(mv>elx_mv(i-1));
    }
    for(i=0;i<5000;i++) {
        unsigned n=elx_bar((int)i,600);
        assert(n>=last && n<=600);last=n;
    }
    assert(elx_bar(-1,600)==0);assert(elx_bar(3500,600)==600);
    assert(elx_seconds(0)==0);assert(elx_seconds(200)==11);
    assert(elx_seconds(4294967295UL)==236223201UL);
    puts("PASS: voltage conversion, invalid codes, graph bounds, 32-bit charging timer");
    return 0;
}
