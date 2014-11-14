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
 * SUBSYSTEM: Internal runtime structures
 * FILE:      class.c
 * OVERVIEW:  Internal runtime class structures (see Class.h).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *	      Edited by Doug Simon 11/1998 (added the string pool)
 *            Added access checks by Sheng Liang for VM-spec compliance
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stddef.h>

static char signatureToTypeCode(char);
static char typeCodeToSignature(char);
static void runClinit(cell *argp, INSTANCE*);

/*=========================================================================
 * Global variables and definitions. These variables are initialized in
 * InitializeMemory to ensure that they are reset each execution of the VM.
 *=======================================================================*/

/*  Pointers to the most important Java classes needed by the VM */
/*  If Romizing, these are defined and initialized in ROMjava.c */

#if ROMIZING
#  define EXTERN_IF_ROMIZING extern
#else 
#  define EXTERN_IF_ROMIZING
#endif

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangObject;	  /*  Pointer to class 'java.lang.Object' */

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangClass;	  /*  Pointer to class 'java.lang.Class' */

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangSystem;	  /*  Pointer to class 'java.lang.System' */

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangString;	  /*  Pointer to class 'java.lang.String' */

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangThread;	  /*  Pointer to class 'java.lang.Thread' */

EXTERN_IF_ROMIZING
INSTANCE_CLASS JavaLangThrowable; /*  Pointer to class 'java.lang.Throwable' */

EXTERN_IF_ROMIZING
ARRAY_CLASS    JavaLangCharArray; /*  Array of characters */

EXTERN_IF_ROMIZING METHOD RunCustomCodeMethod;

EXTERN_IF_ROMIZING NameTypeKey initNameAndType;	  /* void <init>() */
EXTERN_IF_ROMIZING NameTypeKey clinitNameAndType; /* void <clinit>() */
EXTERN_IF_ROMIZING NameTypeKey runNameAndType;    /* void run() */


INSTANCE_CLASS JavaLangOutOfMemoryError;
INSTANCE OutOfMemoryObject;
INSTANCE StackOverflowObject;

/*=========================================================================
 * Constructors
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      initializeClass()
 * TYPE:          constructor
 * OVERVIEW:      After loading a class, it must be initialized
 *                by executing the possible internal static
 *                constructor '<clinit>'. This will initialize the
 *                necessary static structures.
 *
 *		  This function sets up the necessary Class.runClinit
 *		  frame and returns to the interpreter.
 * INTERFACE:
 *   parameters:  class pointer
 *   returns:	  <nothing>
 *=======================================================================*/

void initializeClass(INSTANCE_CLASS thisClass)
{
    if (thisClass->status < CLASS_VERIFIED) {
	if (thisClass->superClass != NULL &&
	    thisClass->superClass->class.accessFlags == ACC_FINAL) {
	    START_TEMPORARY_ROOTS
	    sprintf(str_buffer, 
		    "Class %s extends final class %s",
		    className(thisClass),
		    className(thisClass->superClass));
	    fatalError(str_buffer);
	    END_TEMPORARY_ROOTS
	}
	if ((thisClass->class.accessFlags & ACC_INTERFACE) &&
	    thisClass->superClass != JavaLangObject) {
	    START_TEMPORARY_ROOTS
	    sprintf(str_buffer, 
		    "Interface %s does not extend java.lang.Object class",
		    className(thisClass));
	    fatalError(str_buffer);
	    END_TEMPORARY_ROOTS
	}
	if (verifyClass(thisClass)) {
	    START_TEMPORARY_ROOTS
	    sprintf(str_buffer, 
		    "Error verifying class %s", className(thisClass));
	    fatalError(str_buffer);
            END_TEMPORARY_ROOTS
	}
    }
    if (pushFrame(RunCustomCodeMethod)) { 
	pushStackAsType(CustomCodeCallbackFunction, &runClinit);
	pushStackAsType(INSTANCE_CLASS, thisClass);
	pushStackAsType(long, 1);
    } else { 
	/* Stack overflow */
	thisClass->status = CLASS_ERROR;
    }
}

/*=========================================================================
 * FUNCTION:	  runClinit()
 * TYPE:	  private initialization of a class
 * OVERVIEW:	  Initialize a class. The Class.runCustomCode frame has
 *		  already been pushed.
 *
 *        This function follows the exact steps as documented in
 *        the Java virtual machine spec (2ed) 2.17.5, except that
 *        Error is used instead of ExceptionInInitializerError
 *        because the latter is not defined in CLDC.
 *
 * Notes:	  This is a "CustomCode" callback function.  Therefore,
 *		  when it is called, argP[0] will point to the "thisClass"
 *		  on the stack, and argP[1] points to the state argument
 * INTERFACE:
 *   parameters:  argP:  See notes
 *                exceptionP: NULL if called from the interpret. A pointer to 
 *                    the current exception if called from the error handler. 
 *                    We can either set the pointer to NULL to indicate we've
 *                    completely handled it, or we can replace it with a
 *                    different exception.
 *
 *   returns:	  <nothing>
 *=======================================================================*/

