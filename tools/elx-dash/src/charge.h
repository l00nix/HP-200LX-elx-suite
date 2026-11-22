/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef DASH_CHARGE_H
#define DASH_CHARGE_H
/* A manually started, bounded test cycle. No restart or trickle state. */
struct charge_limits { unsigned mv, minutes, stall; };
struct charge_cycle {
    struct charge_limits limits;
    unsigned long start,last,rise;
    int peak,active;
    const char *reason;
};
int charge_limits_valid(const struct charge_limits *p);
int charge_begin(struct charge_cycle *c,const struct charge_limits *p,
                 unsigned long now,int mv,unsigned flags);
int charge_step(struct charge_cycle *c,unsigned long now,int mv,unsigned flags,int valid);
/* Backend actions: 0 stop, 1 start; DOS uses ABC's own configuration UI. */
int dash_charge_start(const struct charge_limits *p);
int dash_charge_stop(void);
int dash_charge_status(char *text,unsigned size);
int dash_abc_open(void);
int dash_charge_available(void);
extern int dash_charge_test;
#endif
