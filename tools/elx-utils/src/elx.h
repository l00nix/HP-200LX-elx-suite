/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_H
#define ELX_H
#define ELX_VERSION "0.1"
#define ELX_BAT_VERSION "0.1.3"
#define ELX_POWER_MASK 0x78
struct elx_status {
    unsigned char bda[256];
    unsigned long charge_ticks;
    unsigned model;
    char rom[33], date[9], version[25];
    int offline;
};
struct elx_reader { int fd, offline; unsigned model; char rom[33], date[9], version[25]; };
unsigned elx_word(const unsigned char *p);
int elx_mv(unsigned raw); /* -1 for invalid sample; integer mV, truncated */
unsigned elx_bar(int mv, unsigned width);
unsigned long elx_seconds(unsigned long ticks);
int elx_open(struct elx_reader *r, const char *image);
int elx_read(struct elx_reader *r, struct elx_status *s);
int elx_read_power(struct elx_reader *r, unsigned char *flags);
void elx_close(struct elx_reader *r);
const char *elx_type(const struct elx_status *s);
const char *elx_source(const struct elx_status *s);
const char *elx_charging(const struct elx_status *s);
void elx_voltage(char *buf, unsigned raw);
void elx_battery(const struct elx_status *s);
void elx_text_bars(const struct elx_status *s);
struct elx_canvas {
    void (*rect)(int x,int y,int w,int h,int black);
    void (*line)(int x1,int y1,int x2,int y2);
    void (*poly)(const int *xy,unsigned count);
    void (*text)(int x,int y,const char *s,int white);
};
int elx_low_mv(const struct elx_status *s,int backup);
const char *elx_alert(const struct elx_status *s,int backup);
void elx_meter(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned fill);
void elx_tick(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned a);
void elx_marker(const struct elx_canvas *c,int cx,int cy,int rx,int ry,unsigned a,int solid);
void elx_digits(const struct elx_canvas *c,int cx,int y,int mv);
void elx_draw(const struct elx_canvas *c,const struct elx_status *s,const char *stamp);
void elx_power_icon(const struct elx_canvas *c,int x,int y,unsigned flags,int stale);
void elx_draw_power(const struct elx_canvas *c,const struct elx_status *s,int stale);
#endif
