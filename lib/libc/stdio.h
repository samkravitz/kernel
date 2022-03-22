/* maestro
 * License: GPLv2
 * See LICENSE.txt for full license text
 * Author: Sam Kravitz
 *
 * FILE: libc/stdio.h
 * DATE: March 22nd, 2022
 * DESCRIPTION: standard input/output utilities 
 */
#ifndef STDIO_H
#define STDIO_H

#include <maestro.h>
#include <stdarg.h>

typedef struct
{
	char name[32];
	size_t len;
	u32 eof;
	u32 position;
	int fd;
} FILE;

int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int sprintf(char *, const char *, ...);

int vprintf(const char *, va_list);
int vfprintf(FILE *, const char *, va_list);
int vsprintf(char *, const char *, va_list);

#endif    // STDIO_H
