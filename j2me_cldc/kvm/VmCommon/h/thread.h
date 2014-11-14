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
 * FILE:      thread.h
 * OVERVIEW:  This file defines the structures that are needed
 *            for Java-style preemptive multitasking.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (nano libraries, new monitors)
 *            Edited by Nik Shaylor 09/1999 to allow asynchronous I/O
 *=======================================================================*/

/*=========================================================================
 * ORIGINAL COMMENTS
 *
 * Our VM has a machine-independent, portable, pre-emptive
 * threading model that is capable of operating independently
 * of the Java language.
 *
 * A simple round-robin style scheduling model is used, in which
 * all the active threads in the system are stored in a circular
 * linked list.
 *
 * Threads in the list are given execution time one after each other
 * based on the Java-level priority of each task.  At the implementation
 * level thread priority is simply an integer that tells to the
 * interpreter how many primitives the thread may execute until the
 * next thread switch takes place. After each bytecode execution,
 * the "timeslice" counter of the thread is decremented.  When
 * timeslice becomes zero, a thread switch is forced. Some I/O
 * primitives may also initiate a thread switch.
 *
 * Threads in our VM can be in three logical states:
 *
 * SUSPENDED = the thread is not in the active thread list and
 * it does not receive any execution time.
 *
 * RUNNABLE = thread is in the active thread list but it isn't
 * currently active (it will become active as soon as the
 * other threads have consumed their timeslice.
 *
 * ACTIVE = thread is currently being executed and given processor
 * time (up = thisThread). Execution will continue until timeslice
 * becomes zero or the thread voluntarily gives execution turn to
 * another thread.
 *
 *=========================================================================
 *
 * There are certain situations in which thread switching cannot be allowed.
 * For instance, allowing thread switching while executing interpreted
 * code called from a native function could mess up the C/C++ call stack
 * very badly.
 *
 *=======================================================================*/

