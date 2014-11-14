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
 * FILE:      runtime_md2.c
 * AUTHOR:    Nik Shaylor
 *
 * Split out from runtime_md.c because of conflicts between windows.h
 * and global.h
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <machine_md.h>
#include <main.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#define NOMSG
#include <windows.h>
#include <process.h>
#include <winsock.h>

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
        SYSTEMTIME st;

        /* initialize */
        memset(&st, 0, sizeof(st));
        memset(&date, 0, sizeof(date));

        GetSystemTime(&st);

        // initialize calendar fields
        date[YEAR] = st.wYear;
        date[MONTH] = st.wMonth;
        date[DAY_OF_MONTH] = st.wDay;
        date[HOUR] = st.wHour;
        date[MINUTE] = st.wMinute;
        date[SECOND] = st.wSecond;
        date[MILLISECOND] = st.wMilliseconds;
        return date;
}

/*=========================================================================
 * FUNCTION:      Yeild_md()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Yield the current thread
 * INTERFACE:
 *   parameters:  none
 *   returns:     current time, in centiseconds since startup
 *=======================================================================*/

void Yield_md(void) {
    Sleep(0);
}


#if ASYNCHRONOUS_NATIVE_FUNCTIONS


/*
 * Windows mutex object
 */
CRITICAL_SECTION csect;

/*=========================================================================
 * FUNCTION:      enterSystemCriticalSection()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Wait on the system mutex
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void enterSystemCriticalSection(void) {
    EnterCriticalSection( &csect );
}


/*=========================================================================
 * FUNCTION:      exitSystemCriticalSection()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Release the system mutex
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void exitSystemCriticalSection(void) {
    LeaveCriticalSection( &csect );
}


/*=========================================================================
 * Async thread software
 *=======================================================================*/

#if NEWWIN32ASYNCSYSTEM

void fatalError(char *);

/*
 * Handle for thread blocking
 */
HANDLE EventHandle;

/*=========================================================================
 * FUNCTION:      asyncThread()
 * TYPE:          local function
 * OVERVIEW:      Main I/O thread processing loop
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

static void asyncThread(void) {
        for(;;) {
                if(WaitForSingleObject(EventHandle, INFINITE) == WAIT_FAILED) {
                        fatalError("Could not WaitForSingleObject");
                }
                processAcyncThread();
    }
}

/*=========================================================================
 * FUNCTION:      beginthreads()
 * TYPE:          local  function
 * OVERVIEW:      Start 5 I/O thread
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

static void beginthreads() {
    int i;
    EventHandle = CreateEvent(0, FALSE, FALSE, 0);
    for(i = 0 ; i < 5 ; i++) {
        _beginthread((void (*))asyncThread, 0, 0);
    }
}


/*=========================================================================
 * FUNCTION:      releaseAsyncThread()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Release an I/O thread
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void releaseAsyncThread(void) {
    if(SetEvent(EventHandle) == FALSE)
        fatalError("Could not SetEvent");
}

#else

/*=========================================================================
 * FUNCTION:      beginthread()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Create an I/O thread
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

int beginthread(void (*rtn), void *parm) {
    int handle = _beginthread(rtn, 0, parm);
    return handle;
}


#endif

#endif


/*---------------------------------------------------------------------------*/
/*                              runtime2_md_init                             */
/*---------------------------------------------------------------------------*/

/* We need to make sure runtime2_md_init is only called once when
 * -jam -repeat option is supplied.
 */
static int runtime2_md_inited = FALSE;

void runtime2_md_init(void) {
    if (runtime2_md_inited) {
	return;
    }
#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    InitializeCriticalSection(&csect);
#if NEWWIN32ASYNCSYSTEM
    beginthreads();
#endif
#endif
    runtime2_md_inited = TRUE;
}

#define FT2INT64(ft) \
	((ulong64)(ft).dwHighDateTime << 32 | (ulong64)(ft).dwLowDateTime)

ulong64
    sysTimeMillis(void) {
    static ulong64 fileTime_1_1_70 = 0;
    SYSTEMTIME st0;
    FILETIME   ft0;

    if (fileTime_1_1_70 == 0) {
        /*
	 * Initialize fileTime_1_1_70 -- the Win32 file time of midnight
	 * 1/1/70.
         */ 
        memset(&st0, 0, sizeof(st0));
        st0.wYear  = 1970;
        st0.wMonth = 1;
        st0.wDay   = 1;
        SystemTimeToFileTime(&st0, &ft0);
	fileTime_1_1_70 = FT2INT64(ft0);
    } 

    GetSystemTime(&st0);
    SystemTimeToFileTime(&st0, &ft0);

    /* file times are in 100ns increments, i.e. .0001ms */
    return (FT2INT64(ft0) - fileTime_1_1_70) / 10000;
}