static void 
runClinit(cell *argP, INSTANCE* exceptionP)
{
    INSTANCE_CLASS thisClass = *(INSTANCE_CLASS*)(&argP[0]);
    int state = argP[1];
    bool_t haveMonitor = FALSE;

    if (exceptionP != NULL) {
        /* Handle exception during clinit */
        INSTANCE exception = *exceptionP;
        CLASS JavaLangError = getClass("java/lang/Error");
        if (!isInstanceOf((OBJECT)exception, JavaLangError)) {
            /* Replace exception with Error, then continue the throwing */
            INSTANCE error = instantiate((INSTANCE_CLASS)JavaLangError);
            char *p = str_buffer;
            STRING_INSTANCE messageString = (STRING_INSTANCE)exception->data[0].cellp;
            /* Create the new message:
             *   Static initializer: <className of throwable> <message>
             */
            strcpy(p, "Static initializer: "); 
            p += strlen(p);
            getClassName_inBuffer(&exception->ofClass->class, p);
            p += strlen(p);
            if (messageString != NULL) {
                strcpy(p, ": ");
                p += strlen(p);
                getStringContentsSafely(messageString, p,
                                STRINGBUFFERSIZE - (p - str_buffer));
                p += strlen(p);
            }
            error->data[0].cellp = 
	            (cell*)instantiateString(str_buffer, p - str_buffer);
            /* Replace the exception with our new Error, continue throwing */
            *exceptionP = error;
        }
        return;
    }

    /* The 11 steps as documented in page 53 of the virtual machine spec. */
    switch (state) {
    case 1:
	/* A short cut that'll probably happen 99% of the time.  This class
	 * has no monitor, and no one is in the middle of initializing this
	 * class.  Since our scheduler is non preemptive, we can just
	 * mark the class, without worrying about the monitor or other threads.
	 */
	if (thisClass->class.monitor == NULL 
	        && thisClass->initThread == NULL) { 
	    goto markClass;
	}

	/* Step 1:  Grab the class monitor so we have exclusive access. */
	if (monitorEnter((OBJECT)thisClass) != MonitorStatusOwn) { 
	    /* We've been forced to wait.  When we're awoken, we'll have
	     * the lock */
	    argP[1] = 2;
	    return;
	} else {
	    /* FALL THROUGH.  We have the lock */
	}

    case 2:
	haveMonitor = TRUE;
	if (thisClass->initThread && thisClass->initThread != CurrentThread) {
	    /* Step 2:
	     * Someone else is initializing this class.  Just wait until
	     * a notification.  Of course, we'll have to recheck, since the
	     * class could also be notified for other reasons.
	     */
	    long64 timeout;
	    ll_setZero(timeout);
	    monitorWait((OBJECT)thisClass, timeout);
	    argP[1] = 2;
	    return;
	} else if (thisClass->initThread == CurrentThread ||
		   thisClass->status == CLASS_READY) { /* step 4 */
	    /* Step 3, Step 4:
	     * This thread is already initializing the class, or the class
	     * has somehow already become initialized.  We're done.
	     */
	    monitorExit((OBJECT)thisClass);
	    popFrame();
	    return;
	} else if (thisClass->status == CLASS_ERROR) {
	    /* Step 5:
	     * What can we do?
	     */
	    fatalVMError("bad class state");
	    return;
	} 
    markClass:
	/* Step 6:
	 * Mark that we're about to initialize this class */
	setClassInitialThread(thisClass, CurrentThread);
	if (haveMonitor) { 
	    monitorExit((OBJECT)thisClass);
	    haveMonitor = FALSE;
	}
	/* FALL THROUGH */
	
    case 3:
	/* Step 7:
	 * Initialize the superclass, if necessary */
	if ((thisClass->class.accessFlags & ACC_INTERFACE) == 0) {
	    INSTANCE_CLASS superClass = thisClass->superClass;
	    if (superClass && superClass->status != CLASS_READY) {
		argP[1] = 4;
		initializeClass(superClass);
		return;
	    }
	}
	/* FALL THROUGH */

    case 4: {
	/* Step 8:
	 * Run the <clinit> method, if the class has one
	 */
	METHOD thisMethod = getSpecialMethod(thisClass, clinitNameAndType);
	if (thisMethod) {
	    if (Verbose_On) {
		fprintf(stdout,"Invoking static initializer\n");
	    }
	    if (TRACECLASSLOADING) {
		START_TEMPORARY_ROOTS
		    fprintf(stdout, "Initializing class: '%s'\n",
			    className(thisClass));
		END_TEMPORARY_ROOTS
	    }
	    argP[1] = 5;
	    if (!pushFrame(thisMethod)) { 
		/* Stack overflow while pushing the frame */
		thisClass->status = CLASS_ERROR;
		popFrame();
	    }
	    return;
	} else {
	    /* No <clinit method. */
	    /* FALL THROUGH */
	}
    }
    case 5:
	/* Step 9:
	 * Grab the monitor so we can change the flags, and wake up any
	 * other thread waiting on us.
	 *
	 * SHORTCUT:  99% of the time, there is no contention for the class.
	 * Since our scheduler is non-preemptive, if there is no contention
	 * for this class, we just go ahead and unmark the class, without
	 * bothering with the monitor.
	 */

	if (thisClass->class.monitor == NULL) { 
	    goto unmarkClass;
	}

	if (monitorEnter((OBJECT)thisClass) != MonitorStatusOwn) { 
	    /* When we wake up, we'll have the monitor */
	    argP[1] = 6;
	    return;
	} else {
	    /* FALL THROUGH */
	}

    case 6:
	haveMonitor = TRUE;
	/* Step 9, cont.
	 * Mark the class as initialized. Wake up anyone waiting for the
	 * class to be initialized.  Return the monitor.
	 */
    unmarkClass:
	setClassInitialThread(thisClass, NULL);
	setClassStatus(thisClass, CLASS_READY);
	if (haveMonitor) { 
	    monitorNotify((OBJECT)thisClass, TRUE); /* wakeup everyone */
	    monitorExit((OBJECT)thisClass);
	}
	popFrame();
	return;
	/* Step 10, 11:
	 * These handle error conditions that cannot currently be
	 * implemented in the KMV.
	 */

    default:
	fatalVMError("unexpected");
    }
}



/*=========================================================================
 * System-level constructors
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:	  InitializeJavaSystemClasses()
 * TYPE:	  private constructor
 * OVERVIEW:	  Load the standard Java system classes needed by the VM.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:	  <nothing>
 *=======================================================================*/

void InitializeJavaSystemClasses()
{
    /* This just creates empty structures that will eventually be filled in.
     * The magic numbers are the lengths of the strings.
     * The order is slightly important.
     *   JavaLangObject has to be created before JavaLangCharArray.
     *   These four classes have to be created before anything is loaded.
     */
    if (!ROMIZING) {
	JavaLangObject = (INSTANCE_CLASS)getRawClass("java/lang/Object", 16);
	JavaLangClass  = (INSTANCE_CLASS)getRawClass("java/lang/Class",	 15);
	JavaLangString = (INSTANCE_CLASS)getRawClass("java/lang/String", 16);
	JavaLangCharArray = getArrayClass(1, NULL, 'C');

	/* Now we can go back and create these for real. . . .*/
	loadClassfile(JavaLangObject, TRUE);
	loadClassfile(JavaLangClass, TRUE);
	loadClassfile(JavaLangString, TRUE);

	/* Load or initialize some other classes */
	JavaLangSystem = (INSTANCE_CLASS)getClass("java/lang/System");
	JavaLangThread = (INSTANCE_CLASS)getClass("java/lang/Thread");
	JavaLangThrowable = (INSTANCE_CLASS)getClass("java/lang/Throwable");

	if (INCLUDEDEBUGCODE) {
	    if (   (JavaLangObject->status == CLASS_ERROR)
		|| (JavaLangClass->status == CLASS_ERROR)
		|| (JavaLangString->status == CLASS_ERROR)
		|| (JavaLangThread == NULL)
		|| (JavaLangThrowable == NULL)) {
		fatalVMError("Unable to initialize system classes");
	    }
	}
    } else {
	InitializeROMImage();
    }

    if (!ROMIZING || RELOCATABLE_ROM) {
	initNameAndType	  = getNameAndTypeKey("<init>",	  "()V");
	clinitNameAndType = getNameAndTypeKey("<clinit>", "()V");
	runNameAndType	  = getNameAndTypeKey("run",	 "()V");

	/* Get pointers to the special methods.	 CustomCode isn't used yet,
	 * but probably will be in the future */
	RunCustomCodeMethod = getSpecialMethod(JavaLangClass,
				   getNameAndTypeKey("runCustomCode", "()V"));

	/* patch the bytecode, was a "return" */
	if (RELOCATABLE_ROM) {
	    /* Do nothing.	Already patched */
	} else if (!USESTATIC || (inHeapSpace(RunCustomCodeMethod->u.java.code))) {
	    RunCustomCodeMethod->u.java.code[0] = CUSTOMCODE;
	} else {
	    BYTE newcode = CUSTOMCODE;
	    char *start = (char*)((INSTANCE_CLASS)JavaLangClass)->constPool;
	    int offset = (char*)(RunCustomCodeMethod->u.java.code) - start;
	    modifyStaticMemory(start, offset, &newcode, sizeof(newcode));
	}
    }
    JavaLangOutOfMemoryError = 
	(INSTANCE_CLASS)getClass("java/lang/OutOfMemoryError");
    OutOfMemoryObject = instantiate(JavaLangOutOfMemoryError);
    makeGlobalRoot((cell **)&OutOfMemoryObject);

#if 0    
    JavaLangStackOverflowError = 
	(INSTANCE_CLASS)getClass("java/lang/StackOverflowError");
    StackOverflowObject = instantiate(JavaLangStackOverflowError);
    makeGlobalRoot((cell **)&StackOverflowObject);
#else
    /* For now, java.lang.StackOverflowError doesn't exist, and we throw
     * OutOfMemoryError instead.  
     */
    StackOverflowObject = OutOfMemoryObject;
#endif

}


