/* SPDX-License-Identifier: GPL-2.0-or-later
 * Read-only HP identity reader shared with ELXstat.
 */
#ifndef ELX_H
#define ELX_H
struct elx_reader { int fd, offline; unsigned model; char rom[33],date[9],version[25]; };
unsigned elx_word(const unsigned char *p);
int elx_open(struct elx_reader *r,const char *image);
void elx_close(struct elx_reader *r);
#endif
