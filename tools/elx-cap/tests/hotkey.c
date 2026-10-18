/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "../kernel/elxcap-state.h"
int main(void) {
    assert(!elxcap_scan(0x34,8,0,0,100));
    assert(!elxcap_lease(1,10,100));
    assert(elxcap_lease(2,20,100)==-1);
    assert(!elxcap_scan(0x34,0,0,20,100));
    assert(!elxcap_scan(0x34,9,0,20,100));
    assert(!elxcap_scan(0x34,12,0,20,100));
    assert(!elxcap_scan(0x34,8,1,20,100));
    assert(elxcap_scan(0x34,8,0,20,100)); assert(elxcap.count==1);
    assert(elxcap_scan(0x34,8,0,21,100)); assert(elxcap.count==1);
    assert(elxcap_scan(0xb4,0,0,22,100));
    assert(elxcap_scan(0x34,8,0,23,100)); assert(elxcap.count==2);
    assert(!elxcap_scan(0x34,8,0,311,100));
    assert(!elxcap_lease(2,311,100));
    elxcap.count=65535; assert(elxcap_scan(0x34,8,0,312,100));
    assert((elxcap.count&65535)==0);
    elxcap.owner=0; assert(!elxcap_lease(3,ULONG_MAX-10,100));
    assert(elxcap_live(20,100)); assert(!elxcap_live(400,100));
    puts("PASS hotkey lease, modifiers, extended keys, repeat, expiry, counter/tick wrap");
    return 0;
}
