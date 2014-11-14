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
 * FUNCTION:      getClass()Ljava/lang/Class
 * CLASS:         java.lang.Object
 * TYPE:          virtual native function (reflection)
 * OVERVIEW:      Return the class object of the given object
 * INTERFACE (operand stack manipulation):
 *   parameters:  an object instance ('this')
 *   returns:     a class object
 * NOTE:          Current implementation does not create array
 *                classes yet, so arrays return JavaLangObject.
 *=======================================================================*/

void Java_java_lang_Object_getClass(void)
{
    topStackAsType(CLASS) = topStackAsType(OBJECT)->ofClass;
}

/*=========================================================================
 * FUNCTION:      hashCode()I, identityHashCode()I
 * CLASS:         java.lang.Object, java.lang.System
 * TYPE:          virtual native function
 * OVERVIEW:      Generate a unique hash code for an object.
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     hash code
 *=======================================================================*/

void Java_java_lang_Object_hashCode(void)
{
    topStackAsType(long) = (long)topStackAsType(OBJECT);
}

void Java_java_lang_System_identityHashCode(void)
{
    topStackAsType(long) = (long)topStackAsType(OBJECT);
}



/*=========================================================================
 * FUNCTION:      notify()V
 * CLASS:         java.lang.Object
 * TYPE:          virtual native function
 * OVERVIEW:      Release the first thread in the object's monitor.
 * INTERFACE (operand stack manipulation):
 *   parameters:  an object instance ('this')
 *   returns:     <nothing>
 * NOTE:          This operation does nothing
 *                if the current thread is not the owner of the monitor.
 *=======================================================================*/

void Java_java_lang_Object_notify(void)
{
    OBJECT object = popStackAsType(OBJECT);
    monitorNotify(object, FALSE);
}

/*=========================================================================
 * FUNCTION:      notifyAll()V
 * CLASS:         java.lang.Object
 * TYPE:          virtual native function
 * OVERVIEW:      Release all threads in the object's monitor.
 * INTERFACE (operand stack manipulation):
 *   parameters:  an object instance ('this')
 *   returns:     <nothing>
 * NOTE:          This operation does nothing
 *                if the current thread is not the owner of the monitor.
 *=======================================================================*/

void Java_java_lang_Object_notifyAll(void)
{
    OBJECT object = popStackAsType(OBJECT);
    monitorNotify(object, TRUE);
}

/*=========================================================================
 * FUNCTION:      wait()V
 * CLASS:         java.lang.Object
 * TYPE:          virtual native function
 * OVERVIEW:      Wait in the object's monitor forever.
 * INTERFACE (operand stack manipulation):
 *   parameters:  an object instance ('this').
 *   returns:     <nothing>
 * NOTE:          This operation should throw IllegalMonitorState-
 *                Exception if the current thread is not the owner of
 *                the monitor. Other possible exceptions are:
 *                IllegalArgumentException, InterruptedException.
 *=======================================================================*/

void Java_java_lang_Object_wait(void)
{

    long64  period;
    OBJECT object;

    popLong(period);
    object = popStackAsType(OBJECT);

    /* only block if the time period is not zero */
    if (ll_zero_ge(period)) {
        monitorWait(object, period);
    } else {
        raiseException("java/lang/IllegalArgumentException");
    }
}

/*=========================================================================
 * Native functions of class java.lang.Math
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      randomInt()I
 * CLASS:         java.lang.Math
 * TYPE:          static native function
 * OVERVIEW:      Return a random integer value
 * INTERFACE (operand stack manipulation):
 *   parameters:  none
 *   returns:     random integer
 *=======================================================================*/

void Java_java_lang_Math_randomInt(void)
{
    long result = RandomNumber_md();
    pushStack(result);
}

/*=========================================================================
 * Native functions of class java.lang.Class
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      isInterface()B
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function
 * OVERVIEW:      Return true if the class is an interface class.
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer
 *   returns:     Boolean indicating if the class is an interface.
 *=======================================================================*/

void Java_java_lang_Class_isInterface(void)
{
    CLASS clazz = topStackAsType(CLASS);
    topStack = ((clazz->accessFlags & ACC_INTERFACE) != 0);
}

