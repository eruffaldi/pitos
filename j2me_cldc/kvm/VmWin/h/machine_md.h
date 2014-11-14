/*
 * @(#)machine_md.h     1.8 99/12/20
 *
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

/*=========================================================================
 * Spotless Virtual Machine
 *=========================================================================
 * SYSTEM:    KVM
 * FILE:      machine_md.h (for Windows)
 * OVERVIEW:  This file is included in every compilation.  It contains
 *            definitions that are specific to the Windows port of KVM.
 * AUTHOR:    Frank Yellin
 *            Ioi Lam
 *            Richard Berlin
 * NOTE:      This file overrides many of the default compilation
 *            flags and macros defined in VmCommon/h/main.h.
 *=======================================================================*/

/*=========================================================================
 * Platform definition
 *=======================================================================*/

#define WINDOWS 1

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

/*=========================================================================
 * Platform-specific datatype definitions
 *=======================================================================*/

typedef long             Long;
typedef char             Boolean;

typedef __int64          long64;  /*  64-bit signed integer type */
typedef unsigned __int64 ulong64; /*  64-bit unsigned integer type */

#ifndef Calendar_md
unsigned long *Calendar_md();
#endif

#define PATH_SEPARATOR    ';'

/*=========================================================================
 * Compilation flags and macros that override values defined in main.h
 *=======================================================================*/

/* This is a little-endian target platform */
#define LITTLE_ENDIAN 1

/* Make the VM run a little faster (can afford the extra space) */
#define ENABLEFASTBYTECODES 1

/* Enable all the new generic connections */
#define GENERICSTORAGE 1
#define GENERICNETWORK 1

/* Enable the use of asynchronous native functions on Windows */
#define ASYNCHRONOUS_NATIVE_FUNCTIONS 1

/*=========================================================================
 * Platform-specific macros and function prototypes
 *=======================================================================*/

#define InitializeVM()
#define FinalizeVM()

#define freeHeap(x) free(x)
#define RandomNumber_md() rand()

ulong64 sysTimeMillis(void);
void    *sysMapMemory(int memSize, int *pageSize);
int     sysProtectPage(void *ptr, int size, int write);
void    sysUnmapMemory(void *ptr, int memSize);


/* Graphics system operations (for supporting a Palm lookalike GUI) */

void InitializeWindowSystem();
void FinalizeWindowSystem(void);

void winDrawRectangle(int x, int y, int width, int height, int mode);
void winFillRectangle(int x, int y, int width, int height,
                      int cornerDiam, int mode);
void winDrawLine(int x1, int y1, int x2, int y2, int mode);

int  winCharWidth(int c);
void winDrawChar(int c, int x, int y, int mode);
void winRefreshScreen(int x, int y, int width, int height);
void winSetClip(int x, int y, int width, int height);
void winResetClip();
void winCopyRegion(int x, int y, int width, int height, int dstX, int dstY, int mode);
void winCopyRectangle(int srcWin, int dstWin, int x, int y,
                  int width, int height, int dstX, int dstY, int mode);
void winDrawMonoBitmap(int x, int y, int width, int height, int rowBytes, char *bitmap);

/*
 * Nik's additions to make storage.c build
 */

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <direct.h>

#ifndef __GNUC__
/*
 * The following is already defined in GCC from the Cygwin32 distribution
 */
#define open    _open
#define close   _close
#define read    _read
#define write   _write
#define access  _access
#define umask   _umask
#define fstat   _fstat
#define stat    _stat
#define mkdir   _mkdir

#define S_IFMT   _S_IFMT
#define S_IFREG  _S_IFREG
#define S_IFCHR  _S_IFCHR
#define S_IFFIFO _S_IFIFO
#define S_IFDIR  _S_IFDIR
#define S_IFBLK  _S_IFBLK

#define O_BINARY _O_BINARY

#endif /* __GNUC__ */

#ifndef S_ISREG
#define S_ISREG(mode)   ( ((mode) & S_IFMT) == S_IFREG )
#define S_ISCHR(mode)   ( ((mode) & S_IFMT) == S_IFCHR )
#define S_ISFIFO(mode)  ( ((mode) & S_IFMT) == S_ISFIFO )
#define S_ISDIR(mode)   ( ((mode) & S_IFMT) == S_IFDIR )
#define S_ISBLK(mode)   ( ((mode) & S_IFMT) == S_IFBLK )
#endif

/*
 * Flag to enable the new win32 async routine system,
 *
 * The old system created a new thread for each I/O operation.
 * The new system has a pool of 5 threads that are reused,
 */

#define NEWWIN32ASYNCSYSTEM 1

#if NEWWIN32ASYNCSYSTEM
void releaseAsyncThread(void);
void processAcyncThread(void);
#endif


