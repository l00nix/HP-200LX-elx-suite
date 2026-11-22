/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_DASH_H
#define ELX_DASH_H
#include "ctl.h"
#include "elx.h"
#include "charge.h"
#include "abc.h"
#if defined(__WATCOMC__) && !defined(DASH_DOS_MENU)
#define DASH_DOS_MENU
#endif
#define DASH_VERSION "0.2.0-test4"
#ifdef __WATCOMC__
#define DASH_NAME "LXdash"
#else
#define DASH_NAME "ELXdash"
#endif
#define DK_LEFT 256
#define DK_RIGHT 257
#define DK_UP 258
#define DK_DOWN 259
struct dashboard {
    struct ctl_state state,entry;
    struct elx_status battery;
    unsigned selected,zoom;
    int zoom_pending,failed,demo,power_failed;
    int menu,menu_item,confirm;
    struct charge_limits limits;
    struct abc_settings abc;
    unsigned abc_edit[ABC_FIELDS];
    int abc_loaded,abc_action;
    char charge_status[80];
    char message[80];
};
int dash_open(int demo);
int dash_read(struct ctl_state *s,struct elx_status *b);
int dash_read_power(unsigned char *flags);
void dash_call(const struct ctl_plan *p);
void dash_close(void);
int dash_enter(void);
int dash_leave(void);
int dash_key(void);
void dash_wait(void);
unsigned long dash_clock(void);
int dash_init(struct dashboard *d,int demo);
int dash_refresh(struct dashboard *d);
int dash_poll_power(struct dashboard *d);
int dash_adjust(struct dashboard *d,int direction);
int dash_finish(struct dashboard *d,int apply_pending);
int dash_key_event(struct dashboard *d,int key);
void dash_draw(const struct dashboard *d);
int dash_draw_update(const struct dashboard *d);
void dash_draw_menu(const struct dashboard *d);
int dash_dos_menu_key(struct dashboard *d,int key);
void dash_dos_draw_menu(const struct dashboard *d);
void dash_dos_menu_open(struct dashboard *d);
int dash_menu_key(struct dashboard *d,int k);
void dash_draw_power(const struct dashboard *d);
#endif