/*=========================================================================
 * FUNCTION:	  FinalizeJavaSystemClasses()
 * TYPE:	  private constructor
 * OVERVIEW:	  Perform any cleanup necessary so that the class files can
 *		  be run again.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:	  <nothing>
 *=======================================================================*/

void FinalizeJavaSystemClasses()
{
    if (ROMIZING) {
	FinalizeROMImage();
	FOR_ALL_CLASSES(clazz)
	    /* Remove any class monitors */
	    setObjectMonitor((OBJECT)clazz, NULL);
	    if (!IS_ARRAY_CLASS(clazz)) {
		/* Reset the state of the class to be its initial state.
		 * The ACC_ROM_NON_INIT_CLASS says that neither this class nor
		 * any of its superclasses has a <clinit> method */
		INSTANCE_CLASS iclazz = ((INSTANCE_CLASS)clazz);
		setClassInitialThread(iclazz, NULL);
		setClassStatus(iclazz, 
		     (iclazz->class.accessFlags & ACC_ROM_NON_INIT_CLASS) ?
			      CLASS_READY : CLASS_VERIFIED);
	    }
	END_FOR_ALL_CLASSES
    }
}


INSTANCE_CLASS getCurrentClass()
{
    INSTANCE_CLASS class = NULL;
    FRAME frame = fp;
    while (frame && frame->thisMethod) {
	class = frame->thisMethod->ofClass;
	if (class != JavaLangClass || frame->thisMethod != RunCustomCodeMethod) {
	    break;
	}
	class = NULL;
	frame = frame->previousFp;
    }
    return class;
}

/*=========================================================================
 * Operations on classes
 *=======================================================================*/


/*=========================================================================
 * FUNCTION:	  getRawClass()
 * TYPE:	  instance-level operation on runtime classes
 * OVERVIEW:	  Find a class with the given name, or create an empty
 *		  stub for the class, if necessary.
 * INTERFACE:
 *   parameters:  class name as UString
 *   returns:	  class pointer
 *
 *=======================================================================*/

CLASS
getRawClass(const char *start, int length)
{
    const char *firstNonBracket, *p;
    int depth;

    /* Find the firstNonBracket character, and the last slash in the string */
    for (p = start; *p == '['; p++) {};
    firstNonBracket = p;
    depth = p - start;

    if (depth == 0) {
	UString packageName, baseName;
	CLASS clazz;
	for (p = start + length ; ;) {
	    --p;
	    if (*p == '/') {
		packageName = getUStringX(start, p - start);
		baseName = getUStringX(p + 1, (length - 1) - (p - start));
		break;
	    }
	    if (p == start) {
		packageName = NULL;
		baseName = getUStringX(start, length);
		break;
	    }
	}
	clazz = change_Name_to_CLASS(packageName, baseName, GCT_INSTANCE_CLASS);
	return clazz;
    } else if (depth + 1 == length) {
	/* An array of some primitive type */
	return (CLASS)getArrayClass(depth, NULL, *firstNonBracket);
    } else {
	const char *baseClassStart = firstNonBracket + 1;	/* skip the 'L' */
	const char *baseClassEnd = start + length - 1;	/* skip the final ';' */
	CLASS baseClass =
	    getRawClass(baseClassStart, baseClassEnd - baseClassStart);
	return (CLASS)getArrayClass(depth, (INSTANCE_CLASS)baseClass, '\0');
    }
}

/*=========================================================================
 * FUNCTION:	  getArrayClass()
 * TYPE:	  private helper function
 * OVERVIEW:	  Create an array class.
 * INTERFACE:
 *   arguments:	  depth:     depth of the new array
 *		  baseClass: if the base type is a class this is nonNull and
 *			     contains the base type
 *		  signCode:  if the base type is a primitive type, then
 *			     baseClass must be NULL, and this contains the
 *			     letter representing the base type.
 *   parameters:  result: class pointer.
 *=======================================================================*/

