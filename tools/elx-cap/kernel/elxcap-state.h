/* SPDX-License-Identifier: GPL-2.0-or-later
 * Pure state machine shared with tests. Call under saved interrupt flags.
 * A three-second renewable lease prevents a dead listener owning the key.
 */
struct elxcap_state {
    int owner;
    unsigned long renewed;
    unsigned count;
    unsigned char down;
};
static struct elxcap_state elxcap;
static int elxcap_live(unsigned long now, unsigned hz) {
    return elxcap.owner && now-elxcap.renewed < 3UL*hz;
}
static int elxcap_lease(int pid,unsigned long now,unsigned hz) {
    if(elxcap_live(now,hz) && elxcap.owner!=pid) return -1;
    if(!elxcap_live(now,hz)) elxcap.down=0;
    elxcap.owner=pid; elxcap.renewed=now;
    return 0;
}
static int elxcap_scan(unsigned code,unsigned modifiers,unsigned extended,
                       unsigned long now,unsigned hz) {
    if(extended || (code&0x7f)!=0x34) return 0;
    if(code&0x80) {
        int consumed=elxcap.down;
        elxcap.down=0; return consumed;
    }
    if(!elxcap_live(now,hz)) { elxcap.down=0; return 0; }
    if(elxcap.down) return 1; /* Suppress typematic repeats until release. */
    if((modifiers&0x0f)!=0x08) return 0; /* Alt only, no Ctrl/Shift. */
    elxcap.down=1; ++elxcap.count; return 1;
}
