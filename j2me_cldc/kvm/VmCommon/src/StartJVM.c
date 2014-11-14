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
 * KVM
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Main Java program
 * FILE:      StartJVM.c
 * OVERVIEW:  System initialization and start of JVM
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>


/*=========================================================================
 * Shared string buffer
 *=======================================================================*/

char str_buffer[STRINGBUFFERSIZE]; /*  Shared string buffer */

/*=========================================================================
 * Functions
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      readCommandLineArguments()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Read the extra command line arguments provided
 *                by the user and construct a string array containing
 *                the arguments. Note: before executing 'main' the
 *                string array must be pushed onto the operand stack.
 * INTERFACE:
 *   parameters:  command line parameters
 *   returns:     string array containing the command line arguments
 *=======================================================================*/

static ARRAY 
readCommandLineArguments(int argc, char* argv[])
{
    /*  Get the number of command line arguments */
    int numberOfArguments = argc;
    int argCount = (numberOfArguments > 0) ? numberOfArguments : 0;
    int i;

    /*  Create a string array for the arguments and store */
    /*  the array in the global variable CommandLineArguments */
    ARRAY_CLASS arrayClass = getArrayClass(1, JavaLangString, 0);
    ARRAY stringArray = instantiateArray(arrayClass, argCount);

    START_TEMPORARY_ROOT(stringArray)
	for (i = 0; i < numberOfArguments; i++) {
	    stringArray->data[i].cellp = 
		(cell *)instantiateString(argv[i], strlen(argv[i])); 
	}
    END_TEMPORARY_ROOT
    return stringArray;
}


int KVM_Start(int argc, char* argv[])
{
    INSTANCE_CLASS mainClass;
    METHOD thisMethod;
    volatile int returnValue = 0; /* needed to make compiler happy */
    ERROR_TRY {
	CreateROMImage();		/* if ROMIZING and RELOCATABLE_ROM */

	InitializeNativeCode();
	InitializeVM();
#if INCLUDE_ALL_CLASSES
	InitializeEvents();
#endif
	/*	Since we need to allocate memory for command line arguments */
	/*	we initialize the memory manager and the globals here */
	InitializeGlobals();
	InitializeProfiling();
	InitializeMemoryManagement();
	InitializeHashtables();

	/*	Initialize inline caching structures */
	InitializeInlineCaching();

#if USES_CLASSPATH
	/*	Initialize the CLASSPATH for class loading */
	InitializeClassPath();
#endif
	/*	nCreate the first execution thread and initialize the VM registers */
	InitializeThreading();
	
	/*	Load and initialize the necessary Java classes needed by the VM */
	InitializeJavaSystemClasses();

	InitializeVerifier();

	if (argc <= 0 || argv[0] == NULL) {
	    AlertUser("Must provide class name");
	    return -1;
	}
	mainClass = (INSTANCE_CLASS)getClass(replaceLetters(argv[0], '.', '/'));
	if (mainClass) {
	    ARRAY arguments = readCommandLineArguments(argc - 1, argv + 1);
	    START_TEMPORARY_ROOT(arguments)
		thisMethod = getSpecialMethod(mainClass, 
					      getNameAndTypeKey("main", 
						     "([Ljava/lang/String;)V"));
	    END_TEMPORARY_ROOT
	    if (thisMethod == NULL) {
		AlertUser("Class does not contain 'main' function.");
	    } else if ((thisMethod->accessFlags & ACC_PUBLIC) == 0) {
		AlertUser("'main' must be declared public");
	    } else {
		initThreadBehavior(CurrentThread, thisMethod, (OBJECT)mainClass);
		*(ARRAY *)&CurrentThread->stack->cells[0] = arguments;
		initializeClass(mainClass);
		/* Instances of these classes may have been created without
		 * explicitly executing the "new" instruction. Thus we have
		 * to make sure they are initialized.
		 */
		initializeClass(JavaLangSystem);
		initializeClass(JavaLangString);
		initializeClass(JavaLangThread);
		initializeClass(JavaLangClass);
		Interpret();
	    }
	} else {
	    sprintf(str_buffer, "Class %s not found", argv[0]);
	    AlertUser(str_buffer);
	}
    } ERROR_CATCH (error) {
	//returnValue = error;
    } ERROR_END_CATCH

    return returnValue;
}

void KVM_Cleanup()
{
    FinalizeVM();
    printProfileInfo();
    FinalizeInlineCaching();
    FinalizeNativeCode();
    FinalizeJavaSystemClasses();
	fprintf(stderr,"JVM: system classes finalized\n");
    FinalizeMemoryManagement();
	fprintf(stderr,"JVM: memory finalized\n");
    DestroyROMImage();
	fprintf(stderr,"JVM: exiting from KVM_Cleanup...\n");
}

/*=========================================================================
 * FUNCTION:      StartJVM
 * TYPE:          public global operation
 * OVERVIEW:      Boots up the virtual machine and executes 'main'.
 * INTERFACE:
 *   parameters:  command line arguments
 *   returns:     zero if everything went fine, non-zero otherwise.
 * NOTE:          In PalmPilot many of the normal VM initialization
 *                operations do not work yet.
 *=======================================================================*/

int StartJVM(int argc, char* argv[])
{
    int returnValue = KVM_Start(argc, argv);
    KVM_Cleanup();

    return returnValue;
}

