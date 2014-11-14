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
 * Spotless Virtual Machine
 *=========================================================================
 * SYSTEM:    Spotless VM
 * SUBSYSTEM: Machine-specific implementations needed by virtual machine
 * FILE:      runtime_md.c
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>


/*=========================================================================
 * FUNCTION:      alertUser()
 * TYPE:          error handling operation
 * OVERVIEW:      Show an alert dialog to the user and wait for
 *                confirmation before continuing execution.
 * INTERFACE:
 *   parameters:  message string
 *   returns:     <nothing>
 *=======================================================================*/

void AlertUser(const char* message)
{
    fprintf(stderr, "ALERT: %s\n", message);
}


/*=========================================================================
 * FUNCTION:      allocateHeap()
 * TYPE:          allocates memory
 * OVERVIEW:      Show an alert dialog to the user and wait for
 *                confirmation before continuing execution.
 * INTERFACE:
 *   parameters:  *sizeptr:  INPUT:   Pointer to size of heap to allocate
 *                           OUTPUT:  Pointer to actually size of heap
 *                *realresultptr: Returns pointer to actual pointer than
 *                           was allocated, before any adjustments for
 *                           memory alignment.  This is the value that
 *                           must be passed to "free()"
 *
 *  returns:      pointer to aligned memory.
 *
 *  Note that "sizeptr" reflects the size of the "aligned" memory, and
 *  note the actual size of the memory that has been allocated.
 *=======================================================================*/

cell *allocateHeap(long *sizeptr, void **realresultptr) {
        void *space = malloc(*sizeptr + sizeof(cell) - 1);
        *realresultptr = space;
        return (void *) ((((long)space) + (sizeof(cell) - 1)) & ~(sizeof(cell) - 1));
}


/*=========================================================================
 * FUNCTION:      CurrentTime_md
 * TYPE:          Public link routine
 * OVERVIEW:      Get the system time in ms since 1970
 * INTERFACE:
 *   parameters:  none
 *   returns:     time
 *=======================================================================*/

ulong64
CurrentTime_md(void) {
    return sysTimeMillis();
}


/*=========================================================================
 * FUNCTION:      InitializeNativeCode
 * TYPE:          initialization
 * OVERVIEW:      called at start up to perform machine-specific initialization.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/
void InitializeNativeCode() {
    extern void runtime2_md_init(void);
    runtime2_md_init();
}


/*=========================================================================
 * FUNCTION:      nativeFinalization
 * TYPE:          initialization
 * OVERVIEW:      called at start up to perform machine-specific initialization.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void FinalizeNativeCode() {
}



/*=========================================================================
 * Async I/O Routines
 *=======================================================================*/

#if ASYNCHRONOUS_NATIVE_FUNCTIONS


#if NEWWIN32ASYNCSYSTEM
/*
 * Parameters to processAcyncThread
 */
static THREAD nextThread;
static void (*nextRoutine)(THREAD);


/*
 * Routine called from runtime2_md.c when a thread is unblocked
 */
void processAcyncThread(void) {
    /*
     * PIch up parms
     */
        THREAD thread       = nextThread;
        void (*rtn)(THREAD) = nextRoutine;

        /*
         * Clear input queue
         */
        nextRoutine = 0;
        nextThread  = 0;

        /*
         * Sanity check
         */
        if(thread == 0 || rtn == 0) {
            fatalError("processAcyncThread problem");
        }

        /*
         * Execute async function
         */
        (rtn)(thread);
}


/*=========================================================================
 * FUNCTION:      CallAsyncNativeFunction_md()
 * TYPE:          Public link routine for asynchronous native methods
 * OVERVIEW:      Call an asynchronous native method
 * INTERFACE:
 *   parameters:  thread pointer, native function pointer
 *   returns:     <nothing>
 *
 * Note: This is just an example implementation for demonstration purposes.
 *       and does not actually work very well because eventually the WIN32
 *       program seems to run out of thread resources and no new threads
 *       seem to run. Typically no error code is ever returned!
 *
 *=======================================================================*/

void CallAsyncNativeFunction_md(THREAD thisThread, void (*afp)(THREAD)) {

    /*
     * Save the thread parameters
     */
    nextRoutine = afp;
    nextThread  = thisThread;

    /*
     * Release a thread. This function will cause
     * processAcyncThread to be called
     */
    releaseAsyncThread();

    /*
     * Wait until it is running and has cleared its input queue
     */
    while(nextThread != 0) {
        Yield_md();
    }
}


#else


/*
 * This is the old version that created a new thread for each I/O operation
 */
void CallAsyncNativeFunction_md(THREAD thisThread, void (*afp)(THREAD)) {
    extern int beginthread(void (*rtn), void *parm);
    int res;

    res = beginthread(afp, thisThread);
    if (res == -1) {
        char buf[100];
        sprintf(buf, "Error from _beginthread %d", res);
        fatalError(buf);
    }
}
#endif


#endif
