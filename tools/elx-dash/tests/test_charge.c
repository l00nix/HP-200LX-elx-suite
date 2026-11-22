/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "charge.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct charge_limits p={2900,5,2};
static void begin(struct charge_cycle *c){assert(charge_begin(c,&p,100,2500,0x80)==0);}
int main(void) {
    struct charge_cycle c;struct charge_limits bad=p;unsigned f;unsigned long t;
    bad.minutes=0;assert(!charge_limits_valid(&bad));bad=p;bad.mv=3300;assert(!charge_limits_valid(&bad));
    bad=p;bad.stall=6;assert(!charge_limits_valid(&bad));bad=p;bad.minutes=61;assert(!charge_limits_valid(&bad));
    for(f=0;f<256;f++) {
        int ok=charge_begin(&c,&p,100,2500,f)==0;
        assert(ok==((f&0xf8)==0x80));
    }
    assert(charge_begin(&c,&p,100,2900,0x80)<0);
    assert(charge_begin(&c,&p,100,1999,0x80)<0);
    assert(charge_begin(&c,&p,100,2500,0x8080)<0);
    assert(charge_begin(&c,&p,100,2500,0x4080)<0);
    begin(&c);assert(charge_step(&c,101,2500,0x88,1)==0);
    assert(charge_step(&c,102,2900,0x88,1)==1);assert(strstr(c.reason,"Voltage limit"));
    assert(charge_step(&c,103,2400,0x88,1)==0&&!c.active); /* never restart */
    begin(&c);assert(charge_step(&c,101,2500,0xc8,1)==1); /* unplug */
    begin(&c);assert(charge_step(&c,101,2500,0x8088,1)==1);
    begin(&c);assert(charge_step(&c,101,2500,0x4088,1)==1);
    begin(&c);assert(charge_step(&c,101,2500,0x88,0)==1); /* bad ADC */
    begin(&c);assert(charge_step(&c,101,2500,0x80,1)==1); /* firmware veto */
    begin(&c);assert(charge_step(&c,101,2500,8,1)==1); /* chemistry lost */
    begin(&c);assert(charge_step(&c,99,2500,0x88,1)==1);
    begin(&c);assert(charge_step(&c,116,2500,0x88,1)==1);
    begin(&c);for(t=101;t<220;t++)assert(charge_step(&c,t,2500,0x88,1)==0);
    assert(charge_step(&c,220,2500,0x88,1)==1&&strstr(c.reason,"stall"));
    begin(&c);for(t=101;t<400;t++)assert(charge_step(&c,t,2500+(int)((t-100)/30),0x88,1)==0);
    assert(charge_step(&c,400,2510,0x88,1)==1&&strstr(c.reason,"Time"));
    puts("PASS: bounded cycles, exact limits, no restart, faults, stale/invalid samples, chemistry, clock gaps");return 0;
}