/*=========================================================================
 * NEW COMMENTS 09/1999
 * ====================
 *
 * The thread model described above has been changed a little to allow
 * asynchronous I/O to be supported by the VM.
 *
 * One intention is to introduce the ability to easily re-program KVM to
 * have different scheduling mechanisms, but I did not want to introduce
 * too radical a change to the current system so it still works in a similar
 * way as before.
 *
 * In the original system context switching occurred in a number of
 * places. Now it is only ever done at the top of the interpreter dispatch
 * loop. In the original, runnable threads were kept in a circular list with
 * the currently executing one. Now each thread is removed from a
 * circular list of runnable threads when it starts executing and is returned
 * there when it is blocked. 
 *
 * The variable UP has been renamed CurrentThread and points to the
 * currently executing thread.  A new variable RunnableThreads points to the
 * circular list of runnable threads.
 * NOTE: RunnableThreads points to the >>last<< item on the circular queue.
 * This makes it easy to add items to either the front or back of the queue.
 *
 * Important routine changes:
 *
 * suspendThread()
 * ---------------
 *
 * This used to have a parameter of which thread to suspend, but there is no
 * case in which it cannot be UP so this parameter has been removed. This
 * routine used to perform a context switch, but now it just zeros
 * CurrentThread and calls a new function called signalTimeToReschedule()
 * which signals to the interpreter that it should reschedule the VM before
 * the next bytecode is executed. 
 *  
 * A new function * isTimeToReshedule() is provided for the interpreter to
 * test this condition, and a new function reschedule() is called to do
 * this. The code at the top of the interpreter bytecode despatch loop is now:
 *
 *    if(isTimeToReshedule()) {
 *        reschedule();
 *    }
 *
 * Because of the time critical nature of this part of the VM it is intended
 * that these two functions are implemented as macros, and the current
 * version of isTimeToReshedule() expands to "Timeslice-- == 0" which is
 * exactly what the old VM * code was. The function signalTimeToReschedule()
 * simply zeros Timeslice. 
 *
 * startThread()
 * -------------
 *
 * This makes a thread alive, but suspended.
 *
 * resumeThread()
 * ---------------
 *
 * This function places a thread back on the runnable thread list
 * (RunnableThreads) and will also call signalTimeToReschedule() if the
 * priority of the activated thread is higher than the currently executing one.
 * The new thread is put in the RunnableThreads list in a position where
 * it will be the next one to be executed. 
 * All this is a little half-hearded, but it does * allow high priority
 * threads to get switch to immediately in an interrupt driven environment. 
 *
 * The intention is to a more normal event driven priority scheduling
 * model in due course which was the primary reason behind removing the
 * active thread from the runnable queue when it is executing. We can now
 * completely change the format of the runnable queue without effecting the
 * rest of the system. 
 *
 * BuildThread()
 * -------------
 *
 * This now adds all new threads to another list of all alive threads called
 * AllThreads. This list is used by the garbage collector.
 *
 * DismantleThread()
 * -----------------
 *
 * Removes the thread from the AllThreads list.
 *
 * stopThread()
 * ------------
 *
 * This function is the logical opposite to buildThread(). It suspends the
 * thread and calls DismantleThread().
 *
 * resumeThread()
 * ---------------
 *
 * This replaces the use of activateThread() to restart a suspended thread.
 *
 * isActivated()
 * -------------
 *
 * It used to be necessary to test that CurrentThread was non-null before
 * calling this routine. 
 *
 * HandleEvent()
 * -------------
 *
 * Because the original HandleEvent called activateThread it did not require
 * changing (except or the calling convention for isActivated()) but the
 * meaning of the return parameter is slightly different in that is indicates
 * that a new thread was added to RunnableThreads and not that 
 * it just switched context. 
 *
 * SwitchThreads()
 * ---------------
 *
 * SwitchThreads is now only called from reschedule(). CurrentThread may, or
 * may not, point to a thread. If it does we rotate the thread through the
 * RunnableThreads list as before. If it is null nd the RunnableThreads list
 *is empty then the function returns FALSE and reschedule() must decide 
 * what to do. On the Palm it does a wait forever for the next event.
 *
 * JLT_Yield()
 * -----------
 *
 * Now only has to call signalTimeToReschedule()
 *
 *
 *
 * Asynchronous I/O
 * ================
 *
 * The basic thread management strategy of KVM is unchanged in that all the
 * Java * code is executed by the same native thread, and all Java thread
 * context switching * is done internally using the original 'green thread'
 * concept. 
 *
 * Asynchronous I/O is an optional KVM feature that requires some
 * kind of operating system support be that, interrupts,
 * callback routines, or native threads. 
 * 
 * When the author of a Java native function wants to allow the interpreter
 * to execute while the I/O is being performed they should call
 * suspendThread(), then setup the appropriate platform dependent mechanism
 * for being notified of the I/O completion, and then return. >>> It is most
 * important that the call to suspendThread() occurs before the callback
 * routine can run.<<<
 *
 * When the callback routine executes it will almost always want to push some
 * result on the suspended threads stack and then it calls resumeThread().
 *
 * When the native function is called it is running in the context of the
 * calling thread. This is not the case when the callback routine (or whatever
 * it is) is called. Parameters to the native function can therefore be
 * popped from the stack using the normal routines (popStack() etc.), but
 * when the callback routine is called special new versions of these 
 * stack manipulation functions must be used when the context is explicitly
 * supplied. 
 * These new routines all have "FromThread" appended to the end and an extra
 * parameter so that the equivalent of popStack() becomes
 * popStackFromThread(THREAD *) etc. Using these new functions (macros
 * actually) the thread's stack can be setup with the appropriate parameters
 * prior to calling activateThread().
 *
 * To prevent a race condition corrupting the RunnableThreads queue which
 * links asynchronous world outside the VM with its internal rescheduling
 * functions all accesses to this queue much be interlocked with a critical
 * section. Therefore two new platform dependent functions must be defined 
 *
 *     enterSystemCriticalSection()
 *
 * and
 *
 *     exitSystemCriticalSection()
 *
 * Asynchronous Class loading
 * --------------------------
 *
 * There currently is no support for this. The way the current KVM class
 * loading is done coupled with the symantics of the Java platform means that
 * there is a potentially endless level of nesting where a class is loaded
 * which causes the class initializer to be executed, which causes another
 * class to be loaded, and so on. 
 *
 * When the SwitchThread routine detects that a blocking I/O operation has
 * occured in supervisor mode (such as would be the case if a class
 * initializer read from a file) it will spin waiting for the 
 * I/O to complete. 
 * 
 * This should not effect the way the native functions doing the I/O are
 * written. 
 */

/*=========================================================================
 * Global definitions and variables needed for multitasking
 *=======================================================================*/

extern int AliveThreadCount;    /*  Number of alive threads in the system */
extern int ActiveThreadCount; /*  Number of active threads in the system */

