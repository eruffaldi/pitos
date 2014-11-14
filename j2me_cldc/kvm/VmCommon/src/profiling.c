/*
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
 * KVM 
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Profiling
 * FILE:      profiling.c
 * OVERVIEW:  All profiling declarations, and initializations are here.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

#include <global.h>

#if ENABLEPROFILING


int InstructionCounter;      /*  Number of bytecodes executed */
int ThreadSwitchCounter;     /*  Number of thread switches */

int DynamicObjectCounter;       /*  Number of dynamic objects allocated */
int DynamicAllocationCounter;   /*  Bytes of dynamic memory allocated */
int DynamicDeallocationCounter; /*  Bytes of dynamic memory deallocated */
int GarbageCollectionCounter;   /*  Number of garbage collections done */
int TotalGCDeferrals;			/* Total number of GC objects deferred */
int MaximumGCDeferrals;			/*  Maximum number of GC objects deferred */
int GarbageCollectionRescans;   /* Number of extra scans of GC heap */

#if ENABLEFASTBYTECODES
int InlineCacheHitCounter;     /*  Number of inline cache hits */
int InlineCacheMissCounter;    /*  Number of inline cache misses */
int MaxStackCounter;           /*  Maximum amount of stack space needed */
#endif

#if SAFEGARBAGECOLLECTION
int HeapScanPointerValidations; /*  Number of calls to isValidHeapPointer */
#endif

#if USESTATIC
int StaticObjectCounter;        /* Number of static objects allocated */
int StaticAllocationCounter;    /* Bytes of static memory allocated */
#endif


/*=========================================================================
 * FUNCTION:      InitializeProfiling
 * TYPE:          Profiling
 * OVERVIEW:      Reset all profiling counters to zero
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeProfiling()
{ 
    /*  Zero any profiling counters. */
    InstructionCounter         = 0;
    ThreadSwitchCounter        = 0;
    DynamicObjectCounter       = 0;
    DynamicAllocationCounter   = 0;
    DynamicDeallocationCounter = 0;
    GarbageCollectionCounter   = 0;

    TotalGCDeferrals           = 0;
    MaximumGCDeferrals         = 0;
    GarbageCollectionRescans   = 0;

#if ENABLEFASTBYTECODES
    InlineCacheHitCounter      = 0;
    InlineCacheMissCounter     = 0;
    MaxStackCounter            = 0;
#endif

#if SAFEGARBAGECOLLECTION
    HeapScanPointerValidations = 0;
#endif

#if USESTATIC
    StaticObjectCounter        = 0;
    StaticAllocationCounter    = 0;
#endif
}

/*=========================================================================
 * FUNCTION:      printProfileInfo()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print the summary of the contents of all the profiling 
 *                variables for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void printProfileInfo()
{
    long numClasses;
    long classesMemory;
    
    numClasses = countClasses(&classesMemory);
    
    /* Note, the Pilot wants ints printed with %ld, while every
     * other machine in the known universe prints them with %d.
     * To keep everyone happy, we use long's exclusively.
     */
    fprintf(stdout,"Execution completed successfully\n");
    fprintf(stdout,"%ld bytecodes executed\n", (long)InstructionCounter);
    fprintf(stdout,"%ld thread switches\n", (long)ThreadSwitchCounter);
    fprintf(stdout,"%ld classes loaded (%ld bytes)\n", 
            (long)numClasses, (long)classesMemory);
    fprintf(stdout,"%ld objects allocated (%ld bytes)\n", 
        (long)DynamicObjectCounter, (long)DynamicAllocationCounter);
    fprintf(stdout,"%ld garbage collections\n",
            (long)GarbageCollectionCounter);
    fprintf(stdout,"%ld bytes collected\n",
            (long)DynamicDeallocationCounter);

    fprintf(stdout, "%ld objects deferred in GC\n", (long)TotalGCDeferrals);
    fprintf(stdout, "%ld (maximum) objects deferred at any one time\n", 
            (long)MaximumGCDeferrals);
    fprintf(stdout, "%ld rescans of heap because of deferral overflow\n", 
            (long)GarbageCollectionRescans);


#if SAFEGARBAGECOLLECTION
    fprintf(stdout,"%ld pointer validations requiring heap scans\n",
            (long)HeapScanPointerValidations);
#endif    
    fprintf(stdout,"Current memory usage %ld bytes\nHeap size %ld bytes\n", 
            (long)memoryUsage(), (long)getHeapSize());
}


#endif
