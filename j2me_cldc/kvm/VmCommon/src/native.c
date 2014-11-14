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
 * SUBSYSTEM: Native function interface
 * FILE:      native.cpp
 * OVERVIEW:  This file defines the native functions needed 
 *            by the Java virtual machine. The implementation
 *            is _not_ based on JNI (Java Native Interface),
 *            because it seems too complicated for small devices.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (hashing, nano libraries)
 * NOTE:      Many functions in this file are platform-specific,
 *            and require extra work when porting the system to 
 *            run on another machine.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Operations on native functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getNativeFunction()
 * TYPE:          lookup operation
 * OVERVIEW:      Given a function name as a string, try to find
 *                a corresponding native function and return a
 *                a pointer to it if found.
 * INTERFACE:
 *   parameters:  class name, method name
 *   returns:     function pointer or NIL if not found.
 *=======================================================================*/

NativeFunctionPtr
getNativeFunction(const char* className, const char* methodName)
{
#if !ROMIZING
    const ClassNativeImplementationType *cptr;
    const NativeImplementationType *mptr;

    for (cptr = nativeImplementations; ; cptr++) { 
        const char *name = cptr->name;
        if (name == NULL) 
            return NULL;
        if (strcmp(name, className) == 0)
            break;
    }
    for (mptr = cptr->implementation; ; mptr++) { 
        const char *name = mptr->name;
        if (name == NULL) {
            return NULL;
        }
        if (strcmp(name, methodName) == 0) { 
            return mptr->implementation;
        }
    }
#else
    return NULL;
#endif
}

/*=========================================================================
 * FUNCTION:      invokeNativeFunction()
 * TYPE:          private operation
 * OVERVIEW:      Invoke a native function, resolving the native
 *                function reference if necessary.
 * INTERFACE:
 *   parameters:  method pointer
 *   returns:     <nothing>
 * NOTE:          Native functions are automatically synchronized, i.e.,
 *                they cannot be interrupted by other threads unless 
 *                the native code happens to invoke the Java interpreter.
 *=======================================================================*/

void invokeNativeFunction(METHOD thisMethod)
{
    
    NativeFunctionPtr native = thisMethod->u.native.code;

    if (native == NULL) { 
        /*  Native function not found; throw error */
        START_TEMPORARY_ROOTS
            sprintf(str_buffer,"Native method '%s::%s' not found",
                    className(thisMethod->ofClass), methodName(thisMethod));
        END_TEMPORARY_ROOTS
        fatalError(str_buffer);
    }
    


    frameTracing(thisMethod, "=>", +1);

    if (INCLUDEDEBUGCODE && !ASYNCHRONOUS_NATIVE_FUNCTIONS) { 
        cell* expectedSP = sp + getReturnSize(thisMethod)- thisMethod->argCount;
        FRAME expectedFP = fp;
        BYTE *expectedIP = ip;

        native();
        
        if (expectedFP == fp && expectedIP == ip && expectedSP != sp) { 
            /* We presume that if the fp or ip changed, the native code was 
             * doing something special, and we shouldn't interfere.
             * Otherwise, we check to make sure the sp is the value that
             * we expected.
             */
            START_TEMPORARY_ROOTS
                sprintf(str_buffer, 
                        "Bad native stack manipulation: %s.%s. Stack off by %d",
                        className(thisMethod->ofClass), methodName(thisMethod), 
                        sp - expectedSP);
                fatalError(str_buffer);
            END_TEMPORARY_ROOTS
        }
    } else { 
	native();
    }
    frameTracing(thisMethod, "<=", +1);
}

/*=========================================================================
 * FUNCTION:      printNativeFunctions()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print all the native functions currently referred
 *                to by the classes in the system.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE

void printNativeFunctions()
{
    int counter = 0;

    FOR_ALL_CLASSES(clazz)
        if (!IS_ARRAY_CLASS(clazz)) {
            INSTANCE_CLASS iClazz = (INSTANCE_CLASS)clazz;
            FOR_EACH_METHOD(thisMethod, iClazz->methodTable) 
            if (thisMethod->accessFlags & ACC_NATIVE) {
                START_TEMPORARY_ROOTS
                    fprintf(stdout,"Class '%s' native '%s%s' ", 
                            className(iClazz), 
                            methodName(thisMethod), methodSignature(thisMethod));
                    if (thisMethod->accessFlags & ACC_STATIC)
                        fprintf(stdout, "(STATIC)\n");
                    else fprintf(stdout, "(VIRTUAL)\n");
                    counter++;
                END_TEMPORARY_ROOTS
            }
            END_FOR_EACH_METHOD
        }
    END_FOR_ALL_CLASSES
    fprintf(stdout, "\n%d native functions referred to in the system\n", 
            counter);
}

#endif /*  INCLUDEDEBUGCODE */