extern int Timeslice;     /*  Time slice counter for multitasking */

extern THREAD AllThreads;     /*  List of all threads */
extern THREAD CurrentThread;  /*  Current thread */

#define MAX_PRIORITY  10  /*  these constants must be the same as those */
#define NORM_PRIORITY 5   /*  defined in Thread.java */
#define MIN_PRIORITY  1

/*=========================================================================
 * Global multitasking operations
 *=======================================================================*/

void storeExecutionEnvironment(THREAD thisThread);
void loadExecutionEnvironment(THREAD thisThread);
bool_t SwitchThread(void);

/*=========================================================================
 * Thread data structures and operations
 *=======================================================================*/

/*=========================================================================
 * Thread instance data structures:
 *
 * One of the structures below is instantiated
 * for every thread (task) in the system:
 *
 * - THREAD is an internal (VM-level) structure that is used for storing
 *   the necessary information for implementing preemptive task
 *   switching independently of Java-level stuff. Active THREADs 
 *   are singly-linked to a linear list that contains all the 
 *   active threads in the system.
 *
 * - JAVATHREAD is mapped one to one to the instance structure 
 *   defined in Java class 'Thread.java'. It is referred to from
 *   THREAD. 
 *
 * Why do we need two separate structures? Well, since class 'Thread.java' 
 * may be subclassed, we cannot add the necessary implementation-level 
 * fields directly to JAVATHREAD, but we must use a separate struct instead!
 * Also, we wanted to keep the thread system implementation independent
 * of the Java language for portability reasons.
 *
 * Note that a JAVATHREAD must be a full-fledged Java object instance.
 * Thus its beginning must be identical with that of INSTANCE. 
 *=======================================================================*/

/*  THREAD */
struct threadQueue {
    THREAD nextAliveThread;  /*  Queue of threads that are alive; */
    THREAD nextThread;       /*  Queue of runnable or waiting threads */
    JAVATHREAD javaThread;   /*  Contains Java-level thread information */
    long   timeslice;        /*  Calculated from Java-level thread priority */
    STACK stack;             /*  The execution stack of the thread */

    /*  The following four variables are used for storing the */
    /*  virtual machine registers of the thread while the thread */
    /*  is runnable (= active but not currently given processor time). */
    /*  In order to facilitate garbage collection, we store pointers */
    /*  to execution stacks as offsets instead of real pointers. */
    BYTE* ipStore;           /*  Program counter temporary storage (pointer) */
    FRAME fpStore;           /*  Frame pointer temporary storage (pointer) */
    cell* spStore;           /*  Execution stack pointer temp storage (pointer) */
    
    MONITOR monitor;         /*  Monitor whose queue this thread is  on */
    short monitor_depth;
    

    THREAD nextAlarmThread;  /* The next thread on this queue */
    long   wakeupTime[2];    /* We can't demand 8-byte alignment of heap
				    objects  */
    void (*wakeupCall)(THREAD);	/* Callback when thread's alarm goes off */

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
    char *pendingException;  /* Name of class for which the thread has an exception pending */
#endif

    enum { 
	THREAD_JUST_BORN = 0,	/* Not "start"ed yet */
	THREAD_ACTIVE = 1,	/* Currently running, or on Run queue */
	THREAD_SUSPENDED = 2,   /* Waiting for monitor or alarm */
	THREAD_DEAD = 3		/* Thread has quit */
    } state;
};

#define SIZEOF_THREAD            StructSizeInCells(threadQueue)


/*  JAVATHREAD */
/*  This structure is mapped one-to-one with Java thread instances */
/*  The fields should be the same as in 'java.lang.Thread' */
struct javaThreadStruct {    /*  (A true Java object instance) */
    INSTANCE_CLASS ofClass;        /*  Pointer to class 'java.lang.Thread' */
    MONITOR  monitor;        /*  The possible monitor */
    long      priority;       /*  Current priority of the thread */
    THREAD   VMthread;       /*  Backpointer to internal THREAD. */
    INSTANCE target;         /*  Pointer to the object whose 'run' */
    long      stackSize;      /*  An application specified stack size */
};

#define SIZEOF_JAVATHREAD        StructSizeInCells(javaThreadStruct)


/*=========================================================================
* Operations on thread instances
 *=======================================================================*/

/*  Constructors and destructors */
THREAD BuildThread(void);	
void   InitializeThreading(void);
void   InitializeJavaRuntime(void);
THREAD getVMthread(JAVATHREAD);

