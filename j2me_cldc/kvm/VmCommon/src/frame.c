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
 * FILE:      frame.c
 * OVERVIEW:  This file defines the internal operations for 
 *            manipulating stack frames & performing exception
 *            handling.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (simplified exception handling)
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
        
/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

/*=========================================================================
 * Operations on stack frames
 *=======================================================================*/

#if TRACEMETHODCALLS >= 2


/*=========================================================================
 * FUNCTION:      frameDepth();
 * OVERVIEW:      Returns the current stack depth of the Java stack.
 * INTERFACE:
 *   parameters:  none
 *   returns:     depth
 *
 * COMMENTS:      Do not call this function is fp == 0.
 *=======================================================================*/

static int
frameDepth() { 
    int depth = 0;
    FRAME thisFP = fp;
    while (thisFP->previousIp != KILLTHREAD) { 
	depth++;
	thisFP = thisFP->previousFp;
    }
    return depth;
}

/*=========================================================================
 * FUNCTION:      frameTracing();
 * OVERVIEW:      Print a message to stderr indicating that a frame has
 *                been entered or exited.
 * INTERFACE:
 *   parameters:  method:  Method we're calling
 *                glyph:   "=>" or "<="
 *                offset:  Actual depth is frameDepth() + offset
 *=======================================================================*/

void frameTracing(METHOD method, char *glyph, int offset) { 
    START_TEMPORARY_ROOTS
	int depth = frameDepth() + offset;
	char *className = className(method->ofClass);
	char *methodName = methodName(method);
	char *methodSignature = methodSignature(method);
        fprintf(stderr, "%lx: (%ld) ", (long)CurrentThread, (long)depth);
	while (--depth >= 0) { 
	    fprintf(stderr, " | ");
	}
	fprintf(stderr, " %s %s.%s%s\n", glyph, 
		className, methodName, methodSignature);
    END_TEMPORARY_ROOTS
}

/*=========================================================================
 * FUNCTION:      exceptionThrownTracing
 * OVERVIEW:      Print a message indicating that an exception isn't being
 *                caught in the same stack frame that it was called.
 * INTERFACE:
 *   parameters:  exception:  Exception being thrown
 *=======================================================================*/

static void 
exceptionThrownTracing(INSTANCE exception) { 
    START_TEMPORARY_ROOTS
	int depth = frameDepth();
	char *className = className(exception->ofClass);
        fprintf(stderr, "%lx: (%ld) ", (long)CurrentThread, (long)depth);
	while (--depth >= 0) { 
	    fprintf(stderr, " | ");
	}
	fprintf(stderr, " Throwing %s\n", className);
    END_TEMPORARY_ROOTS
}

/*=========================================================================
 * FUNCTION:      exceptionCaughtTracing
 * OVERVIEW:      Print a message indicating that an exception has been
 *                caught in a frame other than the one that threw it.
 * INTERFACE:
 *   parameters:  exception:  Exception being thrown
 *                handler:    Exception handler
 *=======================================================================*/

static void 
exceptionCaughtTracing(INSTANCE exception, HANDLER handler) { 
    START_TEMPORARY_ROOTS
	int depth = frameDepth();
	char *className = className(exception->ofClass);
        fprintf(stderr, "%lx: (%ld) ", (long)CurrentThread, (long)depth);
	while (--depth >= 0) { 
	    fprintf(stderr, " | ");
	}
	fprintf(stderr, " Caught %s%s\n", className, 
		handler->exception == 0 ? " (finally)" : "");
    END_TEMPORARY_ROOTS
}


#else 

#define exceptionThrownTracing(exception)
#define exceptionCaughtTracing(exception, handler)

#endif


/*=========================================================================
 * FUNCTION:      pushFrame()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Creates a new execution stack frame for the currently 
 *                executing thread. The operation is used for invoking 
 *                methods upon message sending to objects.
 * INTERFACE:
 *   parameters:  method pointer
 *   returns:     TRUE if pushFrame succeeds;
 *                FALSE on stack overflow (it will have raised the exception)
 *
 * COMMENTS:      Note: this function operates in the context of 
 *                currently executing thread. All VM registers must
 *                be initialized correctly before allocating frames.
 *
 *                Remember that the zeroeth local variable '*lp'
 *                must contain the 'this' (self) pointer 
 *                in virtual/special/interface function calls.
 *=======================================================================*/


