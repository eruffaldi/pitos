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
 * KVM Virtual Machine
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Internal runtime structures
 * FILE:      cache.c
 * OVERVIEW:  Inline caching support (see Cache.h).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/


#include <global.h>


/*=========================================================================
 * Everything that follows is included in the system only if
 * bytecode optimization is turned on
 *=======================================================================*/

#if ENABLEFASTBYTECODES

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

/*  The master inline cache in the system (see Cache.h) */
ICACHE InlineCache;

/*  Index of the next inline cache entry to be used */
int InlineCachePointer;

/*  Flag telling whether inline cache area is full or not */
int InlineCacheAreaFull;

static void releaseInlineCacheEntry(int index);



/*=========================================================================
 * Constructor/destructors for (re)initializing inline caching
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      InitializeInlineCaching()
 * TYPE:          constructor
 * OVERVIEW:      Create the master inline cache area of the system and
 *                initialize it properly.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void 
InitializeInlineCaching(void)
{
    /*  Align the cache area so that accessing static variables is safe 
     *  regardless of the alignment settings of the compiler 
     */
    InlineCache = 
        (ICACHE)mallocObject(SIZEOF_ICACHE*INLINECACHESIZE+1, GCT_INLINECACHE);
    InlineCachePointer  = 0;
    InlineCacheAreaFull = FALSE;

    makeGlobalRoot((cell **)&InlineCache);
}

/*=========================================================================
 * FUNCTION:      FinalizeInlineCaching()
 * TYPE:          destructor (reconstructor, actually)
 * OVERVIEW:      Flush all the existing inline cache entries from the
 *                master inline cache area, patching all the affected
 *                methods with original code.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void 
FinalizeInlineCaching(void)
{ 
    int last = InlineCacheAreaFull ? INLINECACHESIZE : InlineCachePointer;
    while (--last >= 0) { 
        releaseInlineCacheEntry(last);
    }
    InlineCachePointer = 0;
    InlineCacheAreaFull = FALSE;
}


/*=========================================================================
 * Constructors/destructors for individual icache entries
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      releaseInlineCacheEntry()
 * TYPE:          private destructor
 * OVERVIEW:      Release the requested inline cache entry and patch
 *                code with original bytecode and parameters.
 * INTERFACE:
 *   parameters:  inline cache entry index
 *   returns:     <nothing>
 * NOTE:          This operation should only be called internally
 *                by createICacheEntry() below
 *=======================================================================*/

static void 
releaseInlineCacheEntry(int index)
{
    ICACHE thisICache = &InlineCache[index];

    /*  Read the pointer to the code location  */
    /*  referring to this inline cache entry */
    BYTE* codeLoc = (BYTE*)thisICache->codeLoc;

    /*  Restore original bytecodes */
    *codeLoc = thisICache->origInst;
    putShort(codeLoc+1, thisICache->origParam);
}

/*=========================================================================
 * FUNCTION:      createInlineCacheEntry()
 * TYPE:          constructor
 * OVERVIEW:      Create or reallocate an inline cache entry,
 *                storing the given parameter and the original
 *                bytecode sequence in the cache.
 * INTERFACE:
 *   parameters:  user-supplied contents, location of the original
 *                bytecode sequence
 *   returns:     index of the icache entry
 *=======================================================================*/

int 
createInlineCacheEntry(cell* contents, BYTE* originalCode)
{
    ICACHE thisICache;
    int    index;
    
    /*  Check first if inline cache is already full */
    if (InlineCacheAreaFull) {
        releaseInlineCacheEntry(InlineCachePointer);
    }

    /*  Allocate new entry / reallocate old one */
    thisICache = &InlineCache[InlineCachePointer];
    index      = InlineCachePointer++;

    /*  Check whether the icache area is full now */
    if (InlineCachePointer == INLINECACHESIZE) {
        InlineCacheAreaFull = TRUE;
        InlineCachePointer  = 0;
    }

    /*  Initialize icache values */
    thisICache->contents = contents;
    thisICache->codeLoc = originalCode;
    thisICache->origInst = *originalCode;
    thisICache->origParam = getShort(originalCode+1);

    return index;
}

/*=========================================================================
 * Operations on individual icache entries
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getInlineCache()
 * TYPE:          public accessor function
 * OVERVIEW:      Given an index, get the address of the corresponding
 *                inline cache entry in the master inline cache area.
 * INTERFACE:
 *   parameters:  inline cache entry index
 *   returns:     inline cache entry address
 *=======================================================================*/

ICACHE getInlineCache(int index)
{
    return &InlineCache[index];
}

/*=========================================================================
 * End of conditional compilation (ENABLEFASTBYTECODES)
 *=======================================================================*/

#endif  /*  ENABLEFASTBYTECODES */
