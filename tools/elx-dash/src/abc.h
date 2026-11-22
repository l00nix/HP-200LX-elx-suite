/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef DASH_ABC_H
#define DASH_ABC_H
#define ABC_FIELDS 5
/* millivolts for the first two fields; seconds for the other three (verified ABCUTIL/ABCTSR pair). */
struct abc_settings { unsigned value[ABC_FIELDS]; int valid; };
int abc_read(struct abc_settings *s);
int abc_set(unsigned field,unsigned value,unsigned *actual);
int abc_command(const char *command);
int abc_parse(unsigned field,const char *text,unsigned *value);
int abc_limits_ok(const struct abc_settings *s);
#endif
