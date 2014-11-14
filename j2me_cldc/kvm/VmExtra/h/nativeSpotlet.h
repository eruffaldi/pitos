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
 * FILE:      event.h
 * OVERVIEW:  This file defines the routines to intercept Palm OS events
 *            and invoke the registered handler methods for events
 *            that are made visible at the Java level.
 * AUTHOR:    Doug Simon, Sun Labs
 *=======================================================================*/

#define FOREVER TRUE
#define FIXED_DELAY FALSE

/*=========================================================================
 * This global variable hold references to the JAVATHREAD instance
 * used to execute event handlers and the object which includes the
 * registered event handling methods
 *=======================================================================*/

extern THREAD EventThread;
extern INSTANCE SpotletFocus;

/*=========================================================================
 * Event handler function tables and associated macros
 *=======================================================================*/

extern METHOD eventHandlerTable[];

/*  the type of events for which handlers can be registered. Corresponds */
/*  exactly with the handler methods defined in spotless.Spotlet. */
/*  The PEN* constants must be the first three elements in the list */
/*  and in the order DOWN,UP,MOVE to correspond with the definition in the */
/*  events enumerated type in Event.h */

enum KVMEventTypes {
	beamReceiveKVMEvent = 0,
	penDownKVMEvent = 1,
	penUpKVMEvent   = 2,
	penMoveKVMEvent = 3,
	keyDownKVMEvent = 4,
	lastKVMEvent =    5,
	appStopKVMEvent = 5
};


bool_t HandleEvent(ulong64 nextTimerEvent);


/*  The event record. */
typedef struct {
    enum KVMEventTypes type;
	BYTEARRAY      beam;
    short          screenX;
    short          screenY;
	short          chr;			/* Palm has 16-bit chars */
} KVMEventType;


/* The option flags when registering for events. */
enum eventOptions {
    NOEVENTOPTIONS = 0x00,	
    WANTSYSTEMKEYS = 0x01	/* User wants system "hard" key events */
};


bool_t
GetNextKVMEvent(KVMEventType *event, bool_t forever, ulong64 waitUntil);

void RegisterEventHandlers(INSTANCE spotlet, int option);
void UnregisterEventHandlers(void);