/*  Thread activation and deactivation */
void   initThreadBehavior(THREAD, METHOD, OBJECT);
void   startThread(THREAD);
void   stopThread(void);
void   suspendThread(void);
void   resumeThread(THREAD);
int    isActivated(THREAD);

#if ASYNCHRONOUS_NATIVE_FUNCTIONS
void RundownAsynchronousFunctions(void);
void asyncFunctionProlog(void);
void asyncFunctionEpilog(THREAD);
#else
#define RundownAsynchronousFunctions()
#endif


#if ASYNCHRONOUS_NATIVE_FUNCTIONS
void enterSystemCriticalSection(void);
void exitSystemCriticalSection(void);
#define START_CRITICAL_SECTION { enterSystemCriticalSection();
#define END_CRITICAL_SECTION   exitSystemCriticalSection(); }
#else
#define START_CRITICAL_SECTION { 
#define END_CRITICAL_SECTION   }
#endif





/*  Thread debugging */
#if INCLUDEDEBUGCODE
void   printFullThreadStatus(void);
#else 
#  define printFullThreadStatus()
#endif


/*=========================================================================
 * Clock data structures and operations
 *=======================================================================*/

void registerAlarm(THREAD thread, long64 delta, void (*)(THREAD));
void checkTimerQueue(ulong64 *nextTimerWakeup);
bool_t inTimerQueue(THREAD thread);

/*=========================================================================
 * Monitor data structures and operations
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * Our VM has a general, machine-independent monitor implementation
 * that is used when executing synchronized method calls and 
 * MONITORENTER/MONITOREXIT bytecodes.
 *
 * Monitors are always attached to individual object instances
 * (or class instances when executing synchronized static method 
 * calls). Each monitor structure holds a waiting list in which 
 * those threads who are waiting for the monitor to be released
 * are stored. A democratic FCFS (first-come-first-serve) scheduling 
 * model is used for controlling waiting lists. Threads are 
 * automatically suspended while they wait in a monitor.
 *
 * NOTE: this version of the VM includes a new, faster & smaller,
 * monitor implementation.  Rather than creating explicit wait
 * list objects, the threads that are waiting in monitors are
 * chained simply using their 'nextThread' field. The 'firstWaiter'
 * and 'lastWaiter' fields of the monitor structure are used for
 * pointing to the beginning and end of the wait list, allowing 
 * rapid insertions and removals to the list.
 *=======================================================================*/

/*=========================================================================
 * Monitor data structure
 *=======================================================================*/

/*  MONITOR (allocated in stack frames) */
struct monitorStruct {
    THREAD owner;        /*  Current owner of the monitor */
    THREAD monitor_waitq;
    THREAD condvar_waitq;
    OBJECT object;	  /*  The object we are a monitor for */
    short   depth;        /*  Nesting depth: how many times the */
};

#define SIZEOF_MONITOR           StructSizeInCells(monitorStruct)

/*=========================================================================
 * Operations on monitor instances
 *=======================================================================*/

/* These are the possible return values from monitorEnter, monitorExit, 
 * monitorWait, and monitorNotify. 
 */
enum MonitorStatusType { 
    MonitorStatusOwn,		/* The process owns the monitor */
    MonitorStatusRelease,	/* The process has released the monitor */
    MonitorStatusWaiting,	/* The process is waiting for monitor/condvar */
    MonitorStatusError		/* An error has occurred */
};

enum MonitorStatusType  monitorEnter(OBJECT object);
enum MonitorStatusType  monitorExit(OBJECT object);
enum MonitorStatusType  monitorWait(OBJECT object, long64 time);
enum MonitorStatusType  monitorNotify(OBJECT object, bool_t wakeAll);

bool_t inMonitorQueue(THREAD thread);

/*  Monitor debugging */
int countWaiters(MONITOR monitor);

void InitializeJavaThread(THREAD thread);

/*  STACK */
struct stackStruct {
    STACK next;
    short size;
    short xxunusedxx;		/* must be multiple of 4 on all platforms */
  cell cells[STACKCHUNKSIZE];
};

#define STACK_CONTAINS(stack, p) \
  ((p) >= (stack)->cells && (p) < (stack)->cells + (stack)->size)

#ifndef NATIVE_FUNCTION_COMPLETED
#define NATIVE_FUNCTION_COMPLETED()  /* do nothing by default */
#endif
