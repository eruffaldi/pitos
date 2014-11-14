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
 * SUBSYSTEM: Thread (concurrency) management
 * FILE:      thread.c
 * OVERVIEW:  This file defines the structures and operations
 *            that are needed for Java-style multitasking.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (nano libraries, new monitors)
 *            Edited by Nik Shaylor 09/1999 to allow asynchronous I/O
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * Read the description of thread and monitor structures in Thread.h
 *=======================================================================*/

/*=========================================================================
 * Local include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Global variables needed for multitasking
 *=======================================================================*/

THREAD AllThreads;                      /*  List of all threads */
THREAD CurrentThread;           /*  Current thread pointer */
THREAD TimerQueue;                      /*  Threads waiting for a timer interrupt*/


/* RunnableThreads is a circular queue of threads.  RunnableThreads is
 * either NULL or points to the >>last<< element of the list.  This
 * makes it easier to add to either end of the list *
 */
static THREAD RunnableThreads;          /*  Runnable thread list */

int AliveThreadCount;   /*  Number of alive threads in the system */
int ActiveThreadCount;  /*  Number of active threads in the system */
int Timeslice;                  /*  Time slice counter for multitasking */

/*=========================================================================
 * Static declarations needed for file
 *=======================================================================*/


#define QUEUE_ADDED TRUE
#define QUEUE_REMOVED FALSE

#if TRACEMONITORS
static void
TraceMonitor(THREAD thread, MONITOR mid, THREAD *queue, bool_t added);
#else
#  define TraceMonitor(thread, mid, queue, what)
#endif

#if TRACETHREADING
static void TraceThread(THREAD thread, char *what);
#else
#define TraceThread(thread, what)
#endif


typedef enum { AT_START, AT_END } queueWhere;
static void addThreadToQueue(THREAD *queue, THREAD, queueWhere where);
static THREAD removeQueueStart(THREAD *queue);
static bool_t removeFromQueue(THREAD *queue, THREAD waiter);


static void addMonitorWait(MONITOR mid, THREAD thread);
static void addCondvarWait(MONITOR mid, THREAD thread);
static void removeMonitorWait(MONITOR mid);
static void removeCondvarWait(MONITOR mid, bool_t notifyAll);

static void removePendingAlarm(THREAD thread);

/*=========================================================================
 * Global public multitasking operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      storeExecutionEnvironment()
 *                loadExecutionEnvironment()
 * TYPE:          private operations
 * OVERVIEW:      These functions are used for storing and restoring
 *                virtual machine registers upon task switching.
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     <nothing>
 *=======================================================================*/

void storeExecutionEnvironment(THREAD thisThread)
{
    /* Save the current thread execution environment
     * (virtual machine registers) to the thread structure.
         */
    thisThread->fpStore = fp;
    thisThread->spStore = sp;
    thisThread->ipStore = ip;
}

void loadExecutionEnvironment(THREAD thisThread)
{
    /*  Restore the thread execution environment */
    /*  (VM registers) from the thread structure */
    fp = thisThread->fpStore;
        lp = FRAMELOCALS(fp);
        cp = fp->thisMethod->ofClass->constPool;
    sp = thisThread->spStore;
    ip = thisThread->ipStore;
}

/*=========================================================================
 * FUNCTION:      SwitchThread()
 * TYPE:          public global operations
 * OVERVIEW:      Fundamental task switching operation:
 *                switches to the next thread to be executed.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     Boolean showing if there is now a current thread
 *
 * NOTE:
 *
 * On entry to this routine there are three states the current thread
 * pointer can be in:
 *
 * CurrentThread == NIL
 *
 *     There is no current thread because suspendThread() was called.
 *
 * CurrentThread != NIL && CurrentThread->state == THREAD_ACTIVE
 *
 *     This is the normal case when the current thread's timeslice has
 *     run out.
 *
 * CurrentThread != NIL && CurrentThread->state != THREAD_ACTIVE
 *
 *     This is an atypical condition when suspendThread() was called
 *     while we were in supervisor mode. In this case the routine
 *     must spin until CurrentThread->state == THREAD_ACTIVE.
 *
 *=======================================================================*/

bool_t SwitchThread(void)
{
    THREAD threadToAdd = NIL;

    if (CurrentThread != NIL) {

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
        if (CurrentThread->pendingException != NIL) {
            fatalError("Bad pendingException found in SwitchThread()");
        }
#endif

        if (CurrentThread->state == THREAD_ACTIVE) {
            /* If there is only one thread, or we can't switch threads then
               just return */
            if (ActiveThreadCount == 1) {
                Timeslice = CurrentThread->timeslice;
                return TRUE;
            } else {
                /* Save the VM registers.  Indicate that this thread is to */
                /* to be added to the dormant thread queue */
                storeExecutionEnvironment(CurrentThread);
                threadToAdd = CurrentThread;
                CurrentThread = NIL;
            }
        } else {
            fatalError("Inactive thread found in SwitchThread()");
        }
    }

    /*  Try and find a thread */
    CurrentThread = removeQueueStart(&RunnableThreads);

    /* If there was a thread to add then do so */
    if (threadToAdd != NIL) {
        addThreadToQueue(&RunnableThreads, threadToAdd, AT_END);
    }

    /* If nothing can run then return FALSE */
    if (CurrentThread == NIL) {
        return FALSE;
    }

#if ENABLEPROFILING
    ThreadSwitchCounter++;
#endif
    /*  Load the VM registers of the new thread */
    loadExecutionEnvironment(CurrentThread);

    /* Diagnostics */
    TraceThread(CurrentThread, "Switch to this thread");

    /*  Load new time slice */
    Timeslice = CurrentThread->timeslice;

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    if (CurrentThread->pendingException != NIL) {
        char *pending = CurrentThread->pendingException;
        CurrentThread->pendingException = NIL;
        raiseException(pending);
    }
#endif

    return TRUE;
}


