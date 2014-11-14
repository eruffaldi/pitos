/* Project:     OSLib
 * Description: The OS Construction Kit
 * Date:                1.6.2000
 * Idea by:             Luca Abeni & Gerardo Lamastra
 *
 * OSLin is an SO project aimed at developing a common, easy-to-use
 * low-level infrastructure for developing OS kernels and Embedded
 * Applications; it partially derives from the HARTIK project but it
 * currently is independently developed.
 *
 * OSLib is distributed under GPL License, and some of its code has
 * been derived from the Linux kernel source; also some important
 * ideas come from studying the DJGPP go32 extender.
 *
 * We acknowledge the Linux Community, Free Software Foundation,
 * D.J. Delorie and all the other developers who believe in the
 * freedom of software and ideas.
 *
 * For legalese, check out the included GPL license.
 */

#ifndef __LL_I386_STDIO_H__
#define __LL_I386_STDIO_H__

#include <ll/i386/defs.h>
BEGIN_DEF

#include <stdarg.h>

typedef int FILE;
#define EOF -1
extern FILE * stderr,*stdout;

int putchar(int c);
int vsprintf(char *buf,char *fmt,va_list parms);
int sprintf(char *buf,char *fmt,...) __attribute__((__format__(printf,2,3)));
int vsscanf(char *buf,char *fmt,va_list parms);
int sscanf(char *buf,char *fmt,...) __attribute__((__format__(scanf,2,3)));
int fprintf(FILE * fp,char *fmt,...) __attribute__((__format__(printf,2,3)));

int ll_printf(char *fmt,...);

END_DEF
#endif
