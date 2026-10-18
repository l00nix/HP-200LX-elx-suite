/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_DASH_H
#define ELX_DASH_H
#include "ctl.h"
#include "elx.h"
#define DASH_VERSION "0.1"
#define DASH_NAME "ELXdash"
#define DK_LEFT 256
#define DK_RIGHT 257
#define DK_UP 258
#define DK_DOWN 259
struct dashboard {
    struct ctl_state state,entry;
    struct elx_status battery;
    unsigned selected,zoom;
    int zoom_pending,failed,demo;
    char message[80];
};
int dash_open(int demo);
int dash_read(struct ctl_state *s,struct elx_status *b);
void dash_call(const struct ctl_plan *p);
void dash_close(void);
int dash_enter(void);
int dash_leave(void);
int dash_key(void);
void dash_wait(void);
unsigned long dash_clock(void);
int dash_init(struct dashboard *d,int demo);
int dash_refresh(struct dashboard *d);
int dash_adjust(struct dashboard *d,int direction);
int dash_finish(struct dashboard *d,int apply_pending);
int dash_key_event(struct dashboard *d,int key);
void dash_draw(const struct dashboard *d);
#endif
