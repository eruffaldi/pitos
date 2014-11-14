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
 * SUBSYSTEM: Palm OS event handler
 * FILE:      event.c
 * OVERVIEW:  This file defines the routines to intercept Palm OS events
 *            and invoke the registered handler methods for events
 *            that are made visible at the Java level.
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

#include <global.h>

int wantPalmSysKeys;

/*=========================================================================
 * Native functions of com.sun.kjava.Spotlet
 *=======================================================================*/

void Java_com_sun_kjava_Spotlet_setPalmEventOptions(void)
{
    wantPalmSysKeys = popStack() & WANTSYSTEMKEYS;
}



#ifdef PILOT

#include "KVM.h"

void
Java_com_sun_kjava_Spotlet_beamSend(void) {
    ExgSocketType exgSocket;
    Err err = 0;

    /* Read the bytearray parameter */
    BYTEARRAY byteArray = topStackAsType(BYTEARRAY);

    /* Get the size of the bytearray to send */
    ULong size = byteArray->length;

    /* Important to init structure to zeros */
    MemSet(&exgSocket, sizeof(exgSocket), 0);
    exgSocket.description = "Application data";
    exgSocket.name        = "Application data";
    exgSocket.length  = size;
    exgSocket.target  = KVMCreator;

    /* Put data to destination */
    err = ExgPut(&exgSocket);
    if (!err) {
        ExgSend(&exgSocket, &byteArray->bdata, size, &err);
        ExgDisconnect(&exgSocket, err);
    }

    /* Store return value */
    topStack = err ? false : true;
}

/*======================================================================
 * FUNCTION:     getFlashID()java.lang.String
 * CLASS:        spotless.Spotlet
 * TYPE:         static native function
 * OVERVIEW:     Get the Palm flashID.
 * INTERFACE (operand stack manipulation):
 *   parameters: none
 *   returns:    String containing flashID
 *====================================================================*/

void
Java_com_sun_kjava_Spotlet_getFlashID(void) {
    Word retVal;
    Word bufLen;
    CharPtr bufP;
    Short count;
    char *theID = 0;
    STRING_INSTANCE result;

    retVal = SysGetROMToken(0, sysROMTokenSnum, (BytePtr*) &bufP, &bufLen);

    if ((!retVal) && (bufP) && ((Byte) *bufP != 0xFF)) {
        /* valid serial number; make a string out of it */
        theID = mallocBytes(bufLen+1);
        for (count = 0; count<bufLen; count++) {
            theID[count] = bufP[count];
        }
        theID[bufLen] = 0;
    }

    result = theID ? instantiateString(theID, strlen(theID)) : NULL;
    pushStackAsType(STRING_INSTANCE, result);
}
#else
void Java_com_sun_kjava_Spotlet_getFlashID(void) {
    fatalError("Not implemented");
}

void Java_com_sun_kjava_Spotlet_beamSend(void) {
    fatalError("Not implemented");
}
#endif
