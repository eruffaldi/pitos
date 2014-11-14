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
 * SUBSYSTEM: KVM
 * FILE:      async.h
 * OVERVIEW:  Definitions to support (and not support) asynchronous I/O
 * AUTHOR:    Nik Shaylor, Sun C&E
 *=======================================================================*/

#define ASYNCFUNCTION(x)    ASYNC_FUNCTION(x)
#define ASYNCPOPSTACK       ASYNC_POP_STACK
#define ASYNCPUSHSTACK      ASYNC_PUSH_STACK
#define ASYNCPOPLONG        ASYNC_POP_LONG
#define ASYNCPUSHLONG       ASYNC_PUSH_LONG
#define ASYNCEXCEPTION      ASYNC_EXCEPTION
#define ASYNCRESUMETHREAD() ASYNC_RESUME_THREAD()

#if !ASYNCHRONOUS_NATIVE_FUNCTIONS

/* The following is just a very strange way of getting a secret local
   variable to be placed on the (non) async function's stack frame */

#define ASYNC_FUNCTION(x)                                               \
void x(void) {                                                          \
    char *nonAsyncException = 0;                                        \
    static void internal ## x (char *);                                 \
    internal ## x (nonAsyncException);                                  \
}                                                                       \
static void internal ## x (char *nonAsyncException)


#define ASYNC_POP_STACK           popStack
#define ASYNC_PUSH_STACK          pushStack
#define ASYNC_POP_STACK_AS_TYPE   popStackAsType
#define ASYNC_PUSH_STACK_AS_TYPE  pushStackAsType
#define ASYNC_POP_LONG            popLong
#define ASYNC_PUSH_LONG           pushLong
#define ASYNC_EXCEPTION(x)        (nonAsyncException = (x))
#define ASYNC_RESUME_THREAD()     if(nonAsyncException != 0) raiseException(nonAsyncException)
#define ASYNC_ONE_LESS            oneLess;

#define startLongIOActivity() /**/
#define endLongIOActivity()   /**/

#else

#define ASYNC_FUNCTION(x)                                               \
void x(void) {                                                          \
    THREAD thisThread = CurrentThread;                                  \
        static void internal ## x (THREAD);                             \
        asyncFunctionProlog();                                          \
        while(garbageCollecting()){Yield_md();}                         \
        CallAsyncNativeFunction_md(thisThread, internal ## x);          \
}                                                                       \
static void internal ## x (THREAD asyncThisThread)


#define ASYNC_POP_STACK()                     popStackForThread(asyncThisThread)
#define ASYNC_PUSH_STACK(x)                   pushStackForThread(asyncThisThread,x)
#define ASYNC_POP_STACK_AS_TYPE(_type_)       popStackAsTypeForThread(asyncThisThread, _type_)
#define ASYNC_PUSH_STACK_AS_TYPE(_type_, x)   pushStackAsTypeForThread(asyncThisThread,_type_,x)
#define ASYNC_POP_LONG(x)                     popLongForThread(asyncThisThread,x)
#define ASYNC_PUSH_LONG(x)                    pushLongForThread(asyncThisThread,x)
#define ASYNC_ONE_LESS                        oneLessForThread(asyncThisThread)
#define ASYNC_EXCEPTION(x)                    (asyncThisThread->pendingException = (x))
#define ASYNC_RESUME_THREAD()                 asyncFunctionEpilog(asyncThisThread);

#define sp ERROR_USING_sp
#define ip ERROR_USING_ip
#define fp ERROR_USING_fp

int garbageCollecting(void);

extern int AsyncThreadCount;
#define startLongIOActivity() (--AsyncThreadCount)
#define endLongIOActivity() { ++AsyncThreadCount; while(garbageCollecting()){Yield_md();} }

#endif

/*
 * New style async macros.
 */

#define ASYNC_FUNCTION_START(x)  ASYNC_FUNCTION(x) {
#define ASYNC_FUNCTION_END       ASYNC_RESUME_THREAD() }
#define ASYNC_popStack           ASYNC_POP_STACK
#define ASYNC_pushStack          ASYNC_PUSH_STACK
#define ASYNC_popStackAsType     ASYNC_POP_STACK_AS_TYPE
#define ASYNC_pushStackAsType    ASYNC_PUSH_STACK_AS_TYPE
#define ASYNC_popLong            ASYNC_POP_LONG
#define ASYNC_pushLong           ASYNC_PUSH_LONG
#define ASYNC_raiseException     ASYNC_EXCEPTION


#ifndef NATIVE_FUNCTION_COMPLETED
#define NATIVE_FUNCTION_COMPLETED()  /* do nothing by default */
#endif


