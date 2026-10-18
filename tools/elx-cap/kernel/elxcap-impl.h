/* SPDX-License-Identifier: GPL-2.0-or-later
 * Included by mem.c under CONFIG_HP200LX_FW. No allocation, screen access,
 * filesystem work or BIOS calls in the keyboard interrupt.
 */
#include "elxcap-state.h"
int elxcap_key(unsigned code,unsigned modifiers,unsigned extended) {
    return elxcap_scan(code,modifiers,extended,jiffies,HZ);
}
static int elxcap_ioctl(int cmd,char *arg) {
    flag_t flags;
    unsigned count;
    int result=0;
    save_flags(flags); clr_irq();
    if(cmd==0x4501) {
        if(elxcap_lease(current->pid,jiffies,HZ)<0) result=-EBUSY;
    } else {
        if(elxcap.owner==current->pid) { elxcap.owner=0; elxcap.down=0; }
        else result=-EPERM;
    }
    count=elxcap.count;
    restore_flags(flags);
    if(!result) put_user(count,arg);
    return result;
}