/*=========================================================================
 * Public operations on individual thread instances
 *=======================================================================*/

/*=========================================================================
 * Constructors and destructors
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      BuildThread()
 * TYPE:          public global operation
 * OVERVIEW:      Build a new VM-level thread by instantiating and
 *                initializing the necessary structures.
 * INTERFACE:
 *   returns:     a new thread instance
 * NOTE:          The created thread is an internal structure that does
 *                not map one-to-one with Java class 'Thread.class'.
 *                One-to-one mapping is implemented by structure
 *                JAVATHREAD.
 *=======================================================================*/

THREAD BuildThread(void)
{
    THREAD newThread;

    newThread = (THREAD)callocObject(SIZEOF_THREAD, GCT_THREAD);

    START_TEMPORARY_ROOT(newThread)
        /*  New task needs its own execution stack */
        newThread->stack = (STACK)callocObject(sizeof(struct stackStruct) / CELL,
                                                                                           GCT_EXECSTACK);
        newThread->stack->next = NULL;
        newThread->stack->size = STACKCHUNKSIZE;

        TraceThread(newThread, "Creating thread");

        if (TRACE_STACK_CHUNKS) {
            fprintf(stdout, "thread %lx, init chunk %lx, size %ld\n",
                    (long)newThread, (long)newThread->stack,
                    (long)STACKCHUNKSIZE);
        }

    END_TEMPORARY_ROOT

    /*  Time slice will be initialized to default value */
    newThread->timeslice = BASETIMESLICE;

    /*  Initialize the state */
    newThread->state = THREAD_JUST_BORN;

    /* Add to the alive thread list  */
    newThread->nextAliveThread = AllThreads;
    AllThreads = newThread;

    return(newThread);
}

/*=========================================================================
 * FUNCTION:      DismantleThread()
 * TYPE:          public global operation
 * OVERVIEW:      Free off the thread's resources
 * INTERFACE:
 *   parameters:  The thread pointer
 *   returns:     <none>
 *=======================================================================*/

static void
DismantleThread(THREAD thisThread)
{

    if (AllThreads == thisThread) {
        AllThreads = AllThreads->nextAliveThread;
    } else {
        THREAD prevThread = AllThreads;
        /* Remove from alive thread list */
        while (prevThread->nextAliveThread != thisThread) {
            prevThread = prevThread->nextAliveThread;
        }
        prevThread->nextAliveThread = thisThread->nextAliveThread;
    }
    thisThread->nextAliveThread = NULL;
    if (inTimerQueue(thisThread)) {
        removePendingAlarm(thisThread);
    }
}


/*=========================================================================
 * FUNCTION:      InitializeThreading()
 * TYPE:          public global operation
 * OVERVIEW:      Create the first low-level system thread and
 *                initialize it properly.  Initialize VM registers
 *                accordingly.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeThreading(void)
{
    THREAD mainThread;

    /* AllThreads is initialized to NULL by the garbage collector */

    /*  Create the main execution thread of the system */
        mainThread = BuildThread();

    /*  Ensure that the thread list is properly initialized */
    /*  and set mainThread as the active (CurrentThread) thread */
    mainThread->nextThread = NIL;
    AliveThreadCount  = 1;
    ActiveThreadCount = 1;
    Timeslice = BASETIMESLICE;
    mainThread->state = THREAD_ACTIVE;

    /*  Initialize VM registers */
    CurrentThread = mainThread;
    RunnableThreads = NULL;
    TimerQueue = NULL;
    ip = NULL;
    fp = NULL;
    sp = NULL;
}

/*=========================================================================
 * FUNCTION:      InitializeJavaThread()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Initialize a Java-level thread with default values for
 *                its fields and attach it to the given VM level thread.

 * INTERFACE:
 *   parameters:  a VM level thread
 *   returns:     <nothing>
 * NOTE:          This operation replaces the InitializeJavaRuntime as we
 *                don't have to worry about ThreadGroups any more.
 *=======================================================================*/

void InitializeJavaThread(THREAD thread)
{
    JAVATHREAD javaThread;

    /*  Build the Java-level thread, initializing all the fields to zero and */
    /*  attach it to the given VM level thread */
    javaThread = (JAVATHREAD)callocObject(SIZEOF_JAVATHREAD, GCT_INSTANCE);
    thread->javaThread = javaThread;

    /*  Initialize the necessary fields of the Java-level thread structure. */
    javaThread->ofClass  = JavaLangThread;
    javaThread->priority = 5;
    javaThread->VMthread = thread;
}

