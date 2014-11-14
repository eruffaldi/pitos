/*=========================================================================
 * Spotless Virtual Machine
 *=========================================================================
 * SYSTEM:    Spotless VM
 * SUBSYSTEM: Machine-specific implementations needed by virtual machine
 * FILE:      runtime_md.c
 * AUTHOR:    Ruffaldi Emanuele
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

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
	void *space = malloc(*sizeptr + sizeof(16) - 1);
	*realresultptr = space;
	return (void *) ((((long)space) + (sizeof(16) - 1)) & ~(sizeof(16) - 1));
}



/*=========================================================================
 * FUNCTION:      showStack
 * TYPE:          debug
 * OVERVIEW:      called when we receive a coredump signal
 * INTERFACE:
 *   parameters:  signal
 *   returns:     none
 *=======================================================================*/

static void signal_handler(int sig) {
    char *name;

    switch (sig) {
        case SIGILL:
            name = "SIGILL";
            break;
        case SIGABRT:
            name = "SIGABRT";
            break;
        case SIGBUS:
            name = "SIGBUS";
            break;
        case SIGSEGV:
            name = "SIGSEGV";
            break;
        default:
            name = "??";
    }
    fprintf(stderr, "received signal %s\n", name);
    //printStackTrace();
    //printProfileInfo();

    signal(SIGILL,  NULL);
    signal(SIGABRT, NULL);
    signal(SIGBUS,  NULL); 
    signal(SIGSEGV, NULL); 

    abort();
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
    /*
     * Create signal handlers to dump Java stack in case of coredumps
     */
//  signal(SIGILL,  signal_handler);
//  signal(SIGABRT, signal_handler);
//	signal(SIGBUS,  signal_handler); 
// signal(SIGSEGV, signal_handler); 
	fprintf(stderr,"JVM:Initializing Native Code...\n");
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
#if INCLUDE_ALL_CLASSES
    //FinalizeWindowSystem();
#endif
	fprintf(stderr,"JVM:Finalizing Native Code...\n");
}


/*=========================================================================
 * FUNCTION:      CurrentTime_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Returns the current time. 
 * INTERFACE:
 *   parameters:  none
 *   returns:     current time, in centiseconds since startup
 *=======================================================================*/

ulong64
CurrentTime_md(void)
{
	return 0;
}

#define MAXCALENDARFLDS 15

#define YEAR 1
#define MONTH 2
#define DAY_OF_MONTH 5
#define HOUR 10
#define MINUTE 12
#define SECOND 13
#define MILLISECOND 14

static unsigned long date[MAXCALENDARFLDS];

/*=========================================================================
 * Helper variables and methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      Calendar_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Initializes the calendar fields, which represent the
 *                Calendar related attributes of a date.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 * AUTHOR:        Tasneem Sayeed
 *=======================================================================*/
unsigned long *
Calendar_md(void)
{
        /* initialize */
        memset(&date, 0, sizeof(date));


        // initialize calendar fields
        date[YEAR] = 2001;
        date[MONTH] = 3;
        date[DAY_OF_MONTH] = 4;
        date[HOUR] = 16;
        date[MINUTE] = 00;
        date[SECOND] = 00;
        date[MILLISECOND] = 00;
        return date;
}