void Java_java_lang_Class_isPrimitive(void)
{
    topStack = FALSE;                       /* this should go away */
}


/*=========================================================================
 * FUNCTION:      forName(Ljava/lang/String;)Ljava/lang/Class; (STATIC)
 * CLASS:         java.lang.Class
 * TYPE:          static native function (reflection)
 * OVERVIEW:      Given a string containing a class name (e.g.,
 *                "java.lang.Thread", return the corresponding
 *                class object.
 * INTERFACE (operand stack manipulation):
 *   parameters:  a string object
 *   returns:     a class object
 *=======================================================================*/

void Java_java_lang_Class_forName(void)
{
    STRING_INSTANCE string = topStackAsType(STRING_INSTANCE);
    if (string == NULL) {
        raiseException(NullPointerException);
    } else {
        char* className = getStringContents((STRING_INSTANCE)string);
        CLASS thisClass = NULL;
	if (!strchr(className, '/')) {
	    replaceLetters(className,'.','/');
	    if (verifyName(className, LegalClass, FALSE)) {
		thisClass = getClass(className);
		if (thisClass) {
		    topStackAsType(CLASS) = thisClass;
		    if (!IS_ARRAY_CLASS(thisClass)) {
			if (!CLASS_INITIALIZED((INSTANCE_CLASS)thisClass)) {
			    initializeClass((INSTANCE_CLASS)thisClass);
			}
		    }
		}
	    }
	}
        if (thisClass == NULL) {
            raiseException("java/lang/ClassNotFoundException");
        }
    }
}

/*=========================================================================
 * FUNCTION:      newInstance()Ljava/lang/Object; (VIRTUAL)
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function (reflection)
 * OVERVIEW:      Instantiate an object of the given class
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this'
 *   returns:     an instance of the class represented by 'this'
 * NOTE:          Current implementation does not create array
 *                classes properly.
 *=======================================================================*/

void Java_java_lang_Class_newInstance(void)
{
    CLASS clazz = topStackAsType(CLASS);
    bool_t samePackage =
        (fp->thisMethod->ofClass->class.packageName == clazz->packageName);

    if (IS_ARRAY_CLASS(clazz)
        || ((clazz->accessFlags & (ACC_INTERFACE | ACC_ABSTRACT)) != 0)) {
        raiseException("java/lang/InstantiationException");
        return;
    }

    if (samePackage ? ((clazz->accessFlags & ACC_PRIVATE ) == 0)
                   : ((clazz->accessFlags & ACC_PUBLIC  ) != 0)) {
        METHOD method = lookupMethod(clazz, initNameAndType);
        if (   (method != NULL)
            && (method->ofClass == (INSTANCE_CLASS)clazz)
            && (samePackage ? ((method->accessFlags & ACC_PRIVATE) == 0)
                            : ((method->accessFlags & ACC_PUBLIC )!= 0))) {
            INSTANCE object = instantiate((INSTANCE_CLASS)clazz);
            if (object != NULL) {
                /*  Put the result back on the stack */
                topStackAsType(INSTANCE) = object;
                /* Call the initializer.  Constructors can't be synchronized,
                 * so we don't need to worry about that part.
                 */
                pushStackAsType(INSTANCE, object);
		/* pushFrame may signal a stack overflow.  */
                pushFrame(method);
            } else {
                /* We will already have thrown an appropriate error */
            }
            return;
        }
    }
    raiseException("java/lang/IllegalAccessException");
    return;
}


/*=========================================================================
 * FUNCTION:      getName()Ljava/lang/String; (VIRTUAL)
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function
 * OVERVIEW:      Return the name of the class as a string.
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer
 *   returns:     Name of the class as a string.
 *=======================================================================*/

void Java_java_lang_Class_getName(void)
{
    CLASS clazz = topStackAsType(CLASS);
    START_TEMPORARY_ROOTS
        char *name = className(clazz);
        char *ptr = strchr(name, '/');
        /* Replace all slashes in the string with dots */
        while (ptr != NULL) {
            *ptr = '.';
            ptr = strchr(ptr + 1, '/');
        }
        topStackAsType(STRING_INSTANCE) = instantiateString(name, strlen(name));
    END_TEMPORARY_ROOTS
}
/*=========================================================================
 * FUNCTION:      boolean isArray()
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function
 * OVERVIEW:      true if the class is an array class
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer
 *   returns:     true if the object is an instance.
 *=======================================================================*/