/*=========================================================================
 * FUNCTION:      getVMthread()
 * TYPE:          public auxiliary operation
 * OVERVIEW:      Given a Java-level thread, ensure that the internal
 *                VM-level thread structure has been created.
 * INTERFACE:
 *   parameters:  JAVATHREAD pointer
 *   returns:     THREAD pointer
 *=======================================================================*/

THREAD getVMthread(JAVATHREAD javaThread)
{
    /*  Create the VM-level thread structure if necessary */
    THREAD VMthread = javaThread->VMthread;
    if (!VMthread) {
        VMthread = BuildThread();
        javaThread->VMthread = VMthread;
        VMthread->javaThread = javaThread;
    }
    return VMthread;
}

/*=========================================================================
 * Thread activation and deactivation
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      initThreadBehavior()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Set the behavior of a thread without the context of
 *                a current execution stack (i.e. don't use sp).
 * INTERFACE:
 *   parameters:  thread pointer, pointer to a method object and the sync
 *                object that will be used if this is a synchronized method.
 *   returns:     <nothing>
 * NOTE:          You have to explicitly construct the local stack of
 *                this thread after returning from this call. Don't
 *                forget that 'this' must be the first local is this
 *                thisMethod is virtual.
 *                There are no safety checks here, so do not activate
 *                the same thread twice.
 *=======================================================================*/

static void
initThreadBehaviorFromThread(cell *argP, INSTANCE* exception);

void initThreadBehavior(THREAD thisThread, METHOD thisMethod, OBJECT syncObject)
{
    /* Note, thisThread == CurrentThread when booting.  This code below is slightly
     * inefficient, but it's not worth the hassle of fixing for a one-time
     * case
     */
    THREAD current = CurrentThread;
    if (current != NULL) {
        storeExecutionEnvironment(current);
    }
    CurrentThread = thisThread;

    sp = (thisThread->stack->cells - 1) + thisMethod->argCount;
    fp = NULL;
    ip = KILLTHREAD;
    pushFrame(thisMethod);
    if (thisMethod->accessFlags & ACC_SYNCHRONIZED) {
        fp->syncObject = syncObject;
        pushFrame(RunCustomCodeMethod);
        pushStackAsType(CustomCodeCallbackFunction,
                        &initThreadBehaviorFromThread);
    } else {
        fp->syncObject = NULL;
    }
    storeExecutionEnvironment(thisThread);
    if (current) {
        loadExecutionEnvironment(current);
    }
    CurrentThread = current;
}

/* FUNCTION:      initThreadBehaviorFromThread()
 * TYPE:          private initialization of a class
 * OVERVIEW:      This function performs any thread initialization that can
 *                only be performed by the thread on itself after it has
 *                begun running, and not by whoever created the threads.
 *
 * Notes:         This is a "CustomCode" callback function.
 *
 *                Currently, it only handles the method being synchronized,
 *                and needing  to enter the monitor.  It may be extended to
 *                do more.
 *
 * INTERFACE:
 *   parameters:  argP:  Arguments on stack.  Not currently used
 *                exceptionP: NULL if called from the interpret. A pointer to
 *                    the current exception if called from the error handler.
 *                    We can either set the pointer to NULL to indicate we've
 *                    completely handled it, or we can replace it with a
 *                    different exception.
 *
 *   returns:     <nothing>
 *
 */
static void
initThreadBehaviorFromThread(cell *argP, INSTANCE* exceptionP) {
    if (!exceptionP) {
        popFrame();
        if (fp->thisMethod->accessFlags & ACC_SYNCHRONIZED) {
            monitorEnter(fp->syncObject);
        }
    } else {
        /* We have no interest in dealing with exceptions */
    }
}


/*=========================================================================
 * FUNCTION:      startThread()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Make a thread alive, but suspended.
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     <nothing>
 * NOTE:          There are no safety checks here, so please
 *                do not activate the same thread twice.
 *=======================================================================*/

void startThread(THREAD thisThread)
{
    TraceThread(thisThread, "Starting");

    /*  Add one to the count of alive Java threads */
    thisThread->state = THREAD_SUSPENDED;
    AliveThreadCount++;
}

/*=========================================================================
 * FUNCTION:      stopThread()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Stop a thread and free its resources
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     <nothing>
 * NOTE:          There are no safety checks here, so please
 *                do not stopThread the same thread twice.
 *=======================================================================*/

void stopThread(void)
{
    THREAD thisThread = CurrentThread;

    TraceThread(thisThread, "Stopping");

    /* First suspend the thread */
    suspendThread();

    /* Always force the end of the current thread */
    CurrentThread = NIL;

    /* Make the thread be no longer alive */
    thisThread->state = THREAD_DEAD;
    AliveThreadCount--;

    /* Then kill it off */
    DismantleThread(thisThread);
}


/*=========================================================================
 * FUNCTION:      suspendThread()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Suspend the currently executing thread.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 * NOTE:          There are no safety checks here, so please
 *                do not suspend the same thread twice.
 *
 *                When this function is used with an asynchronous
 *                callback routine that calls resumeThread() it
 *                is vital that this routine is called *BEFORE*
 *                the callback routine can execute. **NS**
 *=======================================================================*/