ARRAY_CLASS
getArrayClass(int depth, INSTANCE_CLASS baseClass, char signCode)
{
	UString packageName, baseName;
	bool_t isPrimitiveBase = (baseClass == NULL);
	ARRAY_CLASS clazz, result;


	if (isPrimitiveBase) {
		packageName = NULL;
		memset(str_buffer, '[', depth);
		str_buffer[depth] = signCode;
		baseName = getUStringX(str_buffer, depth + 1);
	} else {
		if (baseClass->status == CLASS_RAW) {
			/* This may bash str_buffer, so we can't move the memset to
			 * before the "if" */
			loadClassfile(baseClass, TRUE);
		}
		memset(str_buffer, '[', depth);
		sprintf(str_buffer + depth, "L%s;", UStringInfo(baseClass->class.baseName));
		baseName = getUString(str_buffer);
		packageName = baseClass->class.packageName;
	}

	result = (ARRAY_CLASS)change_Name_to_CLASS(packageName, baseName,
											   GCT_ARRAY_CLASS);

	for (clazz = result; clazz->class.ofClass == NULL; ) {
		/* LOOP ASSERT:	 depth is the array depth of "clazz" */

		/* This clazz is newly created.	 We need to fill in info.  We
		 * may need to iterate, since this class's element type may also need
		 * to be be created */
		clazz->class.ofClass = JavaLangClass;
		if (depth == 1 && isPrimitiveBase) {
			char typeCode = signatureToTypeCode(signCode);
			clazz->gcType = GCT_ARRAY;
			clazz->itemSize = arrayItemSize(typeCode);
			clazz->u.primType = typeCode;
			clazz->class.accessFlags =
				ACC_FINAL | ACC_ABSTRACT | ACC_PUBLIC | ACC_ARRAY_CLASS;
			clazz->class.key = signCode + (short)(1 << FIELD_KEY_ARRAY_SHIFT);
			/* All finished */
			break;
		} else {
			clazz->gcType = GCT_OBJECTARRAY;
			clazz->itemSize = arrayItemSize(T_REFERENCE);
			clazz->class.accessFlags =
				isPrimitiveBase || (baseClass->class.accessFlags & ACC_PUBLIC)
				     ? ACC_FINAL | ACC_ABSTRACT | ACC_ARRAY_CLASS
				     : ACC_FINAL | ACC_ABSTRACT | ACC_ARRAY_CLASS | ACC_PUBLIC;
			if (depth >= MAX_FIELD_KEY_ARRAY_DEPTH) {
				clazz->class.key |=
					(MAX_FIELD_KEY_ARRAY_DEPTH << FIELD_KEY_ARRAY_SHIFT);
			} else if (isPrimitiveBase) {
				clazz->class.key =
					(depth << FIELD_KEY_ARRAY_SHIFT) + signCode;
			} else {
				clazz->class.key =
					(depth << FIELD_KEY_ARRAY_SHIFT) + baseClass->class.key;
			}

			if (depth == 1) {
				/* We must be nonPrimitive.  primitive was handled above */
				clazz->u.elemClass = (CLASS)baseClass;
				/* All finished */
				break;
			} else {
				/* Class of depth > 1. */
				UString thisBaseName = clazz->class.baseName;
				UString subBaseName = getUStringX(UStringInfo(thisBaseName)+1,
												  thisBaseName->length - 1);
				CLASS elemClass =
					change_Name_to_CLASS(packageName, subBaseName,
										  GCT_ARRAY_CLASS);
				clazz->u.elemClass = elemClass;

				/* Now "recurse" on elemClass, initialized it, also, if
				 * necessary */
				clazz = (ARRAY_CLASS)elemClass;
				depth--;
				continue;
			}
		}
	}
	return result;
}

/*=========================================================================
 * FUNCTION:      getClass()
 * TYPE:      public instance-level operation on runtime classes
 * OVERVIEW:      Find a class with the given name, loading the class
 *        if necessary.
 * INTERFACE:
 *   parameters:  class name as a string
 *   returns:     class pointer or NIL if not found
 *
 * NOTE:  This operation may load new classes into the system.
 *        Since class loading may necessitate the execution
 *        of class initialization methods (<clinit> methods),
 *        hence requiring the invocation of the bytecode interpreter,
 *        we must be careful not to mess the native (C/C++) call
 *        stack.
 *=======================================================================*/


CLASS
getClass(const char *name)
{
    CLASS clazz = getRawClass(name, strlen(name));
    if (!IS_ARRAY_CLASS(clazz)) {
        if (((INSTANCE_CLASS)clazz)->status == CLASS_RAW) {
            loadClassfile((INSTANCE_CLASS)clazz, FALSE);
        }
        if (((INSTANCE_CLASS)clazz)->status == CLASS_ERROR) {
            return NULL;
        }
    }
    return clazz;
}


/*=========================================================================
 * FUNCTION:      typeCodeToSignature, signatureToTypeCode
 * TYPE:      private function
 * OVERVIEW:      Converts signature characters to the corresponding
 *          type code small integer, and back.
 *=======================================================================*/


static char
typeCodeToSignature(char typeCode) {
    switch(typeCode) {
        case T_CHAR :    return 'C';
        case T_BYTE :    return 'B';
        case T_BOOLEAN : return 'Z';
        case T_FLOAT :   return 'F';
        case T_DOUBLE :  return 'D';
        case T_SHORT :   return 'S';
        case T_INT :     return 'I';
        case T_LONG :    return 'J';
        case T_VOID:     return 'V';
        case T_CLASS:    return 'L';
        default :    fatalVMError("unexpected type code"); return 0;
    }
}

static char
signatureToTypeCode(char signature) {
    switch(signature) {
        case 'C' : return T_CHAR    ;
        case 'B' : return T_BYTE    ;
        case 'Z' : return T_BOOLEAN ;
        case 'F' : return T_FLOAT   ;
        case 'D' : return T_DOUBLE  ;
        case 'S' : return T_SHORT   ;
        case 'I' : return T_INT     ;
        case 'J' : return T_LONG    ;
        case 'V' : return T_VOID    ;
        case 'L' : return T_CLASS   ;
        default : fatalVMError("unexpected type code"); return 0;
    }
}


/*=========================================================================
 * FUNCTION:      getPrimitiveArrayClass(), getObjectArrayClass
 * TYPE:      public instance-level operation on runtime classes
 * OVERVIEW:      Creates an array class whose elements are the specified type
 *        getPrimitiveArrayClass expects a typecode
 *        getObjectArrayClass expects an array or instance Class
 * INTERFACE:
 *   parameters:  see above
 *   returns:     class pointer
 *=======================================================================*/

ARRAY_CLASS
getPrimitiveArrayClass(int typeCode)
{
    return getArrayClass(1, NULL, typeCodeToSignature(typeCode));
}



ARRAY_CLASS
getObjectArrayClass(CLASS clazz) {
    CLASS subClass = clazz;
    int depth = 1;
    for (;;) {
        if (!IS_ARRAY_CLASS(subClass)) {
            /* subClass is a base type */
            return getArrayClass(depth, (INSTANCE_CLASS)subClass, '\0');
        } else if (((ARRAY_CLASS)subClass)->gcType == GCT_ARRAY) {
            /* subClass is an array of primitives of depth 1. */
            int typeCode  = ((ARRAY_CLASS)subClass)->u.primType;
            char signCode = typeCodeToSignature(typeCode);
            return getArrayClass(depth + 1, NULL, signCode);
        } else {
            /* iterate down one level */
            subClass = ((ARRAY_CLASS)subClass)->u.elemClass;
            depth++;
        }
    }
}

/*=========================================================================
 * FUNCTION:      getClassName, getClassName_inBuffer
 * TYPE:      public instance-level operation on runtime classes
 * OVERVIEW:      Gets the name of a class
 *
 * INTERFACE:
 *   parameters:  clazz:       Any class
 *                str_buffer:  Where to put the result
 *
 *   returns:     getClassName() returns a pointer to the result
 *                getClassName_inBuffer() returns a pointer to the NULL
 *                  at the end of the result.  The result is in the
 *                 passed buffer.
 *
 * DANGER:  getClassName_inBuffer() does not return what you expect.
 *
 * DANGER:  getClassName() allocates a byte array, and marks this array
 *      as a temporary root.  This should only be called inside
 *      START_TEMPORARY_ROOTS ... END_TEMPORARY_ROOTS
 *=======================================================================*/

