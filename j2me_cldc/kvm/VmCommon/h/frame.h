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
 * SUBSYSTEM: Interpreter stack frames
 * FILE:      frame.h
 * OVERVIEW:  Definitions for execution frame & exception handling 
 *            manipulation.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (exception handling)
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines the VM-specific internal runtime structures for
 * manipulating stack frames and performing the necessary exception
 * handling operations.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

/* Number of cells reserved for native methods. Native method must push
   a new frame when they use more than 3 stack slots. */

#define RESERVEDFORNATIVE 3

/* frame pointer => pointer to locals */
#define FRAMELOCALS(fp) ((cell*)(fp) - fp->thisMethod->frameSize)

/*=========================================================================
 * COMMENT:
 * The overall stack frame organization for individual stack frames
 * is as follows:
 * 1) Method call parameters + local variables
 * 2) The actual frameStruct (see below)
 * 3) Operand stack
 *
 * Note: there is no separate "operand stack" for data manipulation.
 * The necessary operands are always stored on top of the current 
 * execution stack frame.  We sometimes use the term operand stack
 * to refer to the portion of the execution stack allocated for the
 * operands and data of the currently executing method.
 * 
 * Remember that in virtual function calls, the first local variable
 * (zeroeth actually) always contains the self-reference ('this').
 * In static function calls, the first local variable contains the
 * first method parameter (if any).
 *=======================================================================*/

/*  FRAME (allocated inside execution stacks of threads) */
struct frameStruct {
    METHOD   thisMethod; /*  Pointer to the method currently under execution */
    FRAME    previousFp; /*  Stores the previous frame pointer */
    cell*    previousSp; /*  Stores the previous stack pointer */
    BYTE*    previousIp; /*  Stores the previous program counter */
    OBJECT   syncObject; /*  Holds monitor object if synchronized method call */
	STACK    stack;      /*  Stack chunk containing the frame */
};

/*  HANDLER */
struct exceptionHandlerStruct {
    /*  Note: in the current JVM Specification, all these values */
    /*  are actually shorts (16 bit values), limiting the maximum */
    /*  Java method size to 64k. This is a silly limitation that will */
    /*  probably necessitate changes in the classfile format later on */
    unsigned short startPC;   /*  Start and end program counter indices; these */
    unsigned short endPC;     /*  determine the code range where the handler is valid */
    unsigned short handlerPC; /*  Location that is called upon exception */
    unsigned short exception; /*  Class of the exception (as a constant pool index) */
	                          /*  Note: 0 in 'exception' indicates an 'any' handler */
};

struct exceptionHandlerTableStruct { 
	long length;
	struct exceptionHandlerStruct handlers[1];
};

#define FOR_EACH_HANDLER(__var__, handlerTable) {                            \
     HANDLER __first_handler__ = handlerTable->handlers;                       \
     HANDLER __end_handler__ = __first_handler__ + handlerTable->length;       \
     HANDLER __var__;                                                       \
     for (__var__ = __first_handler__; __var__ < __end_handler__; __var__++) {

#define END_FOR_EACH_HANDLER } }

/*=========================================================================
 * Sizes for the above structures
 *=======================================================================*/

#define SIZEOF_FRAME             StructSizeInCells(frameStruct)
#define SIZEOF_HANDLER           StructSizeInCells(exceptionHandlerStruct)
#define SIZEOF_HANDLERTABLE(n) \
   (StructSizeInCells(exceptionHandlerTableStruct) + (n - 1) * SIZEOF_HANDLER)


/*=========================================================================
 * Stack frame execution modes (special return addresses)
 *=======================================================================*/

#define KILLTHREAD          ((BYTE*) 1)

/*=========================================================================
 * Operations on stack frames
 *=======================================================================*/

bool_t pushFrame(METHOD thisMethod);
void popFrame(void);

/*=========================================================================
 * Operations on exception handler tables
 *=======================================================================*/

HANDLERTABLE  createExceptionHandlerTable(int numberOfEntries);

void setExceptionHandler(HANDLER handler, 
			 unsigned short startPC,
			 unsigned short endPC,
			 unsigned short handlerPC,
			 unsigned short exception);


#if INCLUDEDEBUGCODE
void printStackTrace(void);
void printExceptionHandlerTable(HANDLERTABLE handlerTable);
void printExecutionStack(void);
#else 
#  define printStackTrace()
#  define printExceptionHandlerTable(handlerTable)
#  define printExecutionStack()
#endif 

/*=========================================================================
 * Actual exception handling operations
 *=======================================================================*/

void throwException(INSTANCE exception);

/*=========================================================================
 * Operations for raising exceptions and errors from within the VM
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * There are currently three ways by which the VM can report exceptions
 * and errors it has found during execution:
 *
 * - raiseException is the most elegant one, allowing the VM to report
 * errors via the normal Java exception handling mechanism.  It should
 * be used for reporting only those exceptions and errors which are
 * known to be "safe" and "harmless" from the VM's point of view
 * (i.e., the internal consistency and integrity of the VM should
 * not be endangered in any way).
 *
 * - fatalError is similar to raiseException, except that program 
 * execution is terminated immediately. This function is used for
 * reporting those Java-level errors which are not caused by the VM,
 * but which might be harmful for the consistency of the VM.
 * There are currently quite a lot of fatalError calls inside the VM, 
 * but in the long term it is expected that most of these could 
 * be replaced with raiseException calls.
 *
 * - fatalVMError calls are used for reporting internal (in)consistency
 * problems or other VM-related errors. These errors typically indicate 
 * that there are bugs in the VM.
 *
 * MODIFICATION: The exception handling mechanism of the VM has been altered
 *     for the version that will run on PalmOS. The catchable exceptions will
 *     thrown by the VM have been narrowed down to:
 *
 *         NullPointerException
 *         IndexOutOfBoundsException
 *         
 *     The remainder of the exceptions and errors thrown by the VM are not
 *     catchable. Instead, they display a message which includes the point
 *     where the exception occurred. The point of execution is in
 *     the form <classname>::<methodname>@<bytecode_address>. The bytecode
 *     address is relative to the first bytecode of the method (i.e. the same
 *     as given by javap -c).
 *
 *     One other thing to note is that any uncaught *catchable* exception
 *     (thrown either by the application or the VM) causes the VM to exit
 *     as there is no ThreadGroup class to handle uncaught exceptions. 
 *=======================================================================*/

void raiseException(const char* exceptionClassName);
void fatalError(const char* errorMessage);
void fatalVMError(const char* errorMessage);
void alertUser(const char* message); 

/*=========================================================================
 * Shortcuts to the errors/exceptions that the VM may throw
 *=======================================================================*/
extern const char* NullPointerException;
extern const char* IndexOutOfBoundsException;
extern const char* ArrayIndexOutOfBoundsException;
extern const char* ArrayStoreException;
extern const char* ArithmeticException; 
extern const char* ClassCastException; 
extern const char* NegativeArraySizeException;

/*=========================================================================
 * Most frequently needed error message strings
 *=======================================================================*/

/* We call this function when we are doing excessive method tracing.  */

#if TRACEMETHODCALLS >= 2
     void frameTracing(METHOD method, char *glyph, int offset);
#else 
#    define frameTracing(method, glyph, offset)
#endif