void suspendThread(void)
{
    TraceThread(CurrentThread, "Suspending");

    /* Bump the active thread count */
    ActiveThreadCount--;

    /* Save the VM registers of the currently executing thread */
    storeExecutionEnvironment(CurrentThread);

    /* Set the flag to show that it is inactive */
    CurrentThread->state = THREAD_SUSPENDED;

    /* Show that there is no current thread anymore */
    /* Only do this if we are not in supervisor mode */
    CurrentThread = NIL;

    /* Signal time to reschedule the interpreter */
    signalTimeToReschedule();
}


/*=========================================================================
 * FUNCTION:      resumeThread()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Resume a thread by placing it from the dormant
 *                thread chain.
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     <nothing>
 * NOTE:          There are no safety checks here, so please
 *                do not suspend the same thread twice.
 *                All changed **NS**
 *=======================================================================*/

void resumeThread(THREAD thisThread)
{
    TraceThread(thisThread, "Resuming");

    if (!thisThread->state == THREAD_SUSPENDED) {
        fatalError("Attempt to resume non-suspended thread");
    }

    /* Set the flag to show that it is now active */
    thisThread->state = THREAD_ACTIVE;

    /* Bump the active thread count */
    ActiveThreadCount++;

    /* Check if we are trying to restart CurrentThread
     * Normally this is an error, but if we are
     * spinning in supervisor mode it is normal
         */
    if (thisThread == CurrentThread) {
        fatalError("Attempt to resume CurrentThread in non-supervisor mode");
        /* We don't need to do anything else because SwitchThread will be
         * spinning on the thread state, so we just have to get out
         * of whatever sort of callback routine we are in, and it will then
         * be able to proceed */
    } else {
        /* If the new thread has higher priority then */
        /* add to head of the wait queue and signal that */
        /* it is time to reschedule the processor */
        addThreadToQueue(&RunnableThreads, thisThread, AT_END);
    }
}



/*=========================================================================
 * FUNCTION:      CallAsyncNativeFunction()
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

#if ASYNCHRONOUS_NATIVE_FUNCTIONS

int AsyncThreadCount;

void asyncFunctionProlog() {
    START_CRITICAL_SECTION
        AsyncThreadCount++;
    END_CRITICAL_SECTION
    /* Suspend the current thread */
    suspendThread();
}

#endif

/*=========================================================================
 * FUNCTION:      AsyncResumeThread()
 * TYPE:          Public link routine for asynchronous native methods
 * OVERVIEW:      Spin until all asynchronous I/O is finished
 * INTERFACE:
 *   parameters:  A THREAD
 *   returns:     <nothing>
 *=======================================================================*/

#if ASYNCHRONOUS_NATIVE_FUNCTIONS

void asyncFunctionEpilog(THREAD thisThread) {
    resumeThread(thisThread);

    START_CRITICAL_SECTION
        --AsyncThreadCount;
        NATIVE_FUNCTION_COMPLETED();
    END_CRITICAL_SECTION
}

#endif

/*=========================================================================
 * FUNCTION:      RundownAsynchronousFunctions()
 * TYPE:          Public routine for garbage collector
 * OVERVIEW:      Spin until all asynchronous I/O is finished
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if ASYNCHRONOUS_NATIVE_FUNCTIONS

void RundownAsynchronousFunctions(void) {
    /* Wait until AsyncThreadCount is zero.
     * It is safe to do this outside of a critical section.
     */
    while (AsyncThreadCount != 0) {
        /* Wait */
        Yield_md();
    }
}

#endif


/*=========================================================================
 * FUNCTION:      isActivated()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Check if a thread is active
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     TRUE if the task is active, FALSE otherwise.
 *=======================================================================*/

int isActivated(THREAD thread)
{
    if (thread == NIL) {
        return FALSE;
    } else {
        int state = thread->state;
        return state == THREAD_ACTIVE || state == THREAD_SUSPENDED;
    }
}



#if INCLUDEDEBUGCODE

/*=========================================================================
 * Thread debugging
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      printThreadStatus()
 * TYPE:          public thread instance-level operation
 * OVERVIEW:      Print the status of given thread for debugging purposes.
 * INTERFACE:
 *   parameters:  thread pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void
printThreadStatus(THREAD thisThread)
{
    fprintf(stdout,"Thread %lx status:\n", (long)thisThread);

    if (thisThread == CurrentThread) {
        fprintf(stdout,"ACTIVE (CurrentThread)\n");
        /*  To print most up-to-date information we need to do this */
        storeExecutionEnvironment(CurrentThread);
    } else {
        fprintf(stdout, "%s\n",
                isActivated(thisThread) ? "ACTIVE (DORMANT)" : "SUSPENDED");
    }

    fprintf(stdout,"Timeslice: %ld Priority: %ld\n",
            thisThread->timeslice, thisThread->javaThread->priority);
    fprintf(stdout,"Instruction pointer: %lx\n", (long)thisThread->ipStore);

    fprintf(stdout,"Execution stack location: %lx\n", (long)thisThread->stack);
    fprintf(stdout,"Execution stack pointer: %lx \n", (long)(thisThread->spStore));

    fprintf(stdout,"Frame pointer: %lx \n", (long)(thisThread->fpStore));

    fprintf(stdout,"JavaThread: %lx VMthread: %lx\n",
        (long)thisThread->javaThread, (long)thisThread->javaThread->VMthread);
    if (thisThread->monitor != NULL) {
        START_TEMPORARY_ROOTS
            fprintf(stdout,
                    "Waiting on monitor %lx %s::%lx\n",
                    (long)thisThread->monitor, className(thisThread->monitor->object->ofClass), (long)thisThread->monitor->object);
        END_TEMPORARY_ROOTS
    }
    if (inTimerQueue(thisThread)) {
        fprintf(stdout, "In timer queue\n");
    }
    fprintf(stdout, "\n\n");
}