char *
getClassName(CLASS clazz) {
    char *endBuffer = getClassName_inBuffer(clazz, str_buffer);
    return bufferToByteArray(str_buffer, endBuffer - str_buffer);
}

char*                   /* returns pointer to '\0' at end of resultBuffer */
getClassName_inBuffer(CLASS clazz, char *resultBuffer) {
    UString UPackageName = clazz->packageName;
    UString UBaseName    = clazz->baseName;
    char *baseName = UStringInfo(UBaseName);
    char *from = baseName;      /* pointer into baseName */
    char *to = resultBuffer;    /* used for creating the output */
    int  fromLength = UBaseName->length;
    bool_t isArrayOfObject;

    /* The result should have exactly as many "["s as the baseName has at
     * its beginning.  Then skip over that part in both the input and the
     * output */
    for (from = baseName; *from == '['; from++, fromLength--) {
        *to++ = '[';
    }
    /* We're an array of objects if we've actually written something already
     * to "from", and more than one character remains
     */
    isArrayOfObject = (from != baseName && fromLength != 1);

    if (isArrayOfObject) {
        *to++ = 'L';
    }
    /* Now print out the package name, followed by the rest of the baseName */
    if (UPackageName != NULL) {
        int packageLength = UPackageName->length;
        memcpy(to, UStringInfo(UPackageName), packageLength);
        to += packageLength;
        *to++ = '/';
    }
    if (isArrayOfObject) {
        memcpy(to, from + 1, fromLength - 2); /* skip L and ; */
        to += (fromLength - 2);
        *to++ = ';';
    } else {
        memcpy(to, from, fromLength);
        to += fromLength;
    }
    *to = '\0';
    return to;
}


/*=========================================================================
 * FUNCTION:      printInterfaceTable()
 * TYPE:          public instance-level operation on runtime classes
 * OVERVIEW:      Print the contents of the interface table of the
 *                given runtime class structure for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to the interface table
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
static void
printInterfaceTable(INSTANCE_CLASS thisClass)
{
    unsigned short* ifaceTable = thisClass->ifaceTable;
    int tableLength = *ifaceTable;
    int i;

    for (i = 1; i <= tableLength; i++) {
        CLASS clazz = thisClass->constPool->entries[ifaceTable[i]].clazz;
        START_TEMPORARY_ROOTS
            fprintf(stdout,"%d: %s\n", i, className(clazz));
        END_TEMPORARY_ROOTS
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printClass()
 * TYPE:          public instance-level operation on runtime classes
 * OVERVIEW:      Print the contents of a runtime class structure
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  class pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printClassname(CLASS thisClass)
{
    START_TEMPORARY_ROOTS
        fprintf(stdout,"Class '%s'\n", className(thisClass));
    END_TEMPORARY_ROOTS
}

void printClass(CLASS thisClass)
{

  START_TEMPORARY_ROOTS
    fprintf(stdout,"Class '%s' (%lx)\n",
            className(thisClass), (long)thisClass);
    fprintf(stdout,"Access flags....: %x\n", thisClass->accessFlags);
    if (!IS_ARRAY_CLASS(thisClass)) {
        INSTANCE_CLASS thisIClass = (INSTANCE_CLASS)thisClass;
        INSTANCE_CLASS superClass = thisIClass->superClass;
        fprintf(stdout,"Loading status..: %d\n", thisIClass->status);
        if (thisIClass->status == CLASS_RAW) {
            return;
        }
        if (superClass) {
            fprintf(stdout,"Superclass '%s' (%lx)\n",
                    className(superClass), (long)superClass);
        }

        fprintf(stdout,"Instance size...: %d\n", thisIClass->instSize);

        fprintf(stdout,"Constant pool at %lx contains %ld entries\n",
                (long)thisIClass->constPool,
                CONSTANTPOOL_LENGTH(thisIClass->constPool));
        printConstantPool(thisIClass->constPool);

        if (thisIClass->fieldTable != NULL) {
            fprintf(stdout,"Field table at %lx contains %ld entries\n",
                    (long)thisIClass->fieldTable,
                    thisIClass->fieldTable->length);
            printFieldTable(thisIClass->fieldTable);
        }

        fprintf(stdout,"Method table at %lx contains %ld entries\n",
                (long)thisIClass->methodTable, thisIClass->methodTable->length);
        printMethodTable(thisIClass->methodTable);

        if (thisIClass->ifaceTable != NULL) {
            fprintf(stdout,"Interface table at %lx contains %ld entries\n",
                    (long)thisIClass->ifaceTable,
                    (long)*thisIClass->ifaceTable);
            printInterfaceTable(thisIClass);
        }
    } else {
        ARRAY_CLASS thisAClass = (ARRAY_CLASS)thisClass;
        fprintf(stdout, "GC type: %s\n",
                thisAClass->gcType == GCT_ARRAY
                      ? "GCT_ARRAY" : "GCT_OBJECTARRAY");
        fprintf(stdout, "itemSize: %d\n", thisAClass->itemSize);
    }
  END_TEMPORARY_ROOTS
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printAllClasses()
 * TYPE:      public instance-level operation on runtime classes
 * OVERVIEW:      Print the contents of all the runtime class structures
 *        currently loaded in the system for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE

void printAllClasses()
{
    int count = 0;
    FOR_ALL_CLASSES(clazz)
        printClass(clazz);
        putchar('\n');
        count++;
    END_FOR_ALL_CLASSES
    fprintf(stdout, "\nTotal number of classes in the system: %d\n", count);
}
#endif /*  INCLUDEDEBUGCODE */



/*=========================================================================
 * FUNCTION:      getClassSize()
 * TYPE:      public operation on runtime classes
 * OVERVIEW:      Return the size of the runtime class structure
 * INTERFACE:
 *   parameters:  class pointer
 *   returns:     the size of the runtime class structure
 *=======================================================================*/

#if ENABLEPROFILING
long
getClassSize(CLASS thisClass)
{
    if (ROMIZING && !IS_ROM_CLASS(thisClass)) {
        return 0;
    } else if (!IS_ARRAY_CLASS(thisClass)) {
        INSTANCE_CLASS thisIClass = (INSTANCE_CLASS)thisClass;
        long size = SIZEOF_INSTANCE_CLASS;

        /*  Add the size of the constant pool (doesn't take into account */
        /*  savings gained by CSP) */
        if (thisIClass->constPool) {
            size += getPoolSize(thisIClass->constPool);
        }

        /*  Add the size of the method table */
        if (thisIClass->methodTable) {
            size += getMethodTableSize(thisIClass->methodTable);
        }

        /*  Add the size of the field table */
        if (thisIClass->fieldTable) {
            size += thisIClass->fieldTable->length * SIZEOF_FIELD + 1;
        }
        return size;
    } else {
        return SIZEOF_ARRAY_CLASS;
    }
}
#endif /*  ENABLEPROFILING  || TRACECLASSLOADINGVERBOSE */

