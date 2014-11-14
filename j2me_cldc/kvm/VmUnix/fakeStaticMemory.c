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
 * SUBSYSTEM: Memory management
 * FILE:      staticMemory.c
 * OVERVIEW:  These definitions allow KVM to emulate the special
 *            USESTATIC mode on Unix/Solaris for debugging purposes.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#if USESTATIC

/*=========================================================================
 * Definitions and variables
 *=======================================================================*/

static int devZero = 0;
static caddr_t memoryStart = 0;
static int memoryOffset;
static int pageSize;

#define MEMORY_SIZE 0x10000
#define MEMORY_SHIBBOLETH 0xCAFEBABE

/*=========================================================================
 * Functions for emulating Simonizing on Solaris
 *=======================================================================*/

static void
initialize()
{
	devZero = open("/dev/zero", O_RDONLY);
	memoryStart = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, devZero, 0);
	memoryOffset = 0;
	pageSize = getpagesize();
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
	caddr_t start = (caddr_t)staticMemory + offset;
	caddr_t end = start + size;
	
	caddr_t pageStart = (caddr_t)((long)start & ~(pageSize - 1));
	caddr_t pageEnd = (caddr_t)(((long)end + pageSize - 1) & ~(pageSize - 1));

	if ((caddr_t)staticMemory < memoryStart 
		|| (caddr_t)staticMemory >= (memoryStart + MEMORY_SIZE)) {
		fatalError("Bad modification??");
	}
	if ( ((unsigned long *)staticMemory)[-1] != MEMORY_SHIBBOLETH) { 
		fprintf(stderr, "Bad pointer passed to modifyStaticMemory");
	}
	
	if (mprotect(pageStart, pageEnd - pageStart, PROT_READ | PROT_WRITE) != 0) {
		fprintf(stderr, "Unable to set protection");
	}
	memcpy(start, newVal, size);
	mprotect(pageStart, pageEnd - pageStart, PROT_READ);
}

static void
insertShibboleth(void *staticMemory)
{
	long start = (long)staticMemory & ~(pageSize - 1);
	mprotect((caddr_t)start, pageSize, PROT_READ | PROT_WRITE);
	*(unsigned long *)staticMemory = MEMORY_SHIBBOLETH;
	mprotect((caddr_t)start, pageSize, PROT_READ);
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
mallocStaticBytes(int size) { 
	cell* newChunk, *newObject;
	int actualSize;
	if (memoryStart == 0) { 
		initialize();
	}
	actualSize = (size + sizeof(cell *) + sizeof(cell *) - 1)
		              & ~(sizeof(cell *) - 1);
				  
	newChunk = (cell *)(memoryStart + memoryOffset);
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
	munmap(memoryStart, MEMORY_SIZE);
	close(devZero);
	/*** REINIT ****/
	memoryStart = 0;

}


#endif /* USESTATIC */

