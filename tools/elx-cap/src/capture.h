/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef ELX_CAPTURE_H
#define ELX_CAPTURE_H
#define CAP_BYTES 16000
#define CAP_LEASE 0x4501
#define CAP_RELEASE 0x4502
extern unsigned char cap_pixels[CAP_BYTES];
extern const char *cap_error;
int cap_open(const char *image);
int cap_read(int fd);
int cap_save(const char *path, int pbm, int invert);
int cap_numbered(const char *dir, int pbm, int invert, char *path);
#endif