/*=========================================================================
 * FUNCTION:      countClasses()
 * TYPE:      public operation on runtime classes
 * OVERVIEW:      Count the number of classes in the system.
 * INTERFACE:
 *   parameters:  a variable parameter used to return the total
 *        amount of memory used by loaded classes
 *   returns:     the number of classes
 *=======================================================================*/

#if ENABLEPROFILING
int countClasses(long* memoryUsed)
{
    int count = 0;
    int memory = 0;
    FOR_ALL_CLASSES(clazz)
        memory += getClassSize(clazz);
        count++;
    END_FOR_ALL_CLASSES
    *memoryUsed = memory;
    return count;
}

#endif /*  ENABLEPROFILING */

/*=========================================================================
 * Operations on instances
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      instantiate()
 * TYPE:          constructor
 * OVERVIEW:      Create an instance of the given class.
 * INTERFACE:
 *   parameters:  class pointer
 *   returns:     pointer to object instance
 *
 * NOTE:          This operation only allocates the space for the
 *                instance and initializes its instance variables to zero.
 *                Actual constructor for initializing the variables
 *                must be invoked separately.
 *=======================================================================*/

INSTANCE instantiate(INSTANCE_CLASS thisClass)
{
    int size = SIZEOF_INSTANCE(thisClass->instSize);
    INSTANCE newInstance = (INSTANCE)mallocHeapObject(size, GCT_INSTANCE);
    
    if (newInstance != NULL) { 
	memset(newInstance, 0, size << log2CELL);
	/*  initialize the class pointer (zeroeth field in the instance) */
	newInstance->ofClass = thisClass;
    } else { 
	throwException(OutOfMemoryObject);
    }
    return newInstance;
}

/*=========================================================================
 * FUNCTION:      getClassOf()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an object instance, get the class of the object
 * INTERFACE:
 *   parameters:  instance pointer
 *   returns:     class pointer
 *=======================================================================*/

CLASS getClassOf(OBJECT thisObject)
{
    return thisObject->ofClass;
}

/*=========================================================================
 * FUNCTION:      isInstanceOf()
 * TYPE:          public instance-level operation on runtime objects
 * OVERVIEW:      Check if the given object is an instance of a class
 *                that is a proper subclass of the given class.
 *
 * INTERFACE:
 *   parameters:  instance pointer, class pointer
 *   returns:     boolean
 *=======================================================================*/


bool_t
isInstanceOf(OBJECT object, CLASS givenClass)
{
    if (object == NULL) {
        return TRUE;
    } else {
        bool_t result;
        START_TEMPORARY_ROOT(object)
            result = isAssignableTo(object->ofClass, givenClass);
        END_TEMPORARY_ROOT
        return result;
    }
}

static bool_t
implementsInterface(INSTANCE_CLASS thisClass, INSTANCE_CLASS interface);


/*=========================================================================
 * FUNCTION:      isInstanceOf()
 * TYPE:          public instance-level operation on runtime objects
 * OVERVIEW:      Determines is of object of type "fromClass" can be assigned
 *                to a slot of type "toClass".  This is a generalization
 *                of "isSubclassOf" that also handles arrays and interfaces.
 * INTERFACE:
 *   parameters:  fromClass:  type of object
 *                toClass:    type of slot into which to assign the object
 *   returns:     boolean
 *=======================================================================*/

bool_t
isAssignableTo(CLASS fromClass, CLASS toClass) {
    for (;;) {
        /* You might be wondering how fromClass could ever be an interface,
         * since objects of never of type "fromClass".  This can only happen
         * on the second or subsequent iteration through the loop, when
         * fromClass was originally of type someInterface[]
         */
        if ((fromClass == toClass) || (toClass == (CLASS)JavaLangObject)) {
            return TRUE;
        }
        if (!IS_ARRAY_CLASS(toClass)
            && (((INSTANCE_CLASS)toClass)->status == CLASS_RAW)){
            loadClassfile((INSTANCE_CLASS)toClass, TRUE);
        }
        if (IS_ARRAY_CLASS(toClass)) {
            /* Only a compatible array can be assigned to an array */
            if (!IS_ARRAY_CLASS(fromClass)) {
                /* a non-array class can't be a subclass of an array class */
                return FALSE;
            } else {
                /* assigning an array to an array */
                int fromType = ((ARRAY_CLASS)fromClass)->gcType;
                int toType = ((ARRAY_CLASS)toClass)->gcType;
                /* fromType, toType are either GCT_OBJECTARRAY or GCT_ARRAY */
                if (toType != fromType) {
                    /* An array of primitives, and an array of non-primitives */
                    return FALSE;
                } else if (toType == GCT_ARRAY) {
                    /* Two arrays of primitive types */
                    return ((ARRAY_CLASS)fromClass)->u.primType ==
                           ((ARRAY_CLASS)toClass)->u.primType;
                } else {
                    /* Two object array types.  Look at there element
                     * types and recurse. */
                    fromClass = ((ARRAY_CLASS)fromClass)->u.elemClass;
                    toClass = ((ARRAY_CLASS)toClass)->u.elemClass;
                    continue;
                }
            }
        } else if ((toClass->accessFlags & ACC_INTERFACE) != 0) {
            /* Assigning to an interface.  fromClass must be a non-array that
             * implements the interface.
             * NB: If we ever have Cloneable or Serializable, we must fix
             * this, since arrays do implement those. */
            return (!(IS_ARRAY_CLASS(fromClass)) &&
                    implementsInterface((INSTANCE_CLASS)fromClass,
                                        (INSTANCE_CLASS)toClass));
        } else {
            /* Assigning to a non-array, non-interface, honest-to-god class,
             * that's not java.lang.Object */
            if (IS_ARRAY_CLASS(fromClass) ||
                    ((fromClass->accessFlags & ACC_INTERFACE) != 0)) {
                /* arrays and interfaces can only be assigned to
                 * java.lang.Object.  We already know that's it's not */
                return FALSE;
            } else {
                /* fromClass is also a non-array, non-interface class.  We
                 * need to check to see if fromClass is a subclass of toClass.
                 * We've know fromClass != toClass from the shortcut test
                 * way above
                 */
                INSTANCE_CLASS fromIClass = (INSTANCE_CLASS)fromClass;
                INSTANCE_CLASS toIClass = (INSTANCE_CLASS)toClass;
                while (fromIClass != JavaLangObject) {
                    if (!IS_ARRAY_CLASS(fromIClass)
                        && (((INSTANCE_CLASS)fromIClass)->status == CLASS_RAW)){
                        loadClassfile((INSTANCE_CLASS)fromIClass, TRUE);
                    }
                    fromIClass = fromIClass->superClass;
                    if (fromIClass == toIClass) {
                        return TRUE;
                    }
                }
                return FALSE;
            }
        }
    }
}