/*=========================================================================
 * FUNCTION:      printFullThreadStatus()
 * TYPE:          public thread instance-level operation
 * OVERVIEW:      Print the status of all the active threads in the system.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/


void printFullThreadStatus(void)
{
    THREAD thisThread;
    int    threadCount = 0;

    fprintf(stdout,"%d active threads\n", ActiveThreadCount);

    for (thisThread = AllThreads; thisThread != NULL;
           thisThread = thisThread->nextAliveThread) {
        fprintf(stdout,"Thread #%d\n", ++threadCount);
        printThreadStatus(thisThread);
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Timer implementation
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      registerAlarm()
 * TYPE:          timer queue
 * OVERVIEW:      Perform a callback on this thread
 * INTERFACE:
 *   parameters:  The thread pointer
 *                The time in the future at which the callback should be called
 *                The callback function
 *   returns:     no value
 *=======================================================================*/



void
registerAlarm(THREAD thread, long64 delta, void (*wakeupCall)(THREAD))
{
#if NEED_LONG_ALIGNMENT
    Java8 tdub;
#endif
    THREAD q, prev_q;

    /* set wakeupTime to now + delta.  Save this value in the thread */
    ulong64 wakeupTime = CurrentTime_md();
    ll_inc(wakeupTime, delta);      /* wakeUp += delta */
    SET_ULONG(thread->wakeupTime, wakeupTime);

    TraceThread(thread, "Adding to timer queue");

    /* Save the callback function in the thread */
    thread->wakeupCall = wakeupCall;

    /* insert this value into the clock queue, which is sorted by wakeup time.*/
    q = TimerQueue;
    prev_q = NULL;
    while (q != NULL) {
        ulong64 qtime = GET_ULONG(q->wakeupTime);
        if (ll_compare_ge(qtime, wakeupTime)) {
            break;
        }
        prev_q = q;
        q = q->nextAlarmThread;
    }
    if (prev_q != NULL) {
        /* This is the first item in the queue. */
        prev_q->nextAlarmThread = thread;
        thread->nextAlarmThread = q;
    } else {
        thread->nextAlarmThread = TimerQueue;
        TimerQueue = thread;
    }
    TraceThread(thread, "Added to timer queue");
}

/*=========================================================================
 * FUNCTION:      checkTimerQueue()
 * TYPE:          timer queue
 * OVERVIEW:      Remove objects, as necessary, from the timer queue and
 *                call their callback function.
 * INTERFACE:
 *   parameters:  A pointer to a ulong64.  This value will be set to the time
 *                at which the next clock event will occur.
 *   returns:     no value
 *=======================================================================*/


void
checkTimerQueue(ulong64 *nextTimerWakeup)
{
#if NEED_LONG_ALIGNMENT
    Java8 tdub;
#endif
    if (TimerQueue != NULL) {
        ulong64 now = CurrentTime_md();
        do {
            ulong64 firstTime = GET_ULONG(TimerQueue->wakeupTime);
            if (ll_compare_le(firstTime, now)) {
                /* Remove this item from the queue, and resume it */
                THREAD thread = TimerQueue;
                void (*wakeupCall)() = thread->wakeupCall;
                TraceThread(thread, "Removing from timer queue");
                TimerQueue = thread->nextAlarmThread;
                thread->nextAlarmThread = NULL;
                thread->wakeupCall = NULL; /* signal that not on queue */
                wakeupCall(thread);
                TraceThread(thread, "Removed from timer queue");
            } else {
                break;
            }
        } while (TimerQueue != NULL);
    }

    /* Now indicate when the next timer wakeup should happen */
    if (TimerQueue == NULL) {
        ll_setZero(*nextTimerWakeup);
    } else {
        *nextTimerWakeup = GET_ULONG(TimerQueue->wakeupTime);
    }

}

/*=========================================================================
 * FUNCTION:      removePendingAlarm();
 * TYPE:          timer queue
 * OVERVIEW:      Remove thread from the timer queue.
 * INTERFACE:
 *   parameters:  The thread to remove from the timer queue.
 *   returns:     no value
 *=======================================================================*/

