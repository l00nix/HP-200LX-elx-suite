/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELXCMD_H
#define ELXCMD_H
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define VERSION "0.1"
#define PATH_CAP 256
#define NAME_CAP 64
#define ENTRY_CAP 128
#define PAGE_ROWS 17
struct entry { char name[NAME_CAP]; unsigned long size; mode_t mode; };
struct panel { char path[PATH_CAP]; struct entry entries[ENTRY_CAP]; int count,selected,top,limited,failed; };
extern volatile int stopping;
extern char message[160];
int joinpath(char *out,const char *dir,const char *name);
int canonical(char *out,const char *path);
int loadpanel(struct panel *p);
int copyfile(const char *src,const char *dst,int moving);
int removeitem(const char *path);
int validname(const char *name);
void error_message(const char *action);
enum { K_UP=256,K_DOWN,K_LEFT,K_RIGHT,K_HOME,K_END,K_PGUP,K_PGDN,K_F1=280 };
int terminal_open(void);
void terminal_close(void);
int key(void);
int decodekey(const char *s);
void at(int row,int col);
void field(int row,int col,int width,const char *s,int reverse);
int prompt(const char *label,char *buf,int cap);
#endif
