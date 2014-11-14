/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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
 * SYSTEM:    KVM
 * SUBSYSTEM: Global definitions
 * FILE:      global.h
 * OVERVIEW:  Global system-wide definitions.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * COMMENT: 
 * This file contains declarations that do not belong to any 
 * particular structure or subsystem of the VM. There are many 
 * other additional global variable declarations in other files.
 *=======================================================================*/

/*=========================================================================
 * System include files
 *=======================================================================*/

/* The performs per-machine initialization */
#include <machine_md.h>

/*=========================================================================
 * Global compile-time constants and typedefs
 *=======================================================================*/

#undef  TRUE
#undef  FALSE

#define NIL   0

typedef	enum {
    FALSE = 0,
    TRUE = 1
} bool_t;


/*=========================================================================
 * Global data type declarations
 *=======================================================================*/

/*=========================================================================
 * NOTE: These type declarations are not quite as portable as they 
 * could be.  It might be useful to declare specific type names for
 * all Java-specific types (e.g., jint instead of normal int, jlong
 * instead of long64, and so forth).
 *=======================================================================*/

#define CELL  4        /* Size of a java word (= 4 bytes) */
#define log2CELL 2     /* Shift amount equivalent to dividing by CELL */
#define SHORT 2

#ifndef BYTE_DEF
typedef unsigned char    BYTE;
#endif 
typedef unsigned long     cell;    /*  'int' must be 32 bits long! */

/*=========================================================================
 * System-wide structure declarations
 *=======================================================================*/


typedef struct classStruct         *CLASS;
typedef struct instanceClassStruct *INSTANCE_CLASS;
typedef struct arrayClassStruct    *ARRAY_CLASS;

typedef struct objectStruct*     OBJECT;
typedef struct instanceStruct*   INSTANCE;
typedef struct arrayStruct*      ARRAY;
typedef struct stringInstanceStruct *STRING_INSTANCE;

typedef struct byteArrayStruct*  BYTEARRAY;
typedef struct shortArrayStruct* SHORTARRAY;
typedef struct pointerListStruct* POINTERLIST;

typedef struct fieldStruct*      FIELD;
typedef struct fieldTableStruct* FIELDTABLE; 
typedef struct methodStruct*     METHOD;
typedef struct methodTableStruct*METHODTABLE;
typedef struct stackMapStruct*   STACKMAP;
typedef struct icacheStruct*     ICACHE;
typedef struct chunkStruct*      CHUNK;

typedef struct staticChunkStruct* STATICCHUNK;
typedef struct threadQueue*      THREAD;
typedef struct javaThreadStruct* JAVATHREAD;
typedef struct monitorStruct*    MONITOR;

typedef struct stackStruct*      STACK;

typedef struct frameStruct*            FRAME;
typedef struct exceptionHandlerStruct* HANDLER;
typedef struct exceptionHandlerTableStruct* HANDLERTABLE;
typedef struct filePointerStruct* FILEPOINTER;

typedef union constantPoolEntryStruct*   CONSTANTPOOL_ENTRY;
typedef struct constantPoolStruct* CONSTANTPOOL;


#define ByteSizeToCellSize(n)   (((n) + (CELL - 1)) >> log2CELL)
#define StructSizeInCells(structName) ((sizeof(struct structName) + 3) >> 2)
#define UnionSizeInCells(structName) ((sizeof(union structName) + 3) >> 2)

/* Field and Method key types. */

typedef unsigned short NameKey;
typedef unsigned short MethodTypeKey;
typedef unsigned short FieldTypeKey;

typedef union { 
    struct { 
	unsigned short nameKey;
	unsigned short typeKey;	/* either MethodTypeKey or FieldTypeKey */
    } nt;
    unsigned long i;
} NameTypeKey;


/*=========================================================================
 * Locally defined include files
 *=======================================================================*/