static void
removePendingAlarm(THREAD thread) {
    THREAD q, prev_q;
    TraceThread(thread, "Purging from timer queue");
    for (q = TimerQueue, prev_q = NULL;
               q != NULL; prev_q = q, q = q->nextAlarmThread) {
        if (q == thread) {
            if (prev_q) {
                prev_q->nextAlarmThread = q->nextAlarmThread;
            } else {
                TimerQueue = q->nextAlarmThread;
            }
            q->nextAlarmThread = NULL;
            q->wakeupCall = NULL; /* indicate not on queue anymore */
            break;
        }
    }
    TraceThread(thread, "Purged from timer queue");
}

/*=========================================================================
 * FUNCTION:      inTimerQueue();
 * TYPE:          timer queue
 * OVERVIEW:      Determine if a thread is in the timer queue.
 * INTERFACE:
 *   parameters:  The thread
 *   returns:     TRUE if the thread is in the timer queue.
 *=======================================================================*/


bool_t inTimerQueue(THREAD thread) {
    return thread->wakeupCall != NULL;
}

/*=========================================================================
 * Monitor implementation
 *=======================================================================*/

static char* IllegalMonitorStateException =
          "java/lang/IllegalMonitorStateException";


/*=========================================================================
 * Operations on monitor instances
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      addMonitorWait
 * TYPE:          Monitor handler
 * OVERVIEW:      Called to indicate a thread is waiting on a specific monitor
 * INTERFACE:
 *   parameters:  mid:    Monitor
 *                thread: Thread waiting on the monitor
 *
 *=======================================================================*/

static void
addMonitorWait(MONITOR mid, THREAD thread) {
    /* Add to the wait queue */
    addThreadToQueue(&mid->monitor_waitq, thread, AT_END);
    thread->monitor = mid;

    TraceMonitor(thread, mid, &mid->monitor_waitq, QUEUE_ADDED);

    /* If the wait queue has no owner, perhaps we should try to run this */
    if (mid->owner == NULL) {
        removeMonitorWait(mid);
    }
}


/*=========================================================================
 * FUNCTION:      removeMonitorWait
 * TYPE:          Monitor handler
 * OVERVIEW:      Called to indicate a new thread can take ownership of the
 *                  monitor
 * INTERFACE:
 *   parameters:  mid:    Monitor
 *                thread: Thread waiting to be notified
 *=======================================================================*/

static void
removeMonitorWait(MONITOR mid)
{
    THREAD waiter = removeQueueStart(&mid->monitor_waitq);

    if (waiter != NULL) {
        /* Set the monitor's ownership and restore the entry count */
        mid->owner = waiter;
        mid->depth = waiter->monitor_depth;

        /* Zero this, just to be sure */
        waiter->monitor = NULL;
        waiter->monitor_depth = 0;

        TraceMonitor(waiter, mid, &mid->monitor_waitq, QUEUE_REMOVED);

        resumeThread(waiter);
    } else {
        /* No one wants this monitor.  Mark it as unused */
        mid->owner = NULL;
        mid->depth = 0;
    }
}


/*=========================================================================
 * FUNCTION:      addCondvarWait
 * TYPE:          Monitor handler
 * OVERVIEW:      Called to indicate a thread is waiting to be notified
 * INTERFACE:
 *   parameters:  mid:    Monitor
 *                thread: Thread waiting to be notified
 *=======================================================================*/


static void
addCondvarWait(MONITOR mid, THREAD thread) {
    if (mid->owner != thread) {
        fatalVMError("Bad call to addConvarWait");
    }
    addThreadToQueue(&mid->condvar_waitq, thread, AT_END);
    thread->monitor = mid;

    TraceMonitor(thread, mid, &mid->condvar_waitq, QUEUE_ADDED);

    /* Save our entry count, so we can restore it when we regain control of
     * the monitor */
    thread->monitor_depth = mid->depth;

    /* We're relinquishing the monitor. Get the next guy off the queue */
    removeMonitorWait(mid);
}


/*=========================================================================
 * FUNCTION:      removeCondvarWait
 * TYPE:          Monitor handler
 * OVERVIEW:      Called to notify one thread waiting to be notified
 * INTERFACE:
 *   parameters:  mid:    Monitor
 *=======================================================================*/

static void
removeCondvarWait(MONITOR mid, bool_t notifyAll) {
    do {
        THREAD waiter = removeQueueStart(&mid->condvar_waitq);
        if (waiter == NULL) {
            break;
        }
        /* This thread now has to wait to get ownership of the monitor */
        TraceMonitor(waiter, mid, &mid->condvar_waitq, FALSE);
        /* Just in case we were set to get an alarm */
        removePendingAlarm(waiter);
        /* We can run as soon as we get the monitor again */
        addMonitorWait(mid, waiter);
    } /* Once through if !notifyAll.  Until waiter == NULL, otherwise */
    while (notifyAll);
}

/*=========================================================================
 * FUNCTION:      monitorEnter
 * TYPE:          Monitor handler
 * OVERVIEW:      Try to grab the monitor of an object
 * INTERFACE:
 *   parameters:  object: Any Java object
 *   returns:         MonitorStatusOwn:       own the monitor
 *                    MonitorStatusWaiting:   waiting for the monitor
 *=======================================================================*/