void Java_java_lang_Class_isArray(void)
{
    CLASS clazz = topStackAsType(CLASS);
    topStack = !!IS_ARRAY_CLASS(clazz);
}

/*=========================================================================
 * FUNCTION:      boolean isInstance(Object)
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function
 * OVERVIEW:      true if the object is an instance of "this" class
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer, object
 *   returns:
 *=======================================================================*/

void Java_java_lang_Class_isInstance(void)
{
    OBJECT object = popStackAsType(OBJECT);
    CLASS thisClass = topStackAsType(CLASS);
    topStack = (object != NULL) && isInstanceOf(object, thisClass);
}


/*=========================================================================
 * FUNCTION:      boolean isAssignableFrom(Class)
 * CLASS:         java.lang.Class
 * TYPE:          virtual native function
 * OVERVIEW:      true if objects of the argument class can always be
 *                  assigned to variables of the "this" class.
 *   parameters:  'this' pointer, object
 *   returns:
 *=======================================================================*/


void Java_java_lang_Class_isAssignableFrom(void)
{
    CLASS argClass = popStackAsType(CLASS);
    CLASS thisClass = topStackAsType(CLASS);
    if (argClass == NULL) {
        raiseException(NullPointerException);
    } else {
        topStack = !!isAssignableTo(argClass, thisClass);
    }
}

/*=========================================================================
 * FUNCTION:      static byte[] getResourceAsStream0(String s);
 * CLASS:         java.lang.class
 * TYPE:          virtual native function
 * OVERVIEW:      Internal system function
 * INTERFACE
 *   parameters:
 *   returns:
 *
 * This is just a stub function, until the real definition is written
 *=======================================================================*/


void Java_java_lang_Class_getResourceAsStream0(void)
{
    topStackAsType(cell *) = NULL;
}

/*=========================================================================
 * Native functions of class java.lang.Thread
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      activeCount()I; (STATIC)
 * CLASS:         java.lang.Thread
 * TYPE:          static native function
 * OVERVIEW:      Return the number of threads alive.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     the number of threads alive
 *=======================================================================*/

void Java_java_lang_Thread_activeCount(void)
{
    pushStack(ActiveThreadCount);
}

/*=========================================================================
 * FUNCTION:      currentThread()Ljava/lang/Thread; (STATIC)
 * CLASS:         java.lang.Thread
 * TYPE:          static native function
 * OVERVIEW:      Return pointer to the currently executing Java thread.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     pointer to a JAVATHREAD object (instance of Thread.class)
 *=======================================================================*/

void Java_java_lang_Thread_currentThread(void)
{
    if (CurrentThread->javaThread == NIL) {
        InitializeJavaThread(CurrentThread);
    }
    pushStackAsType(JAVATHREAD, CurrentThread->javaThread);
}

/*=========================================================================
 * FUNCTION:      yield()V (STATIC)
 * CLASS:         java.lang.Thread
 * TYPE:          static native function
 * OVERVIEW:      Force a task switch.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void Java_java_lang_Thread_yield(void)
{
    signalTimeToReschedule();
}


/*=========================================================================
 * FUNCTION:      sleep(J)V (STATIC)
 * CLASS:         java.lang.Thread
 * TYPE:          static native function
 * OVERVIEW:      Pause execution of the thread for a timer period
 * INTERFACE (operand stack manipulation):
 *   parameters:  Long of the number of ms to sleep for
 *=======================================================================*/

void Java_java_lang_Thread_sleep(void)
{
    long64  period;
    popLong(period);

    /* only block if the time period is not zero */
    if (ll_zero_ge(period)) {
        /* Copy CurrentThread because suspendThread clears it */
        THREAD thisThread = CurrentThread;

        /* Suspend the current thread before we add the timer */
        suspendThread();

        /* Now add the timer (this is the safe way) */
        registerAlarm(thisThread, period, resumeThread);
    } else {
        raiseException("java/lang/IllegalArgumentException");
    }

}