bool_t
implementsInterface(INSTANCE_CLASS thisClass, INSTANCE_CLASS interface)
{
    unsigned short *ifaceTable;
    if (thisClass == interface) {
        return TRUE;
    }
    if (!IS_ARRAY_CLASS(thisClass)
        && (((INSTANCE_CLASS)thisClass)->status == CLASS_RAW)){
        loadClassfile((INSTANCE_CLASS)thisClass, TRUE);
    }
    for (;;) {
        ifaceTable = thisClass->ifaceTable;
        if (ifaceTable != NULL) {
            int tableLength = ifaceTable[0];
            int i;
            for (i = 1; i <= tableLength; i++) {
                INSTANCE_CLASS ifaceClass =
                    (INSTANCE_CLASS)resolveClassReference(thisClass->constPool,
                                                          ifaceTable[i],
							  thisClass);
                if (implementsInterface(ifaceClass, interface)) {
                    return TRUE;
                }
            }
        }
        if (thisClass == JavaLangObject) {
            return FALSE;
        }
        thisClass = thisClass->superClass;
    }
}



/*=========================================================================
 * FUNCTION:      printInstance()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of an object instance
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  instance pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printInstance(INSTANCE thisInstance)
{
    INSTANCE_CLASS thisClass = thisInstance->ofClass;

    int   instSize  = thisClass->instSize;
    int   i;

    START_TEMPORARY_ROOTS
        fprintf(stdout, "Instance %lx contents:\n", (long)thisInstance);
        fprintf(stdout, "Class: %lx (%s)\n",
                (long)thisClass, className(thisClass));
        for (i = 0; i < instSize; i++) {
            long contents = thisInstance->data[i].cell;
            fprintf(stdout,"Field %d: %lx (%ld)\n", i, contents, contents);
        }
    END_TEMPORARY_ROOTS
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Operations on array instances
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      instantiateArray()
 * TYPE:          constructor
 * OVERVIEW:      Create an array of given type and size.
 * INTERFACE:
 *   parameters:  array class, array size
 *   returns:     pointer to array object instance
 * NOTE:          This operation only allocates the space for the
 *                array and initializes its slots to zero.
 *                Actual constructor for initializing the array
 *                must be invoked separately.
 *=======================================================================*/

ARRAY instantiateArray(ARRAY_CLASS arrayClass, long length)
{
    if (length < 0) {     
	raiseException(NegativeArraySizeException);
	return NULL;
    } else if (length > 0x1000000) {
        /* Don't even try.  dataSize above may have overflowed */
	throwException(OutOfMemoryObject);
	return NULL;
    } else { 
	/* Does this array contain pointers or not? */
	GCT_ObjectType gctype = arrayClass->gcType;
	/* How big is each item */
	int slotSize = arrayClass->itemSize;
	/* What's the total data size */
	long dataSize = (length * slotSize + (CELL - 1)) >> log2CELL;
	/* What's the total size */
	int arraySize = SIZEOF_ARRAY(dataSize);
	ARRAY newArray;

	newArray = (ARRAY)mallocHeapObject(arraySize, gctype);
	if (newArray == NULL) { 
	    throwException(OutOfMemoryObject);
	} else { 
	    memset(newArray, 0, arraySize << log2CELL);
	    newArray->ofClass   = arrayClass;
	    newArray->length    = length;
	}
	return newArray;
    }
}

/*=========================================================================
 * FUNCTION:      instantiateMultiArray()
 * TYPE:      constructor
 * OVERVIEW:      Create an n-dimensional array (recursively).
 * INTERFACE:
 *   parameters:  array type index, integer array containing the
 *        different dimensions, number of dimensions
 *   returns:     pointer to array object instance
 *=======================================================================*/

ARRAY
instantiateMultiArray(ARRAY_CLASS arrayClass, long *lengths, int dimensions)
{
    int currDepth;
    ARRAY rootArray;
    ARRAY prevArraySet,   currArraySet;
    int   prevArrayWidth, currArrayWidth;
    CLASS currClass;

    for (currDepth = 0; currDepth < dimensions; currDepth++) {
        int dim = lengths[currDepth];
        if (dim < 0) {
            raiseException(NegativeArraySizeException);
            return NIL;
        }
    }

    rootArray = instantiateArray(getArrayClass(1, JavaLangObject, 0), 1);
    if (rootArray == NULL) { 
	/* Out of memory exception has already been thrown */
	return NULL;
    }
    

    currArraySet = rootArray;
    currArrayWidth = 1;

    START_TEMPORARY_ROOT(rootArray)
        /* We have rootArray acting as the previous level to level 0.
         * It has only one slot, and the allocator has already set it to 0,
         * so it will appear as a linked list with one node alone.
         *
         * Now go through all the lengths and do the work
         */
        for (currDepth = 0, currClass = (CLASS)arrayClass;
                 ;
             currDepth++,   currClass = ((ARRAY_CLASS)currClass)->u.elemClass) {
            bool_t lastIteration;
            prevArraySet   = currArraySet;
            prevArrayWidth = currArrayWidth;

            currArraySet   = NULL;
            currArrayWidth = lengths[currDepth];
            lastIteration = (currDepth == dimensions - 1 || currArrayWidth == 0);

            /* Go through each of the previous level's nodes, and allocate
             * and link the current nodes, if needed.  Fill them in the slots
             * of the previous level's nodes.
             */
            do {
                int index;
                ARRAY prevArray;

                /* Get an item from the chain, and advance the chain. */
                prevArray = prevArraySet;
                prevArraySet = (ARRAY)prevArraySet->data[0].cellp;

                /* Fill in each index of prevArray */
                for (index = 0; index < prevArrayWidth; index++) {
                    /* Allocate the node, and fill in the parent's slot. */
                    ARRAY currArray =
                        instantiateArray((ARRAY_CLASS)currClass, currArrayWidth);
		    if (currArray == NULL) { 
			/* OutOfMemoryError has already been thrown */
			rootArray->data[0].cellp = NULL;
			goto done;
		    }

                    prevArray->data[index].cellp = (cell *)currArray;
                    if (!lastIteration) {
                        /* Link it in, for the next time through the loop. */
                        currArray->data[0].cellp = (cell *)currArraySet;
                        currArraySet = currArray;
                    }
                }
            } while (prevArraySet != NULL);

            if (lastIteration) {
                break;
            }
        }
 done:
    END_TEMPORARY_ROOT
    return (ARRAY)rootArray->data[0].cellp;
}

/*=========================================================================
 * FUNCTION:      arrayItemSize()
 * TYPE:          private helper operation for array manipulation
 * OVERVIEW:      Get the item size of the array on the basis of its type.
 * INTERFACE:
 *   parameters:  array type index
 *   returns:     array item size
 *=======================================================================*/

long arrayItemSize(int arrayType)
{
    long itemSize;

    switch (arrayType) {
    case T_BOOLEAN: case T_BYTE:
        itemSize = 1;
        break;
    case T_CHAR: case T_SHORT:
        itemSize = SHORT;
        break;
    case T_INT: case T_FLOAT:
        itemSize = CELL;
        break;
    case T_DOUBLE: case T_LONG:
        itemSize = CELL*2;
        break;
    default:
        /*  Array of reference type (T_REFERENCE or class pointer) */
        itemSize = CELL;
        break;
    }
    return itemSize;
}

