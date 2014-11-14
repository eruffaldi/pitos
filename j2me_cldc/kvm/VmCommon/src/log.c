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
 * SUBSYSTEM: Profiling
 * FILE:      log.c
 * OVERVIEW:  Output for diagnostic information gathered at run-time
 * AUTHOR:    Daniel Blaukopf, based on code by Antero Taivalsaari
 *            and Doug Simon
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

static void log_nullFunction() { return; }

#if INCLUDEDEBUGCODE || TRACEMETHODCALLS
static void log_enterMethod(const char *type,
			    const char *className, 
			    const char *methodName,
			    const char *signature) {
    if (TERSE_MESSAGES) {
	fprintf(stdout, "=> %s.%s\n",
		className, methodName);
    } else {
	fprintf(stdout, "Invoking %s method '%s::%s%s'\n", 
		type, className, methodName, signature);
    }
}

static void log_exitMethod(const char *className, const char *methodName) {
    if (TERSE_MESSAGES) {
	fprintf(stdout,"<= %s.%s\n",
		className, methodName);
    } else { 
	fprintf(stdout, "Leaving %s::%s\n",
		className, methodName);
    }
}
#else
#define log_enterMethod \
 ((void (*)(const char*,const char*,const char *,const char *))log_nullFunction)
#define log_exitMethod ((void (*)(const char *,const char *))log_nullFunction)
#endif

#if INCLUDEDEBUGCODE || TRACEGARBAGECOLLECTION
static void log_allocateObject(long pointer,
			       long cells,
			       int type,
			       long ID,
			       long memoryFree) {

    fprintf(stdout,"Allocated %lx, %ld cells, type:%d, id:%lx, free: %ld\n",
	    pointer, cells, type, ID, memoryFree);
}

static void log_allocateHeap(long heapSize,
			     long heapBottom,
			     long heapTop) {

    fprintf(stdout, "Allocated a dynamic heap of %ld bytes\n", HeapSize);
    fprintf(stdout, "heap bottom: %lx, heap top: %lx\n",
	    heapBottom, heapTop);
}

#else
#define log_allocateObject ((void (*)(long,long,int,long,long))log_nullFunction)
#define log_allocateHeap   ((void (*)(long,long,long))log_nullFunction)
#endif

#if INCLUDEDEBUGCODE || TRACEGARBAGECOLLECTIONVERBOSE

static void log_heapObject(long object,
			   const char *type,
			   long size,
			   int isMarked,
			   const char *className,
			   int isClass) {
    fprintf(stdout, "%lx: %s, size=%ld%s",
	    object, type, size,
	    isMarked ? ", MARKED" : "");
    if (className==NULL) {
	fprintf(stdout, "\n");
    } else if (isClass) {
	fprintf(stdout, ", class %s\n", className);
    } else {
	fprintf(stdout, ", instance of %s\n", className);
    }
}

static void log_heapSummary(int objectCount, int freeCount,
			      int liveByteCount, int freeByteCount,
			      int largestFreeObjectSize,
			      long heapBottom, long heapTop) {
    fprintf(stdout,"%d objects in heap (%d alive/%d free)\n",
	    objectCount+freeCount, objectCount, freeCount);
    fprintf(stdout,"%d live bytes; %d bytes free (%d total)\n",
	    liveByteCount, freeByteCount, liveByteCount+freeByteCount);
    fprintf(stdout,"Largest free object: %d bytes\n",
	    largestFreeObjectSize);
    fprintf(stdout,"Heap bottom: %lx, heap top: %lx\n",
	    heapBottom, heapTop);
}

static void log_horizontalRule(void) {
    fprintf(stdout, "\n====================\n");
}

#define log_startHeapScan log_horizontalRule
#define log_endHeapScan log_horizontalRule

static void log_heapWarning (long freeListCount,
		     long byteCount) {
    fprintf(stdout, "WARNING: %ld objects in the free list (%ld bytes)\n",
	    freeListCount, byteCount);
}


#else
#define log_heapObject \
  ((void (*)(long,const char*,long,int,const char*,int))log_nullFunction)
#define log_heapSummary \
  ((void (*)(int,int,int,int,int,long,long))log_nullFunction)
#define log_startHeapScan ((void (*)(void))log_nullFunction)
#define log_endHeapScan   ((void (*)(void))log_nullFunction)
#define log_heapWarning   ((void (*)(long,long))log_nullFunction)
#endif

#if TRACECLASSLOADING || TRACECLASSLOADINGVERBOSE
static void log_loadClass(const char *className) {
    fprintf(stdout,"Loading class '%s'\n", className);
}
#else
#define log_loadClass ((void (*)(const char *))log_nullFunction)
#endif   
    
#if TRACEEXCEPTIONS
static void 
log_throwException(INSTANCE exception) { 
    START_TEMPORARY_ROOTS
	fprintf(stdout, "exception: %s\n", className(exception->ofClass));
    END_TEMPORARY_ROOT
}

#else
#define log_throwException ((void(*)(INSTANCE))log_nullFunction)
#endif

static void 
log_uncaughtException(INSTANCE exception) 
{
    START_TEMPORARY_ROOTS
	fprintf(stderr, "Uncaught exception %s\n", 
		className(exception->ofClass));
    END_TEMPORARY_ROOT
}


static struct KVMLogInterface_ logImplementation = {
    fprintf,
    log_enterMethod,
    log_exitMethod,
    log_allocateObject,
    log_allocateHeap,
    log_heapObject,
    log_heapSummary,
    log_startHeapScan,
    log_endHeapScan,
    log_heapWarning,
    log_loadClass,
    log_throwException,
    log_uncaughtException
};

KVMLogInterface Log = (KVMLogInterface) &logImplementation;