bool_t pushFrame(METHOD thisMethod)
{
    FRAME newFrame;
    int i;
    int thisMethodHeight = thisMethod->localCount + thisMethod->maxStack +
        SIZEOF_FRAME + RESERVEDFORNATIVE;
    STACK stack = fp ? fp->stack : CurrentThread->stack;
    cell *prev_sp = sp - thisMethod->argCount;
    
    if (sp - stack->cells + thisMethodHeight >= stack->size) {
        STACK newstack;
        thisMethodHeight += thisMethod->argCount;
        if (stack->next && thisMethodHeight > stack->next->size) {
            stack->next = NULL;
        }
        if (stack->next == NULL) {
            int size = thisMethodHeight > STACKCHUNKSIZE ? 
                thisMethodHeight : STACKCHUNKSIZE;
            int stacksize = sizeof(struct stackStruct) / CELL +
                (size - STACKCHUNKSIZE);

	    newstack = (STACK)mallocHeapObject(stacksize, GCT_EXECSTACK);
	    if (newstack == NULL) { 
		throwException(StackOverflowObject);
		return FALSE;
	    }
		
	    memset(newstack, 0, stacksize << log2CELL);

            newstack->next = NULL;
            newstack->size = size;
            stack->next = newstack;

            if (TRACE_STACK_CHUNKS) { 
                fprintf(stdout, "thread %lx, new chunk %lx(prev %lx), size %ld\n",
                        (long)CurrentThread, (long)newstack, (long)stack, (long)size);
            }
        } else {
            newstack = stack->next;
        }

        for (i = 0; i < thisMethod->argCount; i++) {
            newstack->cells[i] = prev_sp[i + 1];
        }

        for (i = thisMethod->argCount; i < thisMethod->frameSize; i++) {
            newstack->cells[i] = 0;
        }

        if (TRACE_STACK_CHUNKS) { 
            fprintf(stdout, "thread %lx, copied %d args, (%lx => %lx)\n",
                    (long)CurrentThread, thisMethod->argCount, 
                    (long)(prev_sp + 1), (long)newstack->cells);
        }

        lp = newstack->cells;

        newFrame = (FRAME)(lp + thisMethod->frameSize);
        newFrame->stack = newstack;
    } else {
        /*  Set the local variable pointer to point to the */
        /*  beginning of the local variables in the execution stack */
        lp = prev_sp + 1;

        /*  Add space for local variables; make slots zero so that */
        /*  garbage collector won't get confused. */
        for (i = 0; i < thisMethod->localCount; i++) pushStack(0);

        /*  Initialize the new frame pointer */
        newFrame = (FRAME)(sp + 1);
        newFrame->stack = stack;
    }

    sp = (cell*)(newFrame + 1) - 1;

    /*  Initialize info needed for popping the stack frame later on */
    newFrame->previousSp = prev_sp;
    newFrame->previousIp = ip;
    newFrame->previousFp = fp;

    /*  Initialize the frame to execute the given method */
    newFrame->thisMethod = thisMethod;
    newFrame->syncObject = NIL; /*  Initialized later if necessary */
    
    /*  Change virtual machine registers to execute the new method */
    fp = newFrame;
    ip = thisMethod->u.java.code;
    cp = thisMethod->ofClass->constPool;

    frameTracing(thisMethod, "=>", 0);

    if (TRACE_FRAMES) { 
        fprintf(stdout, "thread %lx, pushFrame (fp=%lx, sp=%lx)\n",
                (long)CurrentThread, (long)fp, (long)sp);
    }
    return TRUE;
}

/*=========================================================================
 * FUNCTION:      popFrame()
 * TYPE:          destructor (kind of)
 * OVERVIEW:      Deletes an execution stack frame and resumes the 
 *                execution of the method that called the currently 
 *                executing method.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     Nothing directly, but field 'previousIp' holds 
 *                information about what to do after the frame has
 *                been popped (whether to continue interpreter execution,
 *                to kill the thread, or to return to C/C++ code).
 *                Also, the field 'syncObject' holds a monitor object
 *                to be released in case this was a synchronized method
 *                call (see the RETURN bytecodes in Interpret.cpp)
 *=======================================================================*/