/*=========================================================================
 * FUNCTION:      printArray()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of an array object instance
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  array object pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE

void printArray(ARRAY thisArray)
{
    ARRAY_CLASS thisClass = thisArray->ofClass;
    if (!IS_ARRAY_CLASS(thisClass)) {
    fprintf(stdout,"Object %lx is not an array\n", (long)thisArray);
    return;
    }

    START_TEMPORARY_ROOTS
        fprintf(stdout,"Array %lx contents:\n", (long)thisArray);
    fprintf(stdout,"Array type: %s \n", className(thisClass));
        fprintf(stdout,"Length: %ld\n", thisArray->length);
    END_TEMPORARY_ROOTS

    if (thisClass->gcType == GCT_ARRAY) {
        int length, i;
        if (thisClass->u.primType == T_CHAR) {
            SHORTARRAY shortArray = (SHORTARRAY)thisArray;
            fprintf(stdout, "Contents....: '\n");
            length = (thisArray->length < 40) ? thisArray->length : 40;
            for (i = 0; i < length; i++) {
                putchar((char)shortArray->sdata[i]);
            }
            fprintf(stdout,"'\n");
        }
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Operations on string instances
 *=======================================================================*/

static SHORTARRAY createCharArray(const char* utf8string, int utf8length,
                      int *unicodelengthP)
{
    int unicodelength = 0;
    int i;
    SHORTARRAY newArray;
    const char *p = utf8string;
    int size;

    while (p < utf8string + utf8length) {
        utf2unicode(&p);
        unicodelength++;
    }

    size = (unicodelength * sizeof(short) + CELL - 1) >> log2CELL;
    /*  Allocate room for the character array; */
    newArray = (SHORTARRAY)callocObject(SIZEOF_ARRAY(size), GCT_ARRAY);
    newArray->ofClass = JavaLangCharArray;
    newArray->length  = unicodelength;

    /*  Initialize the array with string contents; */
    p = utf8string;
    i = 0;
    while (p < utf8string + utf8length) {
        newArray->sdata[i++] = utf2unicode(&p);
    }
    *unicodelengthP = unicodelength;
    return newArray;
}

/*=========================================================================
 * FUNCTION:      instantiateString()
 * TYPE:           constructor
 * OVERVIEW:      Create an initialized Java-level string object.
 * INTERFACE:
 *   parameters:  UTF8 String containing the value of the new string object
 *                number of bytes in the UTF8 string
 *   returns:     pointer to array object instance
 * NOTE:          String arrays are not intended to be manipulated
 *                directly; they are manipulated from within instances
 *                of 'java.lang.String'.
 *=======================================================================*/

STRING_INSTANCE
instantiateString(const char* utfstring, int utflength)
{
    STRING_INSTANCE newObject;
    int unicodelength;

    /* Instantiate a java.lang.String object.  This function may be
     * called before JavaLangString has been loaded, so we can't
     * just call instantiate(JavaLangString);
     */
    newObject = (STRING_INSTANCE)callocObject(SIZEOF_STRING_INSTANCE,
                                              GCT_INSTANCE);
    newObject->ofClass = JavaLangString;
    START_TEMPORARY_ROOT(newObject)
        /*  The first slot should contain a character array */
        newObject->array = createCharArray(utfstring, utflength, &unicodelength);
        newObject->offset = 0;
        newObject->length = unicodelength;
    END_TEMPORARY_ROOT
    return newObject;
}

/*=========================================================================
 * FUNCTION:      getStringContents()
 * TYPE:          internal operation on string objects
 * OVERVIEW:      Get the contents of a string object in C/C++
 *                string format.
 * INTERFACE:
 *   parameters:  String object pointer
 *   returns:     char* to the string in C/C++ format
 *
 * NOTE:          This operation uses an internal string buffer
 *                that is shared and has a fixed length.  Thus,
 *                the contents of the returned string may (will)
 *                change over time.
 *=======================================================================*/

char* getStringContents(STRING_INSTANCE string)
{
   return getStringContentsSafely(string, str_buffer, STRINGBUFFERSIZE);
}


/*=========================================================================
 * FUNCTION:      getStringContentsSafely()
 * TYPE:      internal operation on string objects
 * OVERVIEW:      Get the contents of a string object in C/C++
 *        string format.
 * INTERFACE:
 *   parameters:  String object pointer
 *   returns:     char* to the string in C/C++ format
 *=======================================================================*/

char *getStringContentsSafely(STRING_INSTANCE string, char *buf, int lth)
{
    /*  Get the internal unicode array containing the string */
    SHORTARRAY thisArray = string->array;
    int        offset    = string->offset;
    int        length    = string->length;
    int        i;

    if ((length+1) > lth) {
	fatalError("Buffer overflow getStringContentsSafely");
    }

    /*  Copy contents of the unicode array to the C/C++ string */
    for (i = 0; i < length; i++) {
	buf[i] = (char)thisArray->sdata[offset + i];
    }

    /*  Terminating C/C++ string with zero */
    buf[length] = 0;
    return buf;
}



/*=========================================================================
 * FUNCTION:      bufferToByteArray
 * TYPE:          internal operation on string objects
 * OVERVIEW:      Converts the string pointed to by "buffer" into
 *                a "malloc"ed string.
 * INTERFACE:
 *   parameters:  String object pointer
 *                length
 *   returns:     A malloc'ed byte array
 *
 * DANGER:  This function should only be called within the context of
 *          a START_TEMPORARY_ROOTS .. END_TEMPORARY_ROOTS pair.  The
 *          resulting byte array is made into a temporary root, and someone
 *          has to remove it!
 *=======================================================================*/

char *bufferToByteArray(const char *buffer, int length) {
    /* For compile time debugging.  MAKE_TEMPORARY_ROOT can normally only be
     * used >>lexically<< inside of a START_TEMPORARY_ROOTS.  This lets
     * the code compile by explicitly indicating we should be inside that macro
     * dynamically.
     */
    INDICATE_DYNAMICALLY_INSIDE_TEMPORARY_ROOTS
	char *result = mallocBytes(length + 1);
	memcpy(result, buffer, length);
	result[length] = '\0';
    MAKE_TEMPORARY_ROOT(result);
    return result;
}


#ifdef INCLUDEDEBUGCODE
void
printString(INSTANCE string)
{
    SHORTARRAY data = (SHORTARRAY)string->data[0].cellp;
    int offset = string->data[1].cell;
    int length = string->data[2].cell;
    int i;
    putchar('"');
    for (i = 0; i < length; i++) {
        putchar((char)data->sdata[offset + i]);
    }
    putchar('"');
    putchar('\n');
 }
#endif
