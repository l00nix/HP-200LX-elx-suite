/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef CONSOLE_MOCK_H
#define CONSOLE_MOCK_H
#define DCGET_GRAPH 0x4401
#define DCREL_GRAPH 0x4402
unsigned mock_bios(unsigned ax);
int mock_open(const char *,int);
int mock_close(int);
int mock_ioctl(int,int,void *);
int mock_tcgetattr(int,struct termios *);
int mock_tcsetattr(int,int,const struct termios *);
ssize_t mock_read(int,void *,size_t);
ssize_t mock_write(int,const void *,size_t);
int mock_usleep(unsigned long);
#define bios_video mock_bios
#define open mock_open
#define close mock_close
#define ioctl mock_ioctl
#define tcgetattr mock_tcgetattr
#define tcsetattr mock_tcsetattr
#define read mock_read
#define write mock_write
#define usleep mock_usleep
#endif
