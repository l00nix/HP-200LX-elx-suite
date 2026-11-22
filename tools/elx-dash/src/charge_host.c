/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "dash.h"
#include <string.h>
int dash_charge_available(void){ctl_error="Demo only";return 0;}
int dash_charge_start(const struct charge_limits *p){(void)p;ctl_error="Demo only";return -1;}
int dash_charge_stop(void){ctl_error="Demo only";return -1;}
int dash_charge_status(char *s,unsigned n){if(n){strncpy(s,"DEMO: no hardware or resident supervisor",n);s[n-1]=0;}return 0;}
int dash_abc_open(void){ctl_error="ABC/LX requires DOS";return -1;}