/*=========================================================================
 * FUNCTION:      start()V (VIRTUAL)
 * CLASS:         java.lang.Thread
 * TYPE:          virtual native function
 * OVERVIEW:      Start executing the 'run' method of the target
 *                object given to the thread.
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer (implicitly)
 *   returns:     <nothing>
 *=======================================================================*/

void Java_java_lang_Thread_start(void)
{
    JAVATHREAD javaThread = popStackAsType(JAVATHREAD);
    INSTANCE target;
    METHOD thisMethod;
    THREAD VMthread;

    /*  Ensure that the internal thread execution  */
    /*  structure has been created */
    START_TEMPORARY_ROOT(javaThread)
        VMthread = getVMthread(javaThread);
    END_TEMPORARY_ROOT

    /*  Ensure that the thread is not already active */
    if (VMthread->state != THREAD_JUST_BORN) {
        raiseException("java/lang/IllegalThreadStateException");
        return;
    }

    /*  In Java, threads can be run in two different ways: */
    /*  either the run method is implemented in (a subclass of) */
    /*  the Thread class itself, or alternatively the thread */
    /*  object can be provided a separate Runnable object which */
    /*  contains the method to run.  Thread start-up must be done */
    /*  differently depending on the situation. */

    /*  Check if a separate Runnable object has been provided */
    target = (javaThread->target) ? javaThread->target : (INSTANCE)javaThread;

    /*  Find the 'run' method and set the thread to execute it */
    thisMethod =
        lookupMethod((CLASS)target->ofClass, getNameAndTypeKey("run", "()V"));


    initThreadBehavior(VMthread, thisMethod, (OBJECT)target);

    /*  Since 'run' is a virtual method, it must have  */
    /*  'this' in its first local variable */
    *(INSTANCE *)&VMthread->stack->cells[0] = target;

    /* Make the thread alive, and tell it that it can run */
    startThread(VMthread);
    resumeThread(VMthread);
}

/*=========================================================================
 * FUNCTION:      isAlive()Z (VIRTUAL)
 * CLASS:         java.lang.Thread
 * TYPE:          virtual native function
 * OVERVIEW:      Check if a thread is active.
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer (implicitly)
 *   returns:     true if thread is active, false otherwise
 *=======================================================================*/

void Java_java_lang_Thread_isAlive(void)
{
    topStack = isActivated(topStackAsType(JAVATHREAD)->VMthread);
}

/*=========================================================================
 * FUNCTION:      setPriority0(I)V (VIRTUAL)
 * CLASS:         java.lang.Thread
 * TYPE:          virtual native function
 * OVERVIEW:      Set the priority of the current thread.
 * INTERFACE (operand stack manipulation):
 *   parameters:  'this' pointer (implicitly), new priority
 *   returns:     <nothing>
 *=======================================================================*/

void Java_java_lang_Thread_setPriority0(void)
{
    int priority          = popStack();
    JAVATHREAD javaThread = popStackAsType(JAVATHREAD);
    THREAD VMthread;

    javaThread->priority = (priority > MAX_PRIORITY ? MAX_PRIORITY :
                           (priority < MIN_PRIORITY ? MIN_PRIORITY : priority));

    /*  Ensure that the internal thread execution  */
    /*  structure has been created */
    START_TEMPORARY_ROOT(javaThread)
        VMthread = getVMthread(javaThread);
    END_TEMPORARY_ROOT


    /*  The actual VM-level timeslice of the thread */
    /*  is calculated by multiplying the given priority */
    VMthread->timeslice = javaThread->priority*100;
}


/* TEMP - To allow old class file to work */
void Java_java_lang_Thread_setPriority(void)
{
    Java_java_lang_Thread_setPriority0();
}

/*=========================================================================
 * Native functions of class java.lang.Runtime
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      exit(I)V
 * CLASS:         java.lang.Runtime
 * TYPE:          virtual native function
 * OVERVIEW:      Stop the VM and exit immediately.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void Java_java_lang_Runtime_exitInternal(void)
{
    long value = popStack();
    oneLess;                                        /* discard runtime object */
    ERROR_THROW(value);
}


/*=========================================================================
 * FUNCTION:      freeMemory()J
 * CLASS:         java.lang.Runtime
 * TYPE:          static native function
 * OVERVIEW:      Return the amount of free memory in the dynamic heap.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   return:      the amount of free dynamic heap memory as long
 *=======================================================================*/

