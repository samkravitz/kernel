#ifndef FCNTL_H
#define FCNTL_H

#define O_RDONLY   0x1
#define O_WRONLY   0x2
#define O_RDWR     0x4
#define O_APPEND   0x8
#define O_CREAT    0x10
#define O_DSYNC    0x20
#define O_EXCL     0x40
#define O_NOTTY    0x80
#define O_NONBLOCK 0x100

int open(const char *, int, ...);

#endif    // FCNTL_H
