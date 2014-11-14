
/*=========================================================================
 * Spotless Virtual Machine
 *=========================================================================
 * SYSTEM:    KVM
 * FILE:      machine_md.h (for PITOS)
 * AUTHOR:    Ruffaldi Emanuele
 * NOTE:      This file overrides many of the default compilation 
 *            flags and macros defined in VmCommon/h/main.h.
 *=======================================================================*/

/*=========================================================================
 * Platform definition
 *=======================================================================*/

#define PITOS 1

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

/*=========================================================================
 * Platform-specific datatype definitions
 *=======================================================================*/

#define USES_CLASSPATH 0
#define COMPILER_SUPPORTS_LONG 1
typedef long long           long64;    /*  64-bit signed integer type */
typedef unsigned long long  ulong64;   /*  64-bit unsigned integer type */

#define ROMIZING 1
/*
#ifndef Calendar_md
unsigned long * Calendar_md(void);
#endif
*/

/*=========================================================================
 * Compilation flags and macros that override values defined in main.h
 *=======================================================================*/

/* This is a big-endian target platform */
#define LITTLE_ENDIAN 1

/* Make the VM run a little faster (can afford the extra space) */
// #define ENABLEFASTBYTECODES 1

/* Solaris port utilizes the Generic Connection framework */
//#define GENERICNETWORK 1
//#define GENERICSTORAGE 1

/* Override the sleep function defined in main.h */
/*#define SLEEP_UNTIL(wakeupTime)                                                         \
    {  long delta = wakeupTime - CurrentTime_md();                                      \
           struct timeval timeout;                                                      \
           timeout.tv_sec = delta / 1000;                                               \
           timeout.tv_usec = (delta % 1000) * 1000;                                     \
           select(0, NULL, NULL, NULL, &timeout);                                       \
        }
*/

/*=========================================================================
 * Platform-specific macros and function prototypes
 *=======================================================================*/

#define InitializeVM()
#define FinalizeVM()

#define freeHeap(heap) free(heap)
#define RandomNumber_md() rand()

#define ERROR_TRY \
	if(1) 
#define ERROR_CATCH(x)\
	else if (0) 
#define ERROR_END_CATCH
#define ERROR_THROW(i)
//void InitializeWindowSystem();
//void FinalizeWindowSystem(void);

