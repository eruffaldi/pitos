/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*
 * This implements the "events:" protocol
 *
 * @author  Nik Shaylor
 * @version 1.0 1/15/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Local Variables
 *=======================================================================*/

#define MAXPARMLENGTH 20

static THREAD waitingThread;
static int  opened;
static cell parameter[MAXPARMLENGTH];
static int  index;
static int  count;

/*=========================================================================
 * Global Routines
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      InitializeEvents
 * TYPE:          global function
 * OVERVIEW:      Initialize the event system.
 *                We have to explicitly initialize the event system
 *                so that the KVM can shutdown and restart cleanly.
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void InitializeEvents()
{
    waitingThread = 0;
    opened  = FALSE;
    index = 0;
    count = 0;
}

/*=========================================================================
 * Protocol Routines
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getEvent
 * TYPE:          local function
 * OVERVIEW:      Wait for a system event
 * INTERFACE:
 *   parameters:  time to wait if no events are forthcoming
 *   returns:     flag to say if an event was found
 *=======================================================================*/

static int getEvent(ulong64 wakeupTime) {
    KVMEventType event;
    bool_t forever;

    /* We wait forever if there is nothing else to run */
    if (ActiveThreadCount == 0) {
        forever = ll_zero_eq(wakeupTime);
    } else {
        forever = FALSE;
        ll_setZero(wakeupTime);
    }

    index = 0;
    while (TRUE) {
        /*  Get the next available event */
        if (!GetNextKVMEvent(&event, forever, wakeupTime)) {
            return FALSE;
        }
        parameter[0] = (cell)event.type;
        count = 2;

        switch (event.type) {
            case penDownKVMEvent:
            case penUpKVMEvent:
            case penMoveKVMEvent:
                parameter[1] = (cell)event.screenX;
                parameter[2] = (cell)event.screenY;
                count++;
                return TRUE;

            case keyDownKVMEvent:
                parameter[1] = (cell)event.chr;
                return TRUE;

            case beamReceiveKVMEvent:
                parameter[1] = (cell)event.beam;
                return TRUE;

            case appStopKVMEvent:
                ERROR_THROW(0);
                return TRUE;

            default:; /* do nothing, but continue in loop */
        }
    }
}


 /*=========================================================================
  * FUNCTION:      BetterHandleEvent
  * TYPE:          Global C function
  * OVERVIEW:      Wait for a system event
  * INTERFACE:
  *   parameters:  time to wait if no events are about
  *   returns:     none
  *=======================================================================*/

void BetterHandleEvent(ulong64 wakeupTime) {

    if (!opened) {
        if (ActiveThreadCount == 0 && ll_zero_ne(wakeupTime)) {
            SLEEP_UNTIL(wakeupTime);
        }
        return;
    }

    if (waitingThread != 0 && getEvent(wakeupTime)) {
        --count;
        (void)popStackForThread(waitingThread);     
        pushStackForThread(waitingThread, parameter[index++]);
        resumeThread(waitingThread);
        waitingThread = 0;
    }

}


/*=========================================================================
 * Protocol Methods
 *=======================================================================*/

 /*=========================================================================
  * FUNCTION:      open()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_open(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (opened) {
        raiseException("java/lang/IllegalAccessException");
    }
    opened = TRUE;
}

 /*=========================================================================
  * FUNCTION:      close()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_close(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (!opened) {
        raiseException("java/lang/IllegalAccessException");
    }
    opened = FALSE;
}


 /*=========================================================================
  * FUNCTION:      readInt0()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readInt(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (count == 0) {
        waitingThread = CurrentThread;
        pushStack(0);  /* For Dear Uncle Frank's checker */
        suspendThread();
    } else {
        --count;
        pushStack(parameter[index++]);
    }
 }


 /*=========================================================================
  * FUNCTION:      readByteArray()Z (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.events.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read byte array from the event queue
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the byte array
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_events_PrivateInputStream_readByteArray(void) {
    INSTANCE instance = popStackAsType(INSTANCE);
    (void)instance;
    if (count == 0) {
        raiseException("java/lang/IllegalAccessException");
    } else {
        --count;
        pushStack(parameter[index++]);
    }
}