#include "main.h"
#include "long.h"
#include "garbage.h"
#include "interpret.h"
#include "hashtable.h"
#include "thread.h"
#include "pool.h"
#include "class.h"
#include "fields.h"
#include "frame.h"
#include "loader.h"
#include "native.h"
#include "cache.h"
#include "nativeSpotlet.h"
#include "runtime.h"
#include "profiling.h"
#include "verifier.h"
#include "log.h"

#if GENERICNETWORK
#include "networkPrim.h"
#endif

/*=========================================================================
 * Miscellaneous global variables
 *=======================================================================*/

/*  Flags for toggling debugging and verbose modes on and off on PalmPilot */
/*  (controlled from the user interface of the VM) */
extern int Debugging_On;
extern int Verbose_On;
extern int Small_Heap;
extern bool_t JamEnabled;
extern bool_t JamRepeat;

/*=========================================================================
 * Most frequently called functions are "inlined" here
 *=======================================================================*/

/*=========================================================================
 * Quick operations for stack manipulation (for manipulating stack
 * frames and operands). 
 *=======================================================================*/

#define topStack               (*sp)
#define secondStack            (*(sp-1))
#define thirdStack             (*(sp-2))
#define fourthStack            (*(sp-3))

#define topStackAsType(_type_)     (*(_type_ *)sp)
#define secondStackAsType(_type_)  (*(_type_ *)(sp - 1))
#define thirdStackAsType(_type_)   (*(_type_ *)(sp - 2))
#define fourthStackAsType(_type_)  (*(_type_ *)(sp - 3))

#define oneMore                 sp++
#define oneLess                 sp--
#define moreStack(n)            sp += (n)
#define lessStack(n)            sp -= (n)

#define popStack()              (*sp--)
#define popStackAsType(_type_)      *(_type_ *)(sp--)

#define pushStack(data)         *++sp = (data)
#define pushStackAsType(_type_, data) \
                                *(_type_ *)(++sp) = (data)

/*=========================================================================
 * The equivalent macros to the above for use when the stack being
 * manipulated is not for the currently executing thread. In all cases
 * the additional parameter is the THREAD pointer.
 *=======================================================================*/

#define topStackForThread(t)         (*((t)->spStore))
#define secondStackForThread(t)      (*((t)->spStore - 1))
#define thirdStackForThread(t)       (*((t)->spStore - 2))
#define fourthStackForThread(t)      (*((t)->spStore - 3))


#define popStackForThread(t)         (*((t)->spStore--))
#define pushStackForThread(t, data)  (*(++(t)->spStore) = (data))
#define popStackAsTypeForThread(t, _type_) \
                                     (*(_type_*)((t)->spStore--))
#define pushStackAsTypeForThread(t, _type_, data)    \
                                    (*(_type_ *)(++(t)->spStore) = (data))
#define moreStackForThread(t, n)     ((t)->spStore += (n))
#define lessStackForThread(t, n)     ((t)->spStore -= (n))
#define oneMoreForThread(t)          ((t)->spStore++)
#define oneLessForThread(t)          ((t)->spStore--)

#define spForThread(t)               ((t)->spStore)


/*=========================================================================
 * Operations for handling system-dependent memory fetches
 *=======================================================================*/

/*=========================================================================
 * These macros define Java-specific memory read/write operations
 * for reading high-endian numbers.
 *=======================================================================*/

#define getCell(addr) ((((long)(((unsigned char *)addr)[0])) << 24) |  \
	                   (((long)(((unsigned char *)addr)[1])) << 16) |  \
	                   (((long)(((unsigned char *)addr)[2])) << 8)  |  \
	                   (((long)(((unsigned char *)addr)[3])) << 0)) 


#define getUShort(addr) ((((unsigned short)(((unsigned char *)addr)[0])) << 8) |  \
						 (((unsigned short)(((unsigned char *)addr)[1])) << 0))

#define getShort(addr) ((short)(getUShort(addr)))

 
#define putShort(addr, value) \
			 ((unsigned char *)addr)[0] = (unsigned char)(value) >> 8; \
  	         ((unsigned char *)addr)[1] = (unsigned char)(value & 0xFF)

