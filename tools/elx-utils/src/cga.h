/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_CGA_H
#define ELX_CGA_H
#include "elx.h"
extern const struct elx_canvas elx_cga_canvas;
int elx_text_width(const char *text);
int elx_cga_save(const char *path);
int elx_console_open(void);
int elx_console_close(void);
int elx_console_key(void);
void elx_console_wait(void);
#endif