void popFrame()
{
    /*  Unallocate a stack frame. */
    /*  Restore virtual machine registers to continue  */
    /*  the execution of the previous method. */

    frameTracing(fp->thisMethod, "<=", 0);

    sp = fp->previousSp;    /*  Restore previous stack pointer */
    ip = fp->previousIp;    /*  Restore previous instruction pointer */
    fp = fp->previousFp;    /*  Restore previous frame pointer */
    lp = FRAMELOCALS(fp);   /*  Restore previous locals pointer */
    cp = fp->thisMethod->ofClass->constPool;
    if (TRACE_FRAMES) { 
        fprintf(stdout, "thread %lx, popFrame (fp=%lx, sp=%lx)\n",
                (long)CurrentThread, (long)fp, (long)sp);
    }
}

/*=========================================================================
 * FUNCTION:      printFrame()
 * TYPE:          public instance-level function on stack frames
 * OVERVIEW:      Prints the contents of a specific execution stack 
 *                frame for debugging purposes.
 * INTERFACE:
 *   parameters:  frame pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
static void 
printFrame(FRAME currentFP, unsigned char *currentIP, cell *currentSP)
{
    METHOD thisMethod = currentFP->thisMethod;
    unsigned char *prevIP = currentFP->previousIp;
    FRAME prevFP = currentFP->previousFp;
    char buffer[256];

    int argCount, i;
    cell* pointer;
    
    /* This is ugly, but this gets called during fatal errors, and we
     * may not have any memory to allocate a className buffer, etc() 
     */
    getClassName_inBuffer((CLASS)thisMethod->ofClass, buffer);
    fprintf(stdout, "Method............: %lx '%s.%s (%s)' \n", 
            (long)thisMethod, 
            buffer, methodName(thisMethod),
            ((thisMethod->accessFlags & ACC_STATIC) ? "static" : "virtual"));
    fprintf(stdout, "Frame Pointer.....: %lx\n", (long)currentFP);
    if (currentIP == NULL) { 
        fprintf(stdout, "Bytecode..........: %lx\n",
                (long)thisMethod->u.java.code);
    } else { 
        fprintf(stdout, "Current IP........: %lx = %lx + offset %ld\n", 
                (long)currentIP, 
                (long)thisMethod->u.java.code,
                (long)(currentIP - thisMethod->u.java.code));
    }
    fprintf(stdout, "Previous Frame....: %lx\n", (long)prevFP);
    fprintf(stdout, "Previous IP.......: %lx", (long)prevIP);
    if (prevFP != NULL) { 
        int offset = prevIP - prevFP->thisMethod->u.java.code;
        fprintf(stdout, " (offset %d)", offset);
    } 
    fprintf(stdout, "\n");

    fprintf(stdout,"Exception handlers: %lx\n", 
            (long)thisMethod->u.java.handlers);
    fprintf(stdout,"Frame size........: %d (%d arguments, %d local variables)\n", 
            thisMethod->frameSize, thisMethod->argCount, 
            thisMethod->localCount);

    /*  Print the parameters and local variables */
    argCount = thisMethod->argCount;
    for (i = 0, pointer = FRAMELOCALS(currentFP); pointer < (cell*)currentFP; 
         i++, pointer++) {
        char *format = (i < argCount) ? "Argument[%d].......: %lx\n"
                                      : "Local[%d]..........: %lx\n";
        fprintf(stdout, format, i, *pointer);
    }
    for (i = 1, pointer = (cell*)(currentFP + 1); pointer <= currentSP; 
            pointer++, i++) { 
        fprintf(stdout,"Operand[%d]........: %lx\n", i, *pointer);
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printStackTrace()
 * TYPE:          public debugging operation
 * OVERVIEW:      Prints the contents of all the stack frames
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 * NOTE:          This operation destroys all the stack frames
 *                so VM execution must be terminated after printing.
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printStackTrace()
{
    FRAME          currentFP = fp;
    unsigned char* currentIP = ip;
    cell         * currentSP = sp;

    while (currentFP != NULL) {
        printFrame(currentFP, currentIP, currentSP); 
        fprintf(stdout,"\n");
        if (currentFP->previousIp == KILLTHREAD) { 
            break;
        }
        currentIP = currentFP->previousIp;
        currentSP = currentFP->previousSp;
        currentFP = currentFP->previousFp;
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printExecutionStack()
 * TYPE:          public debugging operation
 * OVERVIEW:      Prints the contents of the whole execution stack
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printExecutionStack()
{
    cell* i;
    STACK stack;
   
    for (i = 0, stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
        if (STACK_CONTAINS(stack, sp)) {
            i += sp - stack->cells + 1;
            break;
        } else {
            i += stack->size;
        }
    }

    fprintf(stdout, "Execution stack contains %ld items: \n", (long)i);
    
    for (stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
        if (STACK_CONTAINS(stack, sp)) {
            for (i = stack->cells; i <= sp; i++) 
                fprintf(stdout,"%lx  \n", *i);
            break;
        } else {
            for (i = stack->cells; i < stack->cells + stack->size; i++) 
                fprintf(stdout,"%lx  \n", *i);
        }
    }
    fprintf(stdout,"\n");
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Operations on exception handler tables
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      createExceptionHandlerTable()
 * TYPE:          constructor
 * OVERVIEW:      Creates a new table for storing exception handlers
 *                for methods. Each method and runtime frame has one
 *                of these structures.
 * INTERFACE:
 *   parameters:  number of field table entries as integer
 *   returns:     pointer to the beginning of the new table
 *=======================================================================*/

HANDLERTABLE createExceptionHandlerTable(int numberOfEntries)
{
    /*  Exception handler table does not contain any  */
    /*  pointers, so it is safe from GC viewpoint */
    int size = SIZEOF_HANDLERTABLE(numberOfEntries);
    HANDLERTABLE handlerTable = 
        (HANDLERTABLE)mallocObject(size, GCT_NOPOINTERS); 
    handlerTable->length = numberOfEntries;
    return handlerTable;
}

/*=========================================================================
 * FUNCTION:      setExceptionHandler()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Set the contents of a specific exception handler.
 * INTERFACE:
 *   parameters:  pointer to an exception handler table, index, many params
 *   returns:     pointer to the changed handler
 *=======================================================================*/

void 
setExceptionHandler(HANDLER thisHandler, 
                    unsigned short startPC,
                    unsigned short endPC,
                    unsigned short handlerPC,
                    unsigned short exception)
{
    thisHandler->startPC   = startPC;
    thisHandler->endPC     = endPC;
    thisHandler->handlerPC = handlerPC;
    thisHandler->exception = exception;
}

/*=========================================================================
 * FUNCTION:      findHandler()
 * TYPE:          exception handler table lookup operation
 * OVERVIEW:      Find a possible handler in the given exception handler
 *                table that catches the given exception in given
 *                code location.
 * INTERFACE:
 *   parameters:  handler table, exception object, instruction pointer offset
 *   returns:     handler or NIL if no matching handler is found
 * NOTE:          Since in our virtual machine the instruction pointer
 *                always points to the following instruction (i.e.,
 *                'ip' is always one more than specified by the JVM Spec),
 *                in our virtual machine the exception handler range
 *                is inclusive ('<=' rather than '<') in the upper 
 *                end of the handler offset range! In the official JVM Spec
 *                the exception handling offset is inclusive ('>=' at the
 *                bottom of the range and exclusive ('<') at the top.
 *                Weird...
 *=======================================================================*/

static HANDLER 
findHandler(HANDLERTABLE handlerTable, INSTANCE exception, 
	    unsigned short ipOffset)
{
    HANDLER result = NULL;
    
    START_TEMPORARY_ROOT(exception)

        FOR_EACH_HANDLER(thisHandler, handlerTable) 
            if (ipOffset >= thisHandler->startPC 
                      /*  Note: must be '<=' in our VM */
                      && ipOffset <= thisHandler->endPC) { 
                if (thisHandler->exception == 0) { 
                    result = thisHandler;
                    break;
                } else { 
                    CLASS handlerClass = 
                        resolveClassReference(cp, 
                                      (unsigned short)thisHandler->exception,
					      getCurrentClass());
                    if (isInstanceOf((OBJECT)exception, (CLASS)handlerClass)) { 
                        /*  Matching exception handler has been found */
                        result = thisHandler;
                        break;
                    }
                }
            } 
        END_FOR_EACH_HANDLER

    END_TEMPORARY_ROOT
    return result;
}

/*=========================================================================
 * FUNCTION:      printHandler()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given exception handler
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to an exception handler object.
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
static void 
printHandler(HANDLER thisHandler) {
    fprintf(stdout,"Exception handler at %lx:\n", (long)thisHandler);
    fprintf(stdout,"\tStart PC..: %ld\n", (long)thisHandler->startPC);
    fprintf(stdout,"\tEnd PC....: %ld\n", (long)thisHandler->endPC);
    fprintf(stdout,"\tHandler PC: %ld\n", (long)thisHandler->handlerPC);
    fprintf(stdout,"\tException.: %ld \n", (long)thisHandler->exception);
    if (thisHandler->exception == 0) fprintf(stdout,"(finally)\n");
    fprintf(stdout,"\n");
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printExceptionHandlerTable()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given exception handler
 *                table for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to an exception handler table
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printExceptionHandlerTable(HANDLERTABLE handlerTable) {

    FOR_EACH_HANDLER(thisHandler, handlerTable) 
        printHandler(thisHandler);
    END_FOR_EACH_HANDLER
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Actual exception handling operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      handleException()
 * TYPE:          internal exception handling operation

 * INTERFACE:
 *   parameters:  pointer to an exception object
 *   returns:     <nothing>
 *=======================================================================*/


/*=========================================================================
 * FUNCTION:      throwException()
 * TYPE:          internal exception handling operation
 * OVERVIEW:      Throw an exception (to be handled by a matching 
 *                stack frame exception handler.  This operation may be
 *                called both from bytecode (by primitive ATHROW) or
 *                internally from the VM whenever an exceptional 
 *                situation occurs (e.g., an array range exception).
 *
 *                Handle a runtime exception by inspecting exception
 *                handlers in execution stack frames, and unwinding the
 *                execution stack if necessary in order to try to find
 *                a matching handler.  If we find a matching handler
 *                set the interpreter to execute its code.
 * INTERFACE:
 *   parameters:  pointer to an exception object
 *   returns:     <nothing>
 * NOTE:          When invoking this operation from inside the VM,
 *                keep in mind that the exception handler is not
 *                executed until the VM returns back to the interpreter.  
 *                Thus, you should not put any C/C++ code after calls
 *                to 'throwException' because that code would be called 
 *                "too early" (without having executed any of the 
 *                the exception handler bytecode.
 *=======================================================================*/

void throwException(INSTANCE exception) {
    /* These are required in case no handler is found.  We want to display
     * where the exception occurred. */
    FRAME saveFp = fp; 
    BYTE* saveIp = ip;
    cell* saveSp = sp;

    /* If we don't have any active threads in the system, there is no point
     * in trying to unwind stack frames. Print the detailed error message
     * contained in the second slot of the exception option and then exit. 
     * (this code should get executed only if there is an exception before
     * the VM has started properly). 
     */ 
    if (ActiveThreadCount == 0) {
        /* Print the possible error message; this is always stored in the
	 * first data field of the exception object.
	 */
        INSTANCE string = (INSTANCE)exception->data[0].cellp;
        if (string != NULL) {
            fatalError(getStringContents((STRING_INSTANCE)string));
        }
        ERROR_THROW(-1);
    }
    
    if (TRACEEXCEPTIONS) { 
	Log->throwException(exception);
    }

    /*  Start a loop for unwinding stack frames */
    while (fp != NULL) {
        /*  Check if the current execution frame/method */
        /*  has an exception handler table */
	METHOD thisMethod = fp->thisMethod;
	HANDLERTABLE handlerTable = thisMethod->u.java.handlers;
        if (handlerTable) {
            /*  Calculate the instruction pointer offset with method */
            unsigned short ipOffset = ip - fp->thisMethod->u.java.code;
            /* Check if the table contains a handler that catches the
	     * exception */
	    HANDLER  thisHandler = 
		findHandler(handlerTable, exception, ipOffset);
            if (thisHandler != NULL) {
                /*  If a handler is found, set the interpreter to invoke it */
                /*  as soon as we return back to the interpreter */
                ip = thisMethod->u.java.code + thisHandler->handlerPC;

                /*  Push the exception object to the operand stack  */
                /*  before the handler gets executed.  */
                sp = (cell*)(fp + 1);
                topStackAsType(INSTANCE) = exception;

		if (TRACEMETHODCALLS >= 2 && fp != saveFp) { 
		    exceptionCaughtTracing(exception, thisHandler);
		}
                /*  Return back to the interpreter */
                return;
            }
        } else if (thisMethod == RunCustomCodeMethod) { 
	    /* We make a copy of the exception, so that the C compiler
	     * won't be forced to put exception on the stack, rather than
	     * in a register */
	    INSTANCE exceptionCopy = exception;
            cell *stack = (cell*)(fp + 1);
	    CustomCodeCallbackFunction func = *(CustomCodeCallbackFunction *)stack;
	    ip = thisMethod->u.java.code;
	    func(stack + 1, &exceptionCopy);
	    /* Calling the function may change the exception.  Setting it
	     * to NULL means that we've completely handled it, and that
	     * we should just return */
	    exception = exceptionCopy;
	    if (exception == NULL) { 
		return;
	    }
	}
	
        /* If we're about to pop a frame, we must exit a monitor */
        if (fp->syncObject != NULL) { 
            monitorExit(fp->syncObject);
        }

        /* If we've reached the bottom of the stack, then we have found
         * an uncaught error. */
        if (fp->previousFp == NULL) { 
            break;
        }

	/* If no matching exception handler is found or if there are no
	 * exception handlers in the current stack frame, pop the stack frame
	 * and try to find a handler in the enclosing frame. */
	if (TRACEMETHODCALLS >= 2 && fp == saveFp) { 
	    exceptionThrownTracing(exception);
	}
        popFrame();
    }

    /* Print out an error message */
    Log->uncaughtException(exception);
    
    if (INCLUDEDEBUGCODE) { 
	/* Print a stack trace. */
	ip = saveIp;
	fp = saveFp;
	sp = saveSp;
	printStackTrace();
    }
    stopThread();
}


/*=========================================================================
 * Operations for raising VM-level exceptions
 * (see the detailed comparison of these operations in frame.h)
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      raiseException()
 * TYPE:          internal exception handling operation
 * OVERVIEW:      Raise an exception.  This operation is intended
 *                to be used from within the VM only.  It should
 *                be used for reporting only those exceptions and 
 *                errors that are known to be "safe" and "harmless",
 *                i.e., the internal consistency and integrity of 
 *                the VM should not be endangered in any way.
 *
 *                Upon execution, the operation tries to load the
 *                exception class with the given name, instantiate an 
 *                instance of that class, and 
 *                passes the object to the exception handler routines 
 *                of the VM. The operation will fail if the class can't
 *                be found or there is not enough memory to load it or
 *                create an instance of it.
 * INTERFACE:
 *   parameters:  exception class name
 *   returns:     <nothing>
 * NOTE:          Since this operation needs to allocate two new objects
 *                it should not be used for reporting memory-related
 *                problems.
 *=======================================================================*/

void raiseException(const char* exceptionClassName)
{
    INSTANCE_CLASS exceptionClass= (INSTANCE_CLASS)getClass(exceptionClassName);
    INSTANCE exception;
    if (exceptionClass == NULL) { 
        sprintf(str_buffer, "Illegal exception name %s", exceptionClassName);
        AlertUser(str_buffer);
        exceptionClass = JavaLangThrowable;
    }
    exception = instantiate(exceptionClass);
    if (exception != NULL) { 
	/* Otherwise, we will have already thrown an OutOfMemoryError */
	throwException(exception);
    }
}

/*=========================================================================
 * FUNCTION:      fatalVMError()
 * TYPE:          internal error handling operation
 * OVERVIEW:      Report a fatal error indicating that some severe
 *                unexpected situation has been encountered by the VM.
 *                This may be due to a bug in the VM.  VM execution will
 *                be stopped. This operation should be called only from 
 *                inside the VM.
 * INTERFACE:
 *   parameters:  error message string
 *   returns:     <nothing>
 *=======================================================================*/

void fatalVMError(const char* errorMessage)
{
    if (INCLUDEDEBUGCODE) { 
        printVMstatus();
    }
    fatalError(errorMessage);
}


/*=========================================================================
 * FUNCTION:      fatalError()
 * TYPE:          internal error handling operation
 * OVERVIEW:      Report a fatal error indicating that the execution
 *                of erroneous Java code might have endangered the 
 *                integrity of the VM. VM will be stopped. This 
 *                operation should be called only the from inside the VM.
 * INTERFACE:
 *   parameters:  error message string.
 *   returns:     <nothing>
 *=======================================================================*/

void fatalError(const char* errorMessage)
{
    AlertUser(errorMessage);
    ERROR_THROW(FATAL_ERROR_EXIT_CODE);
}


/*=========================================================================
 * Shortcuts to the errors/exceptions that the VM may throw
 *=======================================================================*/
const char* NullPointerException       = "java/lang/NullPointerException";
const char* IndexOutOfBoundsException  = "java/lang/IndexOutOfBoundsException";
const char* ArrayIndexOutOfBoundsException 
                                 = "java/lang/ArrayIndexOutOfBoundsException";
const char* ArrayStoreException        = "java/lang/ArrayStoreException";
const char* ArithmeticException        = "java/lang/ArithmeticException";
const char* ClassCastException         = "java/lang/ClassCastException";
const char* NegativeArraySizeException  
                                 =  "java/lang/NegativeArraySizeException";

