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
 * SUBSYSTEM: Bytecode interpreter
 * FILE:      events.h
 * OVERVIEW:  This file defines the common aspects of the
 *            interpreter event polling code.
 * AUTHOR:    Nik Shaylor 4/20/00
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Scheduling macros.
 *
 * These are here to isolate the scheduling code out of interpret.c
 *=======================================================================*/

/*
 * Indicate task switching is necessary
 */
#define signalTimeToReschedule() (Timeslice = 0)

/*
 * Determine if it is time to schedule
 */
#define isTimeToReschedule() (Timeslice-- == 0)

/*
 * Reschedule
 *
 * Call CheckTime_md() and HandleEvent() to poll for events then try
 * and switch threads.
 *
 */

#if INCLUDE_ALL_CLASSES
void BetterHandleEvent(ulong64);
#else
#define BetterHandleEvent(t)
#endif

#define reschedule()                                                    \
        do  {                                                           \
            ulong64 wakeupTime;                                         \
            if(AliveThreadCount == 0) {                                 \
                return;   /* end of program */                          \
            }                                                           \
            checkTimerQueue(&wakeupTime);                               \
            BetterHandleEvent(wakeupTime);                              \
        } while(!SwitchThread());