void Java_java_lang_Runtime_freeMemory(void)
{
    long result = memoryFree();
    ulong64 result64;
    ll_uint_to_long(result64, result);
    oneLess;                                        /* discard runtime object */
    pushLong(result64);
}

/*=========================================================================
 * FUNCTION:      totalMemory()J
 * CLASS:         java.lang.Runtime
 * TYPE:          static native function
 * OVERVIEW:      Return the total amount of dynamic heap memory.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   return:      the total amount of dynamic heap memory as long
 *=======================================================================*/

void Java_java_lang_Runtime_totalMemory(void)
{
    long result = getHeapSize();
    ulong64 result64;
    ll_uint_to_long(result64, result);

    oneLess;                    /* discard runtime object */
    pushLong(result64);
}

/*=========================================================================
 * FUNCTION:      gc()
 * CLASS:         java.lang.Runtime
 * TYPE:          static native function
 * OVERVIEW:      Invoke the garbage collector.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   return:      <nothing>
 *=======================================================================*/
void Java_java_lang_Runtime_gc(void)
{
    oneLess;                                        /* discard runtime object */

    /*  Garbage collect now, keeping the heap size the same as currently */
    garbageCollect(0);
}

/*=========================================================================
 * Native functions of class java.lang.System
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      arraycopy()B (VIRTUAL)
 * CLASS:         java.lang.System
 * TYPE:          virtual native function
 * OVERVIEW:      Copies a region of one array to another.
 *                Copies a region of one array, src, beginning at the
 *                                array cell at src_position, to another array, dst,
 *                beginning at the dst_position.length cells are copied.
 *                No memory is allocated for the destination array, dst.
 *                This memory must already be allocated.
 *
 *
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   return:      <none>
 *=======================================================================*/

/*
public static native void arraycopy(Object src,
                                    int src_position,
                                    Object dst,
                                    int dst_position,
                                    int length)
*/

void Java_java_lang_System_arraycopy (void)
{
    int length  = popStack();
    int dstPos = popStack ();
    ARRAY dst   = popStackAsType(ARRAY);
    int srcPos = popStack ();
    ARRAY src   = popStackAsType(ARRAY);
    ARRAY_CLASS  srcClass, dstClass;

    int srcEnd, dstEnd;

    if ((src == NULL) || (dst == NULL)) {
        raiseException(NullPointerException);
        return;
    }
    srcClass = src->ofClass;
    dstClass = dst->ofClass;

    if (    (!IS_ARRAY_CLASS((CLASS)srcClass))
         || (!IS_ARRAY_CLASS((CLASS)dstClass))
         /* both must be arrays of Objects, or arrays of primitives */
         || (srcClass->gcType != dstClass->gcType)
         /* If arrays of primitives, must be the same primitive */
         || (srcClass->gcType == GCT_ARRAY
               && (srcClass->u.elemClass != dstClass->u.elemClass))
            ) {
        raiseException("java/lang/ArrayStoreException");
        return;
    }

    srcEnd = length + srcPos;
    dstEnd = length + dstPos;

    if (     (length < 0) || (srcPos < 0) || (dstPos < 0)
          || (length > 0 && (srcEnd < 0 || dstEnd < 0))
          || (srcEnd > (int)src->length)
          || (dstEnd > (int)dst->length)) {
        raiseException("java/lang/ArrayIndexOutOfBoundsException");
        return;
    }

    if (src->ofClass->gcType == GCT_ARRAY) {
        /* We've already determined that src and dst are primitives of the
         * same type */
        int itemSize = src->ofClass->itemSize;
        memmove(&((BYTEARRAY)dst)->bdata[dstPos * itemSize],
                &((BYTEARRAY)src)->bdata[srcPos * itemSize],
                itemSize * length);
    } else {
        CLASS srcElementClass = srcClass->u.elemClass;
        CLASS dstElementClass = dstClass->u.elemClass;
        if (!isAssignableTo(srcElementClass, dstElementClass)) {
            /* We have to check each element to make sure it can be
             * put into an array of dstElementClass */
            int i;
            for (i = 0; i < length; i++) {
                OBJECT item = (OBJECT)src->data[srcPos + i].cellp;
                if ((item != NULL) && !isInstanceOf(item, dstElementClass)) {
                    raiseException("java/lang/ArrayStoreException");
                    break;
                } else {
                    dst->data[dstPos + i].cellp = (cell *)item;
                }
            }
        } else {
            memmove(&dst->data[dstPos], &src->data[srcPos], length << log2CELL);
        }
    }
}