enum MonitorStatusType
monitorEnter(OBJECT object)
{
    THREAD self = CurrentThread;
    MONITOR mid = object->monitor;

    if (mid == NULL) {
        /* Object doesn't yet have a monitor.  Create one */

        START_TEMPORARY_ROOTS
            MAKE_TEMPORARY_ROOT(object);
            mid = (MONITOR)callocObject(SIZEOF_MONITOR, GCT_MONITOR);
            mid->object = object;

            setObjectMonitor(object, mid);

            if (TRACEMONITORS) {
                MAKE_TEMPORARY_ROOT(mid);
                fprintf(stdout,
                        "Monitor %lx created for object %s::%lx\n",
                        (long)mid, className(object->ofClass), (long)object);
            }
        END_TEMPORARY_ROOTS
    }

    if (TRACEMONITORS) {
        START_TEMPORARY_ROOTS
            MAKE_TEMPORARY_ROOT(mid);
            /* This next line is a bad hack.  It prevents the monitor from
             * GC'ing itself because it sees it's unused */
            mid->condvar_waitq = (THREAD)-1;
            fprintf(stdout, "%lx: attempted to gain monitor %s::%lx, owner=%lx depth = %ld\n",
                    (long)self, className(object->ofClass), (long)object,
                    (long)mid->owner, (long)mid->depth);
            mid->condvar_waitq = NULL;
        END_TEMPORARY_ROOTS
    }

    if (mid->owner == NULL) {
        /* The monitor is unowned.  Make ourselves be the owner */
        mid->owner = self;
        mid->depth = 1;
        return MonitorStatusOwn;
    } else if (mid->owner == self) {
        /* We already own the monitor.  Just increase the entry count. */
        mid->depth++;
        return MonitorStatusOwn;
    } else {
        /* Add ourselves to the wait queue.  Indicate that when we are
         * woken, the monitor's depth should be set to 1 */
        self->monitor_depth = 1;
        addMonitorWait(mid, self);
        suspendThread();
        return MonitorStatusWaiting;
    }
}


/*=========================================================================
 * FUNCTION:      monitorExit
 * TYPE:          Monitor handler
 * OVERVIEW:      Relinquish ownership of a monitor
 * INTERFACE:
 *   parameters:  object: Any Java object
 *   returns:         MonitorStatusOwnMonitor:     still own the monitor
 *                    MonitorStatusReleaseMonitor: releasing the monitor
 *                    MonitorStatusError:       some exception occurred
 *=======================================================================*/

enum MonitorStatusType
monitorExit(OBJECT object)
{
    THREAD self = CurrentThread;
    MONITOR mid = object->monitor;

    if (mid == NULL || mid->owner != self) {
        raiseException(IllegalMonitorStateException);
        return MonitorStatusError;
    }

    if (TRACEMONITORS) {
        START_TEMPORARY_ROOTS
            fprintf(stdout,
                    "%lx: returning monitor %s::%lx (depth = %d)\n",
                    (long)self, className(object->ofClass),
                    (long)object, mid->depth);
        END_TEMPORARY_ROOTS
    }

    if (--mid->depth == 0) {
        /* Let someone else have the monitor */
        removeMonitorWait(mid);
        return MonitorStatusRelease;
    } else {
        return MonitorStatusOwn;
    }
}


/*=========================================================================
 * FUNCTION:      monitorWait()
 * TYPE:          Monitor handler
 * OVERVIEW:      Wait for notification on the object
 * INTERFACE:
 *   parameters:  object: Any Java object
 *   returns:         MonitorStatusWaiting:        waiting on the condvar
 *                    MonitorStatusError:       some exception occurred
 *=======================================================================*/


enum MonitorStatusType
monitorWait(OBJECT object, long64 delta)
{
    static void monitorWaitAlarm(THREAD thread);
    THREAD self = CurrentThread;
    MONITOR mid = object->monitor;

    if (TRACEMONITORS) {
        START_TEMPORARY_ROOTS
            fprintf(stdout, "%lx: waiting on condvar %s::%lx\n",
                    (long)self, className(object->ofClass), (long)object);
        END_TEMPORARY_ROOTS
    }

    if (mid == NULL || mid->owner != self) {
        raiseException(IllegalMonitorStateException);
        return MonitorStatusError;
    }

    if (ll_zero_gt(delta)) {
        registerAlarm(self, delta, monitorWaitAlarm);
    }
    addCondvarWait(mid, self);
    suspendThread();
    return MonitorStatusWaiting;
}

/* Callback function if a thread's timer expires while waiting to be notified */

static void monitorWaitAlarm(THREAD thread)
{
    MONITOR mid = thread->monitor;
    if (mid != NULL) {
        if (removeFromQueue(&mid->condvar_waitq, thread)) {
            addMonitorWait(mid, thread);
        } else {
            fatalError("Thread not on condvar queue");
        }
    }
}

/*=========================================================================
 * FUNCTION:      monitorNotify()
 * TYPE:          Monitor handler
 * OVERVIEW:      Notify one process
 * INTERFACE:
 *   parameters:  object: Any Java object
 *                notifyAll:  true if waking up all waiters.
 *   returns:         MonitorStatusOwnMonitor:  normal
 *                    MonitorStatusError:       some exception occurred
 *=======================================================================*/

