/*
 * @(#)fakeStaticMemory.c	1.8 00/05/02
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
 * SUBSYSTEM: Memory management
 * FILE:      staticMemory.c
 * OVERVIEW:  These definitions allow KVM to emulate the special
 *            USESTATIC mode on Windows for debugging purposes.
 *            This mode (informally known as "Simonizing") was 
 *            originally created for the Palm version of 
 *            Spotless/KVM to overcome some Palm-specific memory
 *            limitations.  When the USESTATIC mode is on, KVM will
 *            copy all the immutable runtime structures from "dynamic"
 *            memory to "static/storage" memory (which is fast to
 *            read but very slow to write to).  This allows the VM
 *            to leave more heap space for Java objects.
 *  
 * AUTHOR:    Frank Yellin
 *            Rich Berlin (replaced solaris sys calls with win32 versions)
 *
 * NOTE:      The USESTATIC mode is completely useless on the Windows
 *            and Unix versions of the KVM other than for debugging
 *            the memory system.  If your target device does not 
 *            differentiate "real" RAM from "storage" RAM, then
 *            this mode should always be turned OFF in a production
 *            release.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

#if USESTATIC

/*=========================================================================
 * Definitions and variables
 *=======================================================================*/

static void* memoryStart = 0;
static int memoryOffset;
static int pageSize;

#define MEMORY_SIZE 0x10000
#define MEMORY_SHIBBOLETH 0xCAFEBABE

/*=========================================================================
 * Functions for emulating Simonizing on Windows
 *=======================================================================*/

static void
initialize()
{
    memoryStart  = sysMapMemory(MEMORY_SIZE, &pageSize);
    memoryOffset = 0;
}

/*=========================================================================
 * FUNCTION:      modifyStaticMemory()
 * TYPE:          public global operation
 * OVERVIEW:      Modify a chunk of static memory.
 * INTERFACE:
 *   parameters:  staticMemory - a pointer to some piece of memory returned
 *                     by mallocStaticBytes
 *                ptr - A pointer into the interior of the object pointed at
 *                     by staticMemory, indicating the bytes to change
 *                newVal - The new value to place into the memory
 *                size - The number of bytes to change.
 *=======================================================================*/

void
modifyStaticMemory(void *staticMemory, int offset, void *newVal, int size)
{
    char* start = (char*)staticMemory + offset;
    char* end = start + size;

    char* pageStart = (char*) ((long)start & ~(pageSize - 1));
    char* pageEnd   = (char*) (((long)end + pageSize - 1) & ~(pageSize - 1));

    if (   staticMemory < memoryStart 
	|| (end >= (((char *)memoryStart) + MEMORY_SIZE))) {
	    fatalError("Bad modification??");
    }
    if ( ((unsigned long *)staticMemory)[-1] != MEMORY_SHIBBOLETH) { 
	    fprintf(stderr, "Bad pointer passed to modifyStaticMemory");
    }

    if (!sysProtectPage(pageStart, pageEnd - pageStart, TRUE)) {
	    fprintf(stderr, "Unable to set protection");
    }
    memcpy(start, newVal, size);
    sysProtectPage(pageStart, pageEnd - pageStart, FALSE);
}

static void
insertShibboleth(void *staticMemory)
{
    long start = (long)staticMemory & ~(pageSize - 1);
    sysProtectPage((void *)start, pageSize, TRUE);
    *(unsigned long *)staticMemory = MEMORY_SHIBBOLETH;
    sysProtectPage((void *)start, pageSize, FALSE);
}


/*=========================================================================
 * FUNCTION:      mallocStaticBytes
 * TYPE:          public global operation
 * OVERVIEW:      Allocate space from static memory
 * INTERFACE:
 *   parameters:  size - The size of the memory to allocate.
 *   returns:     the start of the staticized object
 *=======================================================================*/

void *
mallocStaticBytes(int size) 
{ 
    cell* newChunk, *newObject;
    int actualSize;
    if (memoryStart == 0) { 
	    initialize();
    }
    actualSize = (size + sizeof(cell *) + sizeof(cell *) - 1)
			  & ~(sizeof(cell *) - 1);

    newChunk = (cell *)(((char *)memoryStart) + memoryOffset);
    newObject = newChunk + 1;
    insertShibboleth(newChunk);
    memoryOffset += actualSize;
    return newObject;
}


/*=========================================================================
 * FUNCTION:      FinalizeStaticMemory()
 * TYPE:          public global operation
 * OVERVIEW:      Deallocate all static memory chunks allocated by
 *                mallocStaticBytes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void FinalizeStaticMemory()
{
    sysUnmapMemory(memoryStart, MEMORY_SIZE);
}


#endif /* USESTATIC */