/*=========================================================================
 * FUNCTION:      currentTime()I (STATIC)
 * CLASS:         java.lang.System
 * TYPE:          static native function
 * OVERVIEW:      Return the number of centiseconds since VM start.
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     current time in centisecs in operand stack as integer.
 *=======================================================================*/

void Java_java_lang_System_currentTimeMillis(void)
{
    ulong64 result = CurrentTime_md();
    pushLong(result);
}

/*=========================================================================
 * FUNCTION:      getProprty0(Ljava.lang.String)Ljava.lang.String (STATIC)
 * CLASS:         java.lang.System
 * TYPE:          static native function
 * OVERVIEW:      Return a property
 * INTERFACE (operand stack manipulation):
 *   parameters:  A string
 *   returns:     A string
 *=======================================================================*/

void Java_java_lang_System_getProperty0(void)
{
    char *getProperty(char *key);
    STRING_INSTANCE string = topStackAsType(STRING_INSTANCE);
    char* key = getStringContents(string);
    char* value = 0;

    /*
    * Look up the property value with the system dependent routine
    */
    value = getProperty(key);

    if (value != 0) {
        goto done;
    }

    if (strcmp(key, "microedition.configuration") == 0) {
        value = "CLDC-1.0";
        goto done;
    }

    if(strcmp(key, "microedition.platform") == 0) {
#ifdef PLATFORMNAME
        value = PLATFORMNAME;
        goto done;
#else
#ifdef PILOT
        value = "palm";
        goto done;
#endif
#endif
    }

#if GENERICEVENTS
    if (strcmp(key, "microedition.new.events") == 0) {
        value =   "true";                    /* Flag for Spotlet.java */
        goto done;
    }
#endif

#if GENERICSTORAGE
#else
#if !PILOT
    if (strcmp(key, "microedition.no.storage") == 0) {
        value =   "true";
        goto done;
    }
#endif
#endif


#if GENERICNETWORK
#else
    if (strcmp(key, "microedition.no.network") == 0) {
        value =   "true";
        goto done;
    }
#endif

    if (strcmp(key, "microedition.encoding") == 0) {
        value =   "ISO8859_1";
        goto done;
    }

done:
    topStackAsType(STRING_INSTANCE) =
        (value == NULL) ? NULL : instantiateString(value, strlen(value));
}


/* TEMP - So old classes will still work */
void Java_java_lang_System_getProperty(void)
{
    Java_java_lang_System_getProperty0();
}

/*=========================================================================
 * Native functions of class kauai.debug.NativeIO
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      putchar(C)V (STATIC)
 * CLASS:         com/sun/cldc/io/j2me/debug/PrivateOutputStream
 * TYPE:          static native function
 * OVERVIEW:      Print a byte to the stdout file
 * INTERFACE (operand stack manipulation):
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/


void Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar(void)
{
    int c = popStack();
    putchar(c);
}

/* TEMP - To allow old classes to work */
void Java_com_sun_kjava_DebugIO_putchar(void)
{
    Java_com_sun_cldc_io_j2me_debug_PrivateOutputStream_putchar();
}

void Java_java_lang_String_intern(void)
{
    fatalError("Intern is not yet implemented");
}


/*=========================================================================
 * Native functions of class java.util.Calendar
 *=======================================================================*/

void Java_java_util_Calendar_init(void)
{
    unsigned long *date;

    ARRAY dateArray = popStackAsType(ARRAY);
    oneLess;                                        /* ignore "this" */

    /* initialize the dateArray */
    memset(&dateArray->data[0], 0, MAXCALENDARFLDS*sizeof(dateArray->data[0]));

    date = Calendar_md();

    /* copy over the date array back */
    memcpy(&dateArray->data[0], date, MAXCALENDARFLDS*sizeof(dateArray->data[0]));
}