enum MonitorStatusType
monitorNotify(OBJECT object, bool_t notifyAll) {
    THREAD self = CurrentThread;
    MONITOR mid = object->monitor;

    if (mid == NULL || mid->owner != self) {
        raiseException(IllegalMonitorStateException);
        return MonitorStatusError;
    }

    if (TRACEMONITORS) {
        START_TEMPORARY_ROOTS
            fprintf(stdout, "%lx: %s monitor %s::%lx\n",
                    (long)self,
                    (notifyAll ? "notifying" : "notifying all"),
                    className(object->ofClass), (long)object);
        END_TEMPORARY_ROOTS
    }
    removeCondvarWait(mid, notifyAll);
    return MonitorStatusOwn;
}

/*=========================================================================
 *
 * Queue manipulation functions

 *=======================================================================*/


/*=========================================================================
 * FUNCTION:      addThreadToQueue()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Add a thread to the indicated queue
 * INTERFACE:
 *   parameters:  queue:      A pointer to the queue
                  thisThread: thread to add to the queue
                  where:      AT_START or AT_END
 *   returns:     <nothing>
 * NOTE:          There are no safety checks here, so please
 *                do not suspend the same thread twice.
 *=======================================================================*/

static void
addThreadToQueue(THREAD *queue, THREAD thisThread, queueWhere where)
{
    START_CRITICAL_SECTION
        if (*queue == NIL) {
            *queue = thisThread;
            thisThread->nextThread = thisThread;
        } else {
            /* Add thisThread after *queue, which always points
             * to the last element of the list */
            thisThread->nextThread = (*queue)->nextThread;
            (*queue)->nextThread = thisThread;
            if (where == AT_START) {
                /* We are already the first element of the queue */
                ;
            } else {
                /* make this thread be the last thread */
                *queue = thisThread;
            }
        }
    END_CRITICAL_SECTION
}


/*=========================================================================
 * FUNCTION:      removeQueueStart()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Remove a thread by from start of the indicated queue
 * INTERFACE:
 *   parameters:  queue:  A pointer to the queue
 *   returns:     A THREAD pointer or NIL
 *=======================================================================*/

static THREAD removeQueueStart(THREAD *queue)
{
    THREAD thisThread;

    START_CRITICAL_SECTION
        if (*queue == NULL) {
            thisThread = NULL;
        } else {
            /* (*queue) points to the last item on a circular list */
            thisThread = (*queue)->nextThread;
            if (thisThread == *queue) {
                /* We were the only item on the list */
                *queue = NULL;
            } else {
                /* Remove us from the front of the queue */
                (*queue)->nextThread = thisThread->nextThread;
            }
            thisThread->nextThread = NIL;
        }
    END_CRITICAL_SECTION

    return thisThread;
}

/*=========================================================================
 * FUNCTION:      removeFromQueue()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Remove a thread from the queue
 * INTERFACE:
 *   parameters:  queue:  A pointer to the queue
 *                waiter: The item to remove
 *   returns:     TRUE if the item was found on the queue, FALSE otherwise
 *=======================================================================*/

static bool_t
removeFromQueue(THREAD *queue_p, THREAD waiter)
{
    THREAD queue = *queue_p;
    if (queue == NULL) {
        return FALSE;
    } else {
        THREAD prev_q = queue;
        THREAD q = queue->nextThread;

        while (q != queue && q != waiter) {
            /* We start at the first item on the queue, and circle until
             * we reach the last item */
            prev_q = q;
            q = q->nextThread;
        }

        /* Note, q == queue is ambiguous.  Either we were searching for the
         * last item on the list, or the item can't be found */

        if (q != waiter) {
            return FALSE;
        } else {
            prev_q->nextThread = q->nextThread;
            q->nextThread = NULL;
            if (q == queue) {
                /* We were deleting the final item on the list */
                *queue_p = (prev_q == q)
                    ? NULL  /* Deleted last item on the list */
                    : prev_q; /* Still more items on the list */
            }
            return TRUE;
        }
    }
}

bool_t inMonitorQueue(THREAD thread) {
    return thread->monitor != NULL;
}

#if TRACETHREADING
static void
TraceThread(THREAD thread, char* what) {
    fprintf(stdout, "%lx: %s\n", (long)thread, what);
}
#endif




#if TRACEMONITORS
static void
TraceMonitor(THREAD thread, MONITOR mid, THREAD *queuep, bool_t added) {
    int count = 0;
    OBJECT object = mid->object;
    THREAD queue = *queuep;
    if (queue != NULL) {
        THREAD q = queue;
        do {
            q = q->nextThread;
            count++;
        } while (q != queue);
    }

    START_TEMPORARY_ROOTS
        fprintf(stdout,
                "%lx:  %s %s queue %s::%lx (queue length now %d)\n",
                (long)thread,
                added ? "added to" : "removed from",
                queuep == &mid->monitor_waitq ? "monitor" : "condvar",
                className(object->ofClass), (long)mid->object,
                count);
    END_TEMPORARY_ROOTS
}
#endif
