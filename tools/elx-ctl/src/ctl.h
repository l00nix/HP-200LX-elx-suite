/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_CTL_H
#define ELX_CTL_H
enum { CT_STATUS, CT_CONTRAST, CT_VOLUME, CT_ZOOM, CT_ANN, CT_TIMEOUT };
struct ctl_state { unsigned contrast,volume,mode,zoom,cols,rows,ann,ticks; };
struct ctl_request { int command,set,dry,simulate; unsigned value; };
struct ctl_plan { unsigned vector,ax,bx,expected; };
extern const char *ctl_error;
int ctl_parse(int argc,char **argv,struct ctl_request *r);
int ctl_plan(const struct ctl_request *r,const struct ctl_state *s,struct ctl_plan *p);
int ctl_verified(int command,const struct ctl_plan *p,const struct ctl_state *s);
int ctl_open(int simulate);
int ctl_get(struct ctl_state *s);
void ctl_call(const struct ctl_plan *p);
void ctl_close(void);
#ifdef __ia16__
#define CT_ABI __attribute__((cdecl,noinline))
#else
#define CT_ABI
#endif
unsigned CT_ABI ctl_int15(unsigned ax,unsigned bx);
unsigned CT_ABI ctl_int10(unsigned ax);
unsigned CT_ABI ctl_volume_register(void);
#endif
