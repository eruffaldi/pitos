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
 * FILE:      garbage.c
 * OVERVIEW:  Memory manager/garbage collector for the KVM
 *            (2nd implementation; intended for use on devices with
 *            strict memory restrictions such as the Palm Organizer).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (made collection more precise)
 *=======================================================================*/

/*=========================================================================
 * For detailed explanation of the memory system, see Garbage.h
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stdio.h>

/*=========================================================================
 * Dynamic heap variables
 *=======================================================================*/

long   HeapSize;         /*  Heap size */
cell* HeapSpace;        /*  Heap bottom */
cell* HeapSpaceTop;     /*  Top of heap */

static int collectorIsRunning = FALSE; /* Flag to show when we are GCing */

int garbageCollecting(void) {
    return collectorIsRunning;
}


/*  First free chunk in the free list */
CHUNK FirstFreeChunk;

/*  A table for storing temporary root objects  */
/*  for the garbage collector */
POINTERLIST TemporaryRoots;
POINTERLIST GlobalRoots;
POINTERLIST TransientRoots;



#define DEFERRED_OBJECT_TABLE_SIZE 40

static cell *deferredObjectTable[DEFERRED_OBJECT_TABLE_SIZE];
#define endDeferredObjectTable \
       (deferredObjectTable + DEFERRED_OBJECT_TABLE_SIZE)
static cell **startDeferredObjects,  **endDeferredObjects;
static int deferredObjectCount;
static int deferredObjectTableOverflow;


static void markChildren(cell* object, cell* limit, int remainingDepth);
static void putDeferredObject(cell *c);
static cell* getDeferredObject(void);
static void initializeDeferredObjectTable(void);


/*=========================================================================
 * Local definitions
 *=======================================================================*/

static int isValidHeapPointer(cell* number);
static void markRootObjects(void);
static void markThreadStack(THREAD thisThread);

static void sweepTheHeap(void);

#define OBJECT_HEADER(object) ((cell *)(object))[-HEADERSIZE]

#define MARK_OBJECT(object)                                          \
     ( (!ROMIZING && INCLUDEDEBUGCODE && !inHeapSpace(object))   ?  \
            (fatalError("Marking object not in heap space"), 1)      \
     : (!ROMIZING || inHeapSpace(object))                        ?   \
            (OBJECT_HEADER(object) |= MARKBIT, 1)                    \
     : 0)

#define MARK_OBJECT_IF_NON_NULL(object) \
    if (object != NULL) { MARK_OBJECT(object); }

#define OVERWRITE_FREE_MEMORY (ENABLEPROFILING || INCLUDEDEBUGCODE)
#define OVERWRITE_FREE_MEMORY_BYTE 0xF1
#define OVERWRITE_FREE_MEMORY_WORD 0xF1F1F1F1


/*=========================================================================
 * Garbage collection operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      isValidHeapPointer
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Given a number, check if it is a valid pointer to
 *                an object in the dynamic heap.
 * INTERFACE:
 *   parameters:  possible heap address
 *   returns:     TRUE is this is a correct address, FALSE otherwise
 *=======================================================================*/

static int
isValidHeapPointer(cell* number)
{
    int lowbits;
    cell* scanner;
    int depth;

    /*  We don't need to deal with NULL pointers */
    if (number == 0)
        return FALSE;

    /*  Valid heap addresses are always four-byte aligned */
    if ((cell)number & 0x00000003)
        return FALSE;

    /*  Ensure that number is within heap space range */
    if (number < HeapSpace || number >= HeapSpaceTop)
        return FALSE;

    /*  The type field must contain a valid type tag; additionally, */
    /*  both static bit and mark bit must be unset. */
    lowbits = *(number-HEADERSIZE) & 0xFF;
    if ((lowbits & (MARKBIT|STATICBIT)) ||
          (lowbits < (GCT_FIRSTVALIDTAG << TYPE_SHIFT)) ||
          (lowbits > (GCT_LASTVALIDTAG << TYPE_SHIFT)))
        return FALSE;

#if SAFEGARBAGECOLLECTION
#if ENABLEPROFILING
    HeapScanPointerValidations++;
#endif

    /*  Now scan a little further down the heap in an attempt */
    /*  to find a badly formed object header */
    scanner = number + SIZE(*(number-HEADERSIZE));
    depth = TESTNONOBJECT_DEPTH;
    while (depth > 0 && scanner < HeapSpaceTop) {
        long size = SIZE(*scanner);
        GCT_ObjectType type = TYPE(*scanner);

        /*  A valid header will always have a non-zero size */
        if (size == 0)
            return FALSE;

        /*  Check if the scanner is pointing to a badly formed object header */
        if (!ISFREECHUNK(*scanner) &&
               (type < GCT_FIRSTVALIDTAG || type > GCT_LASTVALIDTAG))
            return FALSE;

        depth--;

        /*  Jump to the next object/chunk header in the heap */
        scanner += size+HEADERSIZE;
    }


    /*  Now have to scan from bottom of heap */
    scanner = HeapSpace;
    while (scanner <= number-HEADERSIZE) {
        /*  Get the size of the object (excluding the header) */
        long size = SIZE(*scanner);

        /*  Only examine the current header if it is not a free chunk header */
        if (!ISFREECHUNK(*scanner) && number == scanner+HEADERSIZE)
            return TRUE;

        /*  Jump to the next object/chunk header in the heap */
        scanner += size+HEADERSIZE;
    }

    /*  The given number did not point to any object header so return false */
    return FALSE;
#else
    return TRUE;
#endif
}


/*=========================================================================
 * FUNCTION:      markRootObjects
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Marks the root objects of garbage collection
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/


static void
markRootObjects(void)
{
    POINTERLIST roots = GlobalRoots;
    cell ***ptr = &roots->data[0].cellpp;
    int length = roots->length;
    THREAD thread;

    while (--length >= 0) {
        cell *subobject = **(ptr++);
        MARK_OBJECT_IF_NON_NULL(subobject);
    }

#if ROMIZING
    {
        /* In RELOCATABLE_ROM builds, we have a pointer to the static data.
         * In !RELOCATABLE_ROM builds, we have the actual array.
         */
#if RELOCATABLE_ROM
        long *staticPtr = KVM_staticDataPtr;
#else
        long *staticPtr = KVM_staticData;
#endif
        static void markROMHashtable(HASHTABLE x);
        int refCount = staticPtr[0];

        for( ; refCount > 0; refCount--) {
            MARK_OBJECT_IF_NON_NULL((cell *)staticPtr[refCount]);
        }

        markROMHashtable(UTFStringTable);
        markROMHashtable(InternStringTable);
        markROMHashtable(ClassTable);

        FOR_ALL_CLASSES(clazz)
            MARK_OBJECT_IF_NON_NULL(clazz->monitor);
        END_FOR_ALL_CLASSES
    }
#endif

    for (thread = AllThreads; thread != NULL; thread = thread->nextAliveThread){
        MARK_OBJECT(thread);
        MARK_OBJECT_IF_NON_NULL(thread->javaThread);
        if (thread->stack != NULL) {
            markThreadStack(thread);
        }
    }
}


/*=========================================================================
 * FUNCTION:      markROMHashtable
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Mark a hashtable that isn't in the heap
 * INTERFACE:
 *   parameters:  hashtable
 *   returns:     none.
 *=======================================================================*/

#if ROMIZING
static void
markROMHashtable(HASHTABLE table) {
    int bucketCount = table->bucketCount;
    int i;
    for (i = 0; i < bucketCount; i++) {
        MARK_OBJECT_IF_NON_NULL(table->bucket[i]);
    }
}
#endif


/*=========================================================================
 * FUNCTION:      markNonRootObjects
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Scan the heap looking for objects that are reachable only
 *                from other heap objects.
 * INTERFACE:
 *   parameters:  None
 *   returns:     <nothing>
 * COMMENTS:      This code >>tries<< to do all its work in a single pass
 *                through the heap.  For each live object in the heap, it
 *                calls the function markChildren().  This latter function
 *                sets the variable deferredObjectTableOverflow if it was
 *                ever unable to completely following the children of an
 *                object because of recursion overflow.  In this rare case,
 *                we just rescan the heap.  We're guaranteed to get further
 *                to completion on each pass.
 *=======================================================================*/


#define MAX_GC_DEPTH 4


static void
markNonRootObjects(void) {
    /*  Scan the entire heap, looking for badly formed headers */
    cell* scanner;
    cell* heapSpaceTop = HeapSpaceTop;
    int scans = 0;
    do {
        initializeDeferredObjectTable();
        for (   scanner = HeapSpace;
                scanner < heapSpaceTop;
                scanner += SIZE(*scanner) + HEADERSIZE) {
            if (ISMARKED(*scanner)) {
                cell *object = scanner + 1;
                /* See markChildren() for comments on the arguments */
                markChildren(object, object, MAX_GC_DEPTH);
            }
        }
        if (ENABLEPROFILING) {
            scans++;
        }
        /* This loop runs exactly once in almost all cases */
    } while (deferredObjectTableOverflow);
#if ENABLEPROFILING
    GarbageCollectionRescans += (scans - 1);
#endif
}


/*=========================================================================
 * FUNCTION:      markChildren()
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Scan the contents of an individual object,
 *                recursively marking all those objects that
 *                are reachable from it.
 * INTERFACE:
 *   parameters:  child: an object pointer
 *                limit: the current location of the scanner in
 *                          markNonRootObjects
 *                depth: The number of times further for which this function
 *                          can recurse, before we start adding objects to
 *                          the deferral queue.
 *   returns:     <nothing>
 *
 * This function tries >>very<< hard not to be recursive.  It uses several
 * tricks:
 *   1) It will only recursively mark objects whose location in the heap
 *      is less than the value "limit".  It marks but doesn't recurse on
 *      objects "above the limit" because markNonRootObjects() will
 *      eventually get to them.
 *
 *   2) The variable nextObject is set in those cases in which we think, or
 *      hope that only a single child of the current node needs to be
 *      followed.  In these cases, we iterate (making explicit the tail
 *      recursion), rather than calling ourselves recursively
 *
 *   3) We only allow the stack to get to a limited depth.  If the depth
 *      gets beyond that, we save the sub object on a "deferred object"
 *      queue, and then iterate on these deferred objects once the depth
 *      gets back down to zero.
 *
 *   4) If the deferred object queue overflows, we just throw up our
 *      hands, and scan the stack multiple times.  We're guaranteed to make
 *      some progress in each pass through the stack.  It's hard to
 *      create a case that needs more than one pass through the stack.
 *=======================================================================*/

static void
markChildren(cell* object, cell* limit, int remainingDepth)
{

/* Call this macro to mark a child, when we don't think that there is
 * any useful reason to try for tail recursion (i.e. there's something
 * later in the object that's better to tail recurse on.
 */


#define MARK_AND_RECURSE(child) \
    if (child != NULL && !ISMARKED(OBJECT_HEADER(child))) {  \
        if (MARK_OBJECT(child)) {                            \
            if ((cell*)child < limit) {                      \
                RECURSE((cell *)child);                      \
            }                                                \
        }                                                    \
    }

/* Call this macro to mark a child, when we think tail recursion might
 * be useful on this field.
 * This macro does the right thing, even if we might have already picked
 * another child on which to tail recurse.
 *
 * [We use this macro when looking at elements of an array, or fields of
 * an instance, since we want to tail recurse on one of the children.
 * We just don't know which ones will be good to recurse on
 */
#define MARK_AND_TAIL_RECURSE(child)                         \
    if (child != NULL && !ISMARKED(OBJECT_HEADER(child))) {  \
        if (MARK_OBJECT(child)) {                            \
            if ((cell*)child < limit) {                      \
                if (nextObject != NULL) {                    \
                    RECURSE(nextObject);                     \
                }                                            \
                nextObject = (cell *)(child);                \
            }                                                \
        }                                                    \
    }

/* This is the same as MARK_AND_TAIL_RECURSE, except we only call it when we
 * absolutely know that we have not yet called MARK_AND_TAIL_RECURSE, so
 * we know that we have not yet picked a child upon which to tail recurse.
 */

#define MARK_AND_TAIL_RECURSEX(child)                        \
    if (child != NULL && !ISMARKED(OBJECT_HEADER(child))) {  \
        if (MARK_OBJECT(child)) {                            \
            if ((cell*)child < limit) {                      \
                nextObject = (cell *)(child);                \
            }                                                \
        }                                                    \
    }


/* Used only in the macros above.  Either recurse on the child, or
 * save it in the deferred items list.
 *
 * Note that remainingDepth will have already been decremented from the value
 * that it originally had when the function was called
 */

#define RECURSE(child)                                       \
    if (remainingDepth < 0) {                                \
       putDeferredObject(child);                             \
    } else {                                                 \
       markChildren(child, limit, remainingDepth);           \
    }


    /* If non-NULL, then it holds the value of object for the next iteration
     * through the loop.  Used to implement tail recursion.
     */
    cell *nextObject = NULL;

    remainingDepth -= 1;        /* remaining depth for any subcalls */

    for(;;) {
        cell *header = object - HEADERSIZE;
        GCT_ObjectType gctype = TYPE(*header);

        if (TRACEGARBAGECOLLECTIONVERBOSE) {
            fprintf(stdout, "GC Mark: ");
            printObject(object);
        }

        switch (gctype) {
            int length;
            cell **ptr;

        case GCT_INSTANCE: {
            /*  The object is a Java object instance.  Mark pointer fields */
            INSTANCE instance = (INSTANCE)object;
            INSTANCE_CLASS clazz = instance->ofClass;
            while (clazz) {
                /*  This is the tough part: walk through all the fields  */
                /*  of the object and see if they contain pointers */

                /*  Mark the possible monitor object alive */
                MARK_AND_RECURSE(instance->monitor);

                FOR_EACH_FIELD(thisField, clazz->fieldTable)
                    /*  Is this a non-static pointer field? */
                    if ((thisField->accessFlags & (ACC_POINTER | ACC_STATIC))
                               == ACC_POINTER) {
                        int offset = thisField->u.offset;
                        cell *subobject = instance->data[offset].cellp;
                        MARK_AND_TAIL_RECURSE(subobject);
                    }
                END_FOR_EACH_FIELD
                clazz = clazz->superClass;
            }
            break;
        }

        case GCT_ARRAY: {
            /*  The object is a Java array with primitive values. */
            /*  Only the possible monitor will have to be marked alive. */
            INSTANCE thisObject = (INSTANCE)object;
            MARK_AND_TAIL_RECURSEX(thisObject->monitor);
        }
        break;

        case GCT_POINTERLIST: {
            POINTERLIST list = (POINTERLIST)object;
            length = list->length;
            ptr = &list->data[0].cellp;
            goto markArray;
        }

        case GCT_OBJECTARRAY: {
            /*  The object is a Java array with object references. */
            /*  The contents of the array and the possible monitor  */
            /*  will have to be scanned. */
            ARRAY array = (ARRAY)object;

            /*  Mark the possible monitor object alive */
            MARK_AND_RECURSE(array->monitor);

            length = array->length;
            ptr = &array->data[0].cellp;
            /* FALL THROUGH */
        }

        markArray:
            /* Keep objects in the array alive. */
            while (--length >= 0) {
                cell *subobject = *ptr++;
                MARK_AND_TAIL_RECURSE(subobject);
            }
            break;

        case GCT_ARRAY_CLASS:
            /*  Mark the possible monitor object alive */
            MARK_AND_RECURSE(((ARRAY_CLASS)object)->class.monitor);
            MARK_AND_TAIL_RECURSEX(((ARRAY_CLASS)object)->class.next);
            break;

        case GCT_INSTANCE_CLASS: {
            INSTANCE_CLASS thisClass = (INSTANCE_CLASS)object;
            MARK_AND_RECURSE(thisClass->class.monitor);

            MARK_AND_TAIL_RECURSEX(thisClass->class.next);
            MARK_AND_TAIL_RECURSE(thisClass->staticFields);

            /*  Mark the constant pool alive.  We know from the code
             * in GCT_CONSTPOOL that this has no subpointers */
            if (!USESTATIC) {
                MARK_OBJECT_IF_NON_NULL(thisClass->constPool);
                /*  Mark the interface table alive.  This is a NOPOINTER */
                MARK_OBJECT_IF_NON_NULL(thisClass->ifaceTable);
                /*  Mark the method table alive */
                MARK_AND_TAIL_RECURSE(thisClass->methodTable);
                /*  Mark the field table alive. */
                MARK_OBJECT_IF_NON_NULL(thisClass->fieldTable);
            } else if (ENABLEFASTBYTECODES) {
                /* This is ugly.  The method blocks are in static, but
                 * the code is in the heap.   Yuck! */
                /*  The object is an internal method table of a class. */
                METHODTABLE methodTable  = thisClass->methodTable;
                if (methodTable != NULL) {
                    FOR_EACH_METHOD(thisMethod, methodTable)
                        if (!(thisMethod->accessFlags & ACC_NATIVE)) {
                            MARK_OBJECT_IF_NON_NULL(thisMethod->u.java.code);
                        }
                    END_FOR_EACH_METHOD
                }
            }
            break;
        }

        case GCT_FIELDTABLE:
            /* No longer necessary to scan field tables.  The names and
             * signatures are stored in the string pool, which is kept
             * alive automatically.
             */
            break;

        case GCT_METHODTABLE: {
            /*  The object is an internal method table of a class. */
            METHODTABLE  methodTable  = (METHODTABLE)object;
            FOR_EACH_METHOD(thisMethod, methodTable)
                /*  Mark the bytecode object alive for non-native methods */
                if (!(thisMethod->accessFlags & ACC_NATIVE)) {
                    MARK_OBJECT_IF_NON_NULL(thisMethod->u.java.code);
                    MARK_OBJECT_IF_NON_NULL(thisMethod->u.java.handlers);
                    MARK_OBJECT_IF_NON_NULL(thisMethod->u.java.stackMaps);
                }
                MARK_OBJECT_IF_NON_NULL(thisMethod->mayThrow);
            END_FOR_EACH_METHOD
            break;
        }


        case GCT_INLINECACHE:
            /*  Inline cache area does not currently contain  */
            /*  any pointers that are not scanned by other means. */
            if (!ENABLEFASTBYTECODES) {
                fatalVMError("Fatal: Bad dynamic heap objects");
            }
            break;

#if !ROMIZING
        case GCT_HASHTABLE:
            length = ((HASHTABLE)object)->bucketCount;
            ptr = ((HASHTABLE)object)->bucket;
            goto markArray;
#endif

        case GCT_UTF_HASH_ENTRY:
            MARK_AND_TAIL_RECURSEX(((UString)object)->next);
            break;

        case GCT_STRING_HASH_ENTRY: {
            STRING_HASH_ENTRY entry = ((STRING_HASH_ENTRY)object);
            STRING_INSTANCE string = entry->string;
            if (string != NULL) {
                /* As a bootstrapping issue, we mark the string object "by
                   hand", since "java/lang/String" may not yet be loaded. */

                MARK_OBJECT(string);
                MARK_OBJECT_IF_NON_NULL(string->array);
            }
            MARK_AND_TAIL_RECURSEX(entry->next);
            break;
        }

        case GCT_MONITOR: {
            MONITOR monitor  = (MONITOR)object;
            if (       monitor->owner == NULL
                    && monitor->monitor_waitq == NULL
                    && monitor->condvar_waitq == NULL) {
                /* This monitor isn't really in use.  Detach it */
                setObjectMonitor(monitor->object, NULL);
                OBJECT_HEADER(object) &= ~MARKBIT;
            }
            break;

        }

        case GCT_CONSTANTPOOL:
            /* Scanning the constant pool is no longer necessary, since the
             * strings are now stored in a separate string pool, which is
             * kept alive automatically. */
            break;

        case GCT_NOPOINTERS:
            /*  The object does not have any pointers in it */
            break;

        case GCT_EXECSTACK:
            /* Scanning of executing stack is done as part of the root set */
            break;

        case GCT_THREAD:
            /* Scanning of all live threads is done as part of the root set */
            break;

        case GCT_GLOBAL_ROOTS:
            /* These are all marked as part of the root set */
            break;

        default:
            /*  We should never get here as isValidHeapPointer should */
            /*  guarantee that the header tag of this object is in the */
            /*  range GCT_FIRSTVALIDTAG && GCT_LASTVALIDTAG */
            fatalVMError("Fatal: Bad dynamic heap objects");

        } /* End of switch statement */

        if (nextObject != NULL) {
            /* We can perform a tail recursive call. */
            object = nextObject;
            nextObject = NULL;
            /* continue */
        } else if (remainingDepth == MAX_GC_DEPTH - 1
                         && deferredObjectCount > 0) {
            /* We're at the outer level of recursion and there is a deferred
             * object for us to look up.  Note that remainingDepth has
             * already been decremented, so we don't compare against
             * MAX_GC_DEPTH
             */
            object = getDeferredObject();
            /* continue */
        } else {
            break;              /* finish "for" loop. */
        }
    } /* end of for(ever) loop */
}

/*=========================================================================
 * FUNCTION:      markThreadStack
 * TYPE:          private garbage collection operation
 * OVERVIEW:      Scan the execution stack of the given thread, and
 *                mark all the objects that are reachable from it.
 * INTERFACE:
 *   parameters:  pointer to the thread whose stack is to be scanned.
 *   returns:     <nothing>
 *=======================================================================*/

static void
markThreadStack(THREAD thisThread)
{
    STACK  stack  = thisThread->stack;
    /*  Perform a complete stack trace, looking for pointers  */
    /*  inside the stack and marking the corresponding objects. */

    FRAME  thisFp = thisThread->fpStore;
    cell*  thisSp = thisThread->spStore;
    long    size, i;
    cell*  operands;
    long   pointerMask;

    {
        STACK stack_tmp = stack;
        while (stack_tmp) {
            if (TRACEGARBAGECOLLECTIONVERBOSE) {
                fprintf(stdout, "Marking stack@%lx [%ld]\n",
                    (long)stack_tmp, getObjectSize((cell*)stack_tmp));
            }
            MARK_OBJECT(stack_tmp);
            /* Reclaim unused stack chunks */
            if (STACK_CONTAINS(stack_tmp, thisSp)) {
                if (TRACE_STACK_CHUNKS) {
                    STACK s = stack_tmp->next;
                    while (s) {
                        fprintf(stdout,
                                "thread %lx, GCing chunk %lx, size = %d\n",
                                (long)CurrentThread, (long)s, s->size);
                        s = s->next;
                    }
                }
                stack_tmp->next = NULL;
                break;
            } else {
                stack_tmp = stack_tmp -> next;
            }
        }
    }

    while (thisFp != NULL) {
        /*  Get a pointer to the method of the current frame */
        METHOD method = thisFp->thisMethod;
        int argCount = method->argCount;
        cell* thisLp = FRAMELOCALS(thisFp);

        /*  Mark the possible synchronization object alive */
        MARK_OBJECT_IF_NON_NULL(thisFp->syncObject);

        /*  Get the mask for the current method that indicates for */
        /*  each parameter slot whether or not it is a pointer type */
        pointerMask = getParameterSlotsPointerMask(method);
        if (pointerMask >= 0 && (method->accessFlags & ACC_STATIC) == 0)
            pointerMask = (pointerMask << 1) | 1;

        /*  Now scan method parameters using type information. If the
         *  method parameters take up more than 31 slots, the necessary
         *  type information will not have been successfully generated
         *  (pointerMask will be negative) and so we must use the
         *  isValidHeapPointer check instead.
         *
         *  We then Scan the local variables, for which we have to use
         *  isValidHeapPointer();
         */
        for (i = 0; i < method->frameSize; i++) {
            cell* arg;
            if (i < argCount && pointerMask >= 0) {
                if (pointerMask & (1 << i)) {
                    arg = *(cell**)&thisLp[i];
                } else {
                    continue;
                }
            } else {
                arg = *(cell**)&thisLp[i];
                if (!isValidHeapPointer(arg)) {
                    continue;
                }
            }
            MARK_OBJECT_IF_NON_NULL(arg);

        }

        /*  Scan the operand stack of the current frame. */
        /*  Once again, no type information is available, so we use the same */
        /*  check as above when scanning the local variables. */
        size = thisSp - (cell*)thisFp - SIZEOF_FRAME + 1;
        operands = (cell*)thisFp + SIZEOF_FRAME;
        for (i = 0; i < size; i++) {
            cell* operand = *(cell**)&operands[i];
            if (isValidHeapPointer(operand))
                MARK_OBJECT_IF_NON_NULL(operand);
        }

        /*  This frame is now done.  Go to the previous one */
        /*  using the same algorithm as in 'popFrame()'. */
        thisSp = thisFp->previousSp;
        thisFp = thisFp->previousFp;
    }
}







/*=========================================================================
 * FUNCTION:      sweepTheHeap
 * TYPE:          private garbage collection function
 * OVERVIEW:      Sweep the heap, adding all the unmarked objects to
 *                the free list and resetting the mark bits in the
 *                marked (live) objects.  This operation implements
 *                the "sweep" phase of our mark-and-sweep garbage
 *                collector.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

static void
sweepTheHeap(void)
{
    /*  These variables are used for merging chunks in the free list  */
    int   previousObjectWasDead = FALSE;
    CHUNK previousFreeChunk = NIL;
    CHUNK newChunk;

    /*  Walk through the objects in the heap one at a time, */
    /*  looking for unmarked objects. */
    cell* scanner = HeapSpace;
    while (scanner < HeapSpaceTop) {
        /*  Get the size of the object (excluding the header) */
        long size = SIZE(*scanner);

        /*  Read the mark bit */
        if (ISMARKED(*scanner)) {
            /*  This is a live object; unset the mark bit */
            *scanner &= ~MARKBIT;
            previousObjectWasDead = FALSE;
        } else {
            if (*scanner & 0x2) {
                fatalError("Bad bit set");
            }
            /*  This is a dead object: */
            if (OVERWRITE_FREE_MEMORY && !ISFREECHUNK(*scanner)) {
                /* Zap the newly recovered memory with 6's to facilitate
                 * debugging */
                memset(scanner+1, OVERWRITE_FREE_MEMORY_BYTE, size << log2CELL);
            }
            /*  If previous object in the heap was garbage, too,  */
            /*  then we merge the space of this object with the previous */
            /*  chunk in the free list. */
            if (previousObjectWasDead) {
                /*  Add the newly reclaimed space to the previous */
                /*  chunk, keeping in mind that object length does */
                /*  not include header size. */
                previousFreeChunk->size += ((size+HEADERSIZE) << TYPEBITS);
                if (OVERWRITE_FREE_MEMORY) {
                    /* also overwrite our own header */
                    *scanner = OVERWRITE_FREE_MEMORY_WORD;
                }
            } else {
                /*  Previous object was alive; create a new chunk */
                /*  in the free list. */
                newChunk = (CHUNK)scanner;
                newChunk->size = size << TYPEBITS;
                newChunk->next = NIL;

                /*  If this is the first dead object in the heap,  */
                /*  assign its address to the global variable  */
                /*  'FirstFreeChunk'. */
                if (previousFreeChunk) {
                    previousFreeChunk->next = newChunk;
                } else {
                    FirstFreeChunk = newChunk;
                }

                previousFreeChunk = newChunk;
                previousObjectWasDead = TRUE;
            }
            if (ENABLEPROFILING) {
                /* Ensure that the newly created or grown chunk is within
                 * the heap boundaries */
                if ((cell*)previousFreeChunk + (size+HEADERSIZE) > HeapSpaceTop) {
                    fatalVMError("Free chunk overflowed heap top");
                }
            }
        }

        /*  Go to the next object in the heap */
        scanner += size+HEADERSIZE;
    }
    if (ENABLEPROFILING) {
        /*  Ensure that scanning finished exactly at the heap boundary. */
        if (scanner != HeapSpaceTop) {
            fatalVMError("Heap sweep went past heap top");
        }
    }
}

/*=========================================================================
 * FUNCTION:      garbageCollect
 * TYPE:          public garbage collection function
 * OVERVIEW:      Perform mark-and-sweep garbage collection.
 * INTERFACE:
 *   parameters:  int moreMemory: the amount by which the heap
 *                size should be grown during garbage collection
 *                (this feature is not supported in the mark-and-sweep
 *                collector).
 *   returns:     <nothing>
 *=======================================================================*/

void
garbageCollect(int moreMemory)
{
    static void garbageCollectForReal(int moreMemory);
    collectorIsRunning = TRUE;
    RundownAsynchronousFunctions();
    garbageCollectForReal(moreMemory);
    collectorIsRunning = FALSE;
}

static void
garbageCollectForReal(int moreMemory)
{
    UNUSEDPARAMETER(moreMemory)
    long beforeCollection = 0;
    long afterCollection;

    if (ENABLEPROFILING && INCLUDEDEBUGCODE) {
        checkHeap();
    }

     if (TRACEGARBAGECOLLECTION && !TERSE_MESSAGES)
        Log->fprintf(stdout,"Garbage collecting...\n");
    if (ENABLEPROFILING || TRACEGARBAGECOLLECTION || Verbose_On)
        beforeCollection = memoryFree();

    /*  Store virtual machine registers of the currently  */
    /*  active thread before garbage collection (must be */
    /*  done to enable execution stack scanning). */
    if (CurrentThread)
        storeExecutionEnvironment(CurrentThread);

    /*  Mark the root objects */
    if (TRACEGARBAGECOLLECTIONVERBOSE) {
        Log->fprintf(stdout, "Marking roots");
    }

    markRootObjects();

    /* Mark all the objects pointed at by the root objects, or pointed at
     * by objects that are pointed at by root objects. . . .
     */
    if (TRACEGARBAGECOLLECTIONVERBOSE) {
        Log->fprintf(stdout, "Scanning heap for additional objects\n");
    }
    markNonRootObjects();

    /*  Sweep the heap, adding all the unmarked objects to */
    /*  the free list and resetting the mark bits in the */
    /*  marked (live) objects. This is the "sweep" phase of */
    /*  our mark-and-sweep collector. */
    if (TRACEGARBAGECOLLECTIONVERBOSE) {
        Log->fprintf(stdout, "Removing dead objects\n");
    }
    sweepTheHeap();

    if (INCLUDEDEBUGCODE && TRACEGARBAGECOLLECTION) {
        printHeapContents();
    }


    if (ENABLEPROFILING || TRACEGARBAGECOLLECTION || Verbose_On) {
        afterCollection = memoryFree();
#if ENABLEPROFILING
        GarbageCollectionCounter += 1;
        DynamicDeallocationCounter += (afterCollection - beforeCollection);
#endif
        if (TRACEGARBAGECOLLECTION || Verbose_On) {
            Log->fprintf(stdout,
                         "Collected %ld bytes of garbage.  In use %ld bytes\n",
                         afterCollection - beforeCollection, getHeapSize());
        }
    }

}


/*=========================================================================
 * FUNCTION:      makeTemporaryRoot
 * TYPE:          public garbage collection function
 * OVERVIEW:      Make the given object a temporary root for the garbage
 *                collector. This operation is needed because sometimes
 *                the VM/native operations must perform two or more
 *                memory allocations in a sequence (see 'instantiateString').
 *                If the allocated objects are not stored in permanent
 *                structures right away, a garbage collection could
 *                potentially deallocate the structures before they are
 *                actually taken in use. The temporary root list is used
 *                for ensuring that accidental deallocations do not
 *                happen.
 * INTERFACE:
 *   parameters:  the object that should become a temporary root.
 *   returns:     <nothing>
 * NOTE:          This operation replaces the 'ensureCapacity' operation
 *                used in the earlier copying collector.
 *=======================================================================*/

void makeTemporaryRoot(cell* object)
{
    int index    = TemporaryRoots->length;

    /* Note, this is one less than the actual amount of data it
     * can hold.  See note below */
    int size = getObjectSize((cell *)TemporaryRoots) - SIZEOF_POINTERLIST(1);

    TemporaryRoots->data[index].cellp = object;
    TemporaryRoots->length = index + 1;

    if (index >= size) {
        /* We must make sure that the TemporaryRoot structure always has
         * one empty spot in it.  We must always be able to insert a new
         * object into the array, before performing the allocation below */
        int newSize = 2 * size + 1;
        POINTERLIST newRoots = (POINTERLIST)mallocObject(SIZEOF_POINTERLIST(newSize),
                                                     GCT_POINTERLIST);
        memcpy(newRoots->data, TemporaryRoots->data, (index + 1) << log2CELL);
        newRoots->length = index + 1;
        TemporaryRoots = newRoots;
    }
}


/*=========================================================================
 * FUNCTION:      makeTransientRoot
 * TYPE:          public garbage collection function
 * OVERVIEW:      Make the given object a transient for the garbage
 *                collector.
 *                Transient roots are similar to temporary roots, but they
 *                do not need to obey a stack discipline.  Roots can be
 *                established and removed in any order.
 * INTERFACE:
 *   parameters:  the object that should become a temporary root.
 *   returns:     index
 *
 * NOTE:          The transient root can be removed by calling either
 *                removeTransientRootByValue() with the object, or by
 *                calling removeTransientRootByIndex(int) with the
 *                index returned by this function.  The latter is faster.
 *
 *=======================================================================*/

int
makeTransientRoot(cell* object)
{
    POINTERLIST roots = TransientRoots;
    POINTERLIST newRoots;
    int size    = roots->length;
    int newSize;
    int i;

    if (object == NULL) {
        /* We reserve slot 0 for NULL objects.  We also use slot 0 as
         * a temporary reserve, for when we need to grow this root set */
        return 0;
    }

    for (i = 1; i < size; i++) {
        /* Look for a hole.  If one exists, put this object in it, and
         * return that slot number
         */
        if (roots->data[i].cellp == NULL) {
            roots->data[i].cellp = object;
            return i;
        }
    }

    /* The structure is completely full.  Temporarily protect the new
     * object by putting it into slot 0.
     */
    roots->data[0].cellp = object;

    /* Grow this array */
    newSize = 2 * size + 1;
    newRoots = (POINTERLIST)callocObject(SIZEOF_POINTERLIST(newSize),
                                         GCT_POINTERLIST);
    newRoots->length = newSize;
    memcpy(newRoots->data, roots->data, size << log2CELL);
    newRoots->data[size].cellp = object; /* it was protected */
    newRoots->data[0].cellp = NULL;

    TransientRoots = newRoots;
    return size;
}

/*=========================================================================
 * FUNCTION:      removeTransientRootByValue
 *                removeTransientRootByIndex
 * TYPE:          public garbage collection function
 * OVERVIEW:      Remove an object from the transient root set.
 *
 * INTERFACE:
 *   parameters:  value:  The value in the transient root set
 *             or index:  The index of the object in the root set.
 *   returns:     <nothing>
 *=======================================================================*/

void
removeTransientRootByValue(cell *value) {
    /* Find the value in the array.  NULL it */
    POINTERLIST roots = TransientRoots;
    int size    = roots->length;
    int i;
    for (i = 0; i < size; i++) {
        /* value == NULL matches i == 0 */
        if (roots->data[i].cellp == value) {
            roots->data[i].cellp = NULL;
            return;
        }
    }
    fatalError("Removal of non-existent asynchronous root");
}

void
removeTransientRootByIndex(int index) {
    /* Null the corresponding array element */
    TransientRoots->data[index].cellp = NULL;
}


void
makeGlobalRoot(cell** object)
{
    POINTERLIST roots = GlobalRoots;
    int index    = roots->length;

    /* Note, this is one less than the actual amount of data it
     * can hold.  See note below */
    int size = getObjectSize((cell *)roots) - SIZEOF_POINTERLIST(1);

    roots->data[index].cellpp = object;
    roots->length = index + 1;

    if (index >= size) {
        /* We must make sure that the GlobalRoot structure always has
         * one empty spot in it.  We must always be able to insert a new
         * object into the array, before performing the allocation below */
        int newSize = 2 * size + 1;
        POINTERLIST newRoots = (POINTERLIST)mallocObject(SIZEOF_POINTERLIST(newSize),
                                                     GCT_GLOBAL_ROOTS);
        memcpy(newRoots->data, roots->data, (index + 1) << log2CELL);
        newRoots->length = index + 1;
        GlobalRoots = newRoots;
    }
}



/*=========================================================================
 * Memory allocation operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      allocateFreeChunk
 * TYPE:          private memory allocation operation
 * OVERVIEW:      Find a chunk of memory in the free list with
 *                enough space to store an object of 'size' words
 *                (including the header).
 *                If successful, update the free list correspondingly.
 * INTERFACE:
 *   parameters:  size: the amount of data requested (in 32 bit words)
 *   returns:     Pointer to the beginning of the chunk, or NIL if
 *                there isn't a big enough chunk available.
 * COMMENTS:      This operation should be streamlined later.
 *                It is rather heavyweight to serve as the primary
 *                memory allocation operation.
 *=======================================================================*/

cell* allocateFreeChunk(long size)
{
    CHUNK thisChunk = FirstFreeChunk;
    CHUNK prevChunk = NIL;
    cell* dataArea  = NIL;

    while (thisChunk) {

        /*  Calculate how much bigger or smaller the */
        /*  chunk is than the requested size */
        long overhead = SIZE(thisChunk->size) + HEADERSIZE - size;

        /* If the chunk is big enough, allocate it */
        if (overhead >= 0) {

            dataArea = (cell*)thisChunk;

            /* If there is enough overhead in the current chunk */
            /* to store another object (minimum size is header + */
            /* 1 cell), create a new chunk from the remains of  */
            /* the current one.  */
            if (overhead > HEADERSIZE) {
                CHUNK newChunk = (CHUNK)(dataArea + size);
                newChunk->size = (overhead - HEADERSIZE) << TYPEBITS;
                newChunk->next = thisChunk->next;
                if (prevChunk)
                    prevChunk->next = newChunk;
                else
                    FirstFreeChunk = newChunk;

                /*  Store the size of the object in the object header */
                *dataArea = (size - HEADERSIZE) << TYPEBITS;
            }
            else {
                /*  There was an exact match or overhead is too small to  */
                /*  be useful (header only). Remove this chunk from free */
                /*  list and if there is extra space at the end of the */
                /*  chunk, it becomes wasted space for the lifetime of the */
                /*  allocated object. */
                if (prevChunk) prevChunk->next = thisChunk->next;
                else FirstFreeChunk = thisChunk->next;

                /*  Store the size of the object in the object header */
                *dataArea = (size + overhead - HEADERSIZE) << TYPEBITS;
            }

            if (ENABLEPROFILING || TRACEMEMORYALLOCATION) {
                if (dataArea < HeapSpace || dataArea+size > HeapSpaceTop) {
                    fprintf(stderr, "Heap pointers corrupted\n");
                    fatalVMError("Execution stopped");
                }
            }
            return (cell*)dataArea;
        }

        prevChunk = thisChunk;
        thisChunk = thisChunk->next;
    }

    /*  If we got here, there was no chunk with enough memory available */
    return NIL;
}

/*=========================================================================
 * FUNCTION:      mallocHeapObject()
 * TYPE:          public memory allocation operation
 * OVERVIEW:      Allocate a contiguous area of n cells in the dynamic heap.
 * INTERFACE:
 *   parameters:  size: the requested object size in cells,
 *                type: garbage collection type of the object
 *   returns:     pointer to newly allocated area, or
 *                NIL is allocation fails.
 * COMMENTS:      Allocated data area is not initialized, so it
 *                may contain invalid heap references. If you do
 *                not intend to initialize data area right away,
 *                you must use 'callocObject' instead (or otherwise
 *                the garbage collector will get confused next time)!
 *=======================================================================*/

cell* mallocHeapObject(long size, GCT_ObjectType type)
/*  Remember: size is given in CELLs rather than bytes */
{
    cell* thisChunk;

    /*  Size must be at least 1 */
    size = size > 0 ? size : 1;

    /*  Try to find a chunk of memory with enough space  */
    /*  to store the object and its header */

    if (EXCESSIVE_GARBAGE_COLLECTION && (type != GCT_GLOBAL_ROOTS)) {
        garbageCollect(0);
    }

    thisChunk = allocateFreeChunk(size+HEADERSIZE);
    if (thisChunk == NULL) {
        /*  If there is not enough memory, perform a garbage collection */
        garbageCollect(0);
        /* If there still isn't enough space, throw an error */
        thisChunk = allocateFreeChunk(size+HEADERSIZE);
        if (thisChunk == NULL) {
            return NULL;
        }
    }

    /*  Augment the object header with gc type information */
    *thisChunk |= (type << 2);

    if (TRACEMEMORYALLOCATION) {
        cell *object = thisChunk+HEADERSIZE;
        Log->allocateObject((long)object,
                            getObjectSize(object),
                            getObjectType(object),
                            (long)object,
                            memoryFree());
    }

#if ENABLEPROFILING
    DynamicObjectCounter     += 1;
    DynamicAllocationCounter += (size+HEADERSIZE)*CELL;
#endif

    return thisChunk + 1;
}

/*=========================================================================
 * FUNCTION:      mallocObject, callocObject
 * TYPE:          public memory allocation operation
 * OVERVIEW:      Allocate a contiguous area of n cells in the dynamic heap.
 * INTERFACE:
 *   parameters:  size: the requested object size in cells,
 *                type: garbage collection type of the object
 *   returns:     pointer to newly allocated area.
 *                Gives a fatal error if space cannot be allocated.
 *
 * COMMENTS:      callocObject zeros the space before returning it.
 *=======================================================================*/

cell* mallocObject(long size, GCT_ObjectType type)
/*  Remember: size is given in CELLs rather than bytes */
{
    cell *result = mallocHeapObject(size, type);
    if (result == NULL) {
        fatalVMError("Out of heap memory!");
    }
    return result;
}

cell* callocObject(long size, GCT_ObjectType type)
{
    cell *result = mallocHeapObject(size, type);
    if (result == NULL) {
        fatalVMError("Out of heap memory!");
    }
    memset(result, 0, size << log2CELL);
    return result;
}

/*=========================================================================
 * FUNCTION:      mallocBytes
 * TYPE:          public memory allocation operation
 * OVERVIEW:      Allocate a contiguous area of n bytes in the dynamic heap.
 * INTERFACE:
 *   parameters:  size: the requested object size in bytes
 *   returns:     pointer to newly allocated area, or
 *                NIL is allocation fails.
 * COMMENTS:      mallocBytes allocates areas that are treated as
 *                as byte data only. Any possible heap references
 *                within such areas are ignored during garbage
 *                collection.
 *
 *                The actual size of the data area is rounded
 *                up to the next four-byte aligned memory address,
 *                so the area may contain extra bytes.
 *=======================================================================*/

char* mallocBytes(long size)
{
    /*  The size is given in bytes, so it must be */
    /*  rounded up to next long word boundary */
    /*  and converted to CELLs */
    cell* dataArea = mallocObject((size + CELL - 1) >> log2CELL,
                                  GCT_NOPOINTERS);
    return (char*)dataArea;
}


/*=========================================================================
 * Auxiliary dynamic heap operations & printing
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getHeapSize
 * TYPE:          public function
 * OVERVIEW:      Return the total amount of memory in the dynamic heap
 *                in bytes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     total amount of memory (in bytes)
 *=======================================================================*/

long getHeapSize(void)
{
    return HeapSize;
}

/*=========================================================================
 * FUNCTION:      memoryFree
 * TYPE:          public function
 * OVERVIEW:      Return the total amount of free memory in the
 *                dynamic heap in bytes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     total amount of free memory (in bytes)
 *=======================================================================*/

long memoryFree(void)
{
    /*  Calculate the amount of memory available in the free list */
    CHUNK thisChunk = FirstFreeChunk;
    long available = 0;

    while (thisChunk) {
        available += (thisChunk->size >> TYPEBITS) + HEADERSIZE;
        thisChunk = thisChunk->next;
    }

    return available*CELL;
}




/*=========================================================================
 * FUNCTION:      memoryUsage
 * TYPE:          public function
 * OVERVIEW:      Return the amount of memory currently consumed by
 *                all the heap objects.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     amount of consumed memory (in bytes).
 *=======================================================================*/

long memoryUsage(void)
{
    return HeapSize - memoryFree();
}

/*=========================================================================
 * FUNCTION:      printContents, getObjectTypeStr
 * TYPE:          public debugging operation
 * OVERVIEW:      Print the contents of the heap. Intended for
 *                debugging and heap consistency checking.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE || TRACEGARBAGECOLLECTIONVERBOSE


/*=========================================================================
 * FUNCTION:      largestFree
 * TYPE:          public function
 * OVERVIEW:      Return the largest block of free memory in the
 *                dynamic heap in bytes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     size of largest free block (in bytes)
 *=======================================================================*/

long largestFree(void)
{
    CHUNK thisChunk = FirstFreeChunk;
    long largest = 0;
    long size;

    while (thisChunk) {
        size = (thisChunk->size >> TYPEBITS) + HEADERSIZE;
        if (largest < size) largest = size;
        thisChunk = thisChunk->next;
    }
    return largest*CELL;
}


static char*
getObjectTypeStr(GCT_ObjectType type) {
    static char* typeNames[] = GCT_TYPENAMES;
    if ((type >= 0) && (type <= GCT_LASTVALIDTAG)) {
        return typeNames[type];
    } else {
        return "<unknown>";
    }
}

void
printObject(cell *object) {
    cell *header = object - HEADERSIZE;
    char buffer[256];
    GCT_ObjectType gctype = getObjectType(object);

    switch(gctype) {

        case GCT_INSTANCE_CLASS:
        case GCT_ARRAY_CLASS:
            getClassName_inBuffer((CLASS)object, buffer);
            Log->heapObject((long)object, getObjectTypeStr(gctype),
                             getObjectSize(object),
                             (*header & MARKBIT),
                             buffer, 1);
            break;

        case GCT_INSTANCE: {
            /*  The object is a Java object instance.  Mark pointer fields */
            INSTANCE thisObject = (INSTANCE)object;
            INSTANCE_CLASS thisClass = thisObject->ofClass;
            getClassName_inBuffer((CLASS)thisClass, buffer);
            Log->heapObject((long)object, getObjectTypeStr(gctype),
                             getObjectSize(object),
                             (*header & MARKBIT),
                             buffer, 0);
            break;
        }

        default:
            Log->heapObject((long)object, getObjectTypeStr(gctype),
                             getObjectSize(object),
                             (*header & MARKBIT),
                             NULL, 0);
    }
}

void printHeapContents(void)
{
    cell* scanner = HeapSpace;
    cell* heapSpaceTop = HeapSpaceTop;
    int   objectCounter  = 0;
    int   objectSize     = 0;
    int   garbageCounter = 0;
    int   garbageSize    = 0;
    int   garbageCounterX= 0;
    int   garbageSizeX   = 0;
    int   largestFree    = 0;
    Log->startHeapScan();
    for ( ; scanner < heapSpaceTop; scanner += SIZE(*scanner) + HEADERSIZE) {
        int  size = SIZE(*scanner);
        cell *object = scanner + HEADERSIZE;
        printObject(object);
        if (TYPE(*scanner) != GCT_FREE) {
            objectCounter++;
            objectSize += size + HEADERSIZE;
        } else {
            garbageCounter++;
            garbageSize += size + HEADERSIZE;
            if (size > largestFree) {
                largestFree = size;
            }
        }
    }
    Log->endHeapScan();
    Log->heapSummary(objectCounter, garbageCounter,
                    objectSize*CELL, garbageSize*CELL,
                    largestFree*CELL,
                    (long)HeapSpace, (long)HeapSpaceTop);

    /*  Check that free list matches with heap contents */
    for (scanner = (cell*)FirstFreeChunk; scanner != NULL;
         scanner = (cell*)(((CHUNK)scanner)->next)) {
        garbageCounterX++;
        garbageSizeX +=  SIZE(*scanner) + HEADERSIZE;
    }
    if (garbageCounter != garbageCounterX || garbageSize != garbageSizeX) {
        Log->heapWarning((long)garbageCounterX, (long)garbageSizeX * CELL);
    }
    Log->fprintf(stdout, "====================\n\n");
}

void printHeapStatistics(void) {
    int sizes[(GCT_LASTVALIDTAG/GCT_FIRSTVALIDTAG) + 1];
    int counts[(GCT_LASTVALIDTAG/GCT_FIRSTVALIDTAG) + 1];
    int totalSize = 0;
    int totalCount = 0;
    int largestFree = 0;
    int i;

    cell* scanner = HeapSpace;
    cell* heapSpaceTop = HeapSpaceTop;

    memset(sizes, 0, sizeof(sizes));
    memset(counts, 0, sizeof(sizes));

    for ( ; scanner < heapSpaceTop; scanner += SIZE(*scanner) + HEADERSIZE) {
        int size = SIZE(*scanner);
        int type = TYPE(*scanner);
        counts[type]++;
        sizes[type] += size;
        if (type != 0) {
            totalCount++;
            totalSize += size;
        } else {
            if (size > largestFree) largestFree = size;
        }
    }
    fprintf(stdout, "%18s  %6s %6s\n", "Type", "Size", "Count");
    for (i = 0; i < (sizeof(sizes)/sizeof(sizes[0])); i++) {
        fprintf(
            stdout, "%18s: %6d %6d\n", getObjectTypeStr(i),
                                       sizes[i], counts[i]);
    }
    fprintf(stdout, "%18s: %6d %6d\n", "Total", totalSize, totalCount);
    fprintf(stdout, "%18s: %6d\n", "Largest Free", largestFree);
}

#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * End of dynamic heap management operations
 *=======================================================================*/

/*=========================================================================
 * Auxiliary memory management operations (both static and dynamic heap)
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getObjectSize()
 * TYPE:          public helper function
 * OVERVIEW:      Returns the size of the given heap object (in CELLs).
 * INTERFACE:
 *   parameters:  an object pointer
 *   returns:     the size of the object in cells (excluding the header).
 *=======================================================================*/

unsigned long getObjectSize(cell* object)
{
    /*  Calculate the address of the object header */
    cell* header = object-HEADERSIZE;

    /*  Get the size of object by shifting the type  */
    /*  information away from the first header word */
    return ((unsigned long)*header) >> TYPEBITS;
}

/*=========================================================================
 * FUNCTION:      getObjectType()
 * TYPE:          public helper function
 * OVERVIEW:      Returns the garbage collection type (GCT_*)
 *                of the given heap object.
 * INTERFACE:
 *   parameters:  an object pointer
 *   returns:     GC type
 *=======================================================================*/

GCT_ObjectType getObjectType(cell* object)
{
    /*  Calculate the address of the object header */
    cell* header = object-HEADERSIZE;

    /*  Get the type of object by masking away the */
    /*  size information and the flags */
    return TYPE(*header);
}

/*=========================================================================
 * Global garbage collection operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      InitializeGlobals()
 * TYPE:          public global operation
 * OVERVIEW:      Initialize all the globals variables. This ensures that
 *                they have correct values in each invocation of the VM.
 *                DON'T rely on static initializers instead. Any new
 *                globals added to the system should be initialized here.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

void InitializeGlobals(void)
{
}

/*=========================================================================
 * FUNCTION:      InitializeMemoryManagement()
 * TYPE:          public global operation
 * OVERVIEW:      Initialize the system memory heaps.
 * INTERFACE:
 *   parameters:  DEFAULTHEAPSIZE (defined in global.h)
 *   returns:     <nothing>
 *=======================================================================*/

void* TheHeap;

void InitializeMemoryManagement(void)
{
    int index;
    HeapSize = MAXIMUMHEAPSIZE;
    HeapSpace = allocateHeap(&HeapSize, &TheHeap);
    HeapSpaceTop =  HeapSpace + (HeapSize >> log2CELL);
    if (TheHeap == NIL)
        fatalVMError("Not enough memory to run VM!");

    if (TRACEGARBAGECOLLECTION || Verbose_On) {
        fprintf(stdout,"GC heap size: %ld bytes\n", (long)HeapSize);
    }

#if !CHUNKY_HEAP
    if (OVERWRITE_FREE_MEMORY) {
        memset(HeapSpace, OVERWRITE_FREE_MEMORY_BYTE, HeapSize);
    }

    /*  Create the first free list chunk */
    FirstFreeChunk = (CHUNK)HeapSpace;

    /*  The size of the first free chunk is the size of the heap (in cells) */
    /*  minus the number of cells required for a chunk header */
    FirstFreeChunk->size = (HeapSize / CELL - HEADERSIZE) << TYPEBITS;
    FirstFreeChunk->next = NIL;
#endif

    /* This is a "root", in addition to those in the Global Roots. */
    AllThreads = NULL;

    /* Create the global roots */
    GlobalRoots =
        (POINTERLIST)mallocObject(SIZEOF_POINTERLIST(12), GCT_GLOBAL_ROOTS);
    index = 0;
    GlobalRoots->data[index++].cellpp = (cell **)&GlobalRoots;
    GlobalRoots->data[index++].cellpp = (cell **)&TemporaryRoots;
    GlobalRoots->data[index++].cellpp = (cell **)&TransientRoots;
#if CHUNKY_HEAP
    GlobalRoots->data[index++].cellpp = (cell **)&ChunkyHeapPinnedItems;
#endif

    GlobalRoots->length = index;

    /*  Create the temporary root list with initial space for 6 pointers */
    TemporaryRoots = (POINTERLIST)mallocObject(SIZEOF_POINTERLIST(6),
                                               GCT_POINTERLIST);
    TemporaryRoots->length = 0;

    /* Note that the length of TransientRoots is set to the full length.
     * We specifically initialize everything to NULL.
     */
    TransientRoots = (POINTERLIST)callocObject(SIZEOF_POINTERLIST(6),
                                                  GCT_POINTERLIST);
    TransientRoots->length = 6;

    if (TRACEMEMORYALLOCATION) {
        Log->allocateHeap(HeapSize,
                          (long)HeapSpace,
                          (long)HeapSpaceTop);
    }
}

void FinalizeMemoryManagement(void) {
    /* printHeapStatistics(); */
	fprintf(stderr,"JVM:GC:starting finalizing\n");
    if (TheHeap != NIL) {
        /** REINIT ***/
        /* Set all the roots to NULL, in case we are reinitialized */

        POINTERLIST roots = GlobalRoots;
        cell ***ptr = &roots->data[0].cellpp;
        int length = roots->length;

        while (--length >= 0) {
            cell **address = *(ptr++);
            *address = NULL;
        }
        freeHeap(TheHeap);
        TheHeap = NIL;
    }
	fprintf(stderr,"JVM:GC:finalized\n");
    if (USESTATIC) {
        FinalizeStaticMemory();
		fprintf(stderr,"JVM:GC:static memory finalized\n");
    }
}


/*=========================================================================
 * FUNCTION:      checkHeap()
 * TYPE:          public debugging operation
 * OVERVIEW:      Verify that the heap is consistent. That is, all
 *                objects/free chunks are contiguous.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     the heap is consistent
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void checkHeap(void)
{
    /*  Scan the entire heap, looking for badly formed headers */
    cell* scanner = HeapSpace;
    cell* heapSpaceTop = HeapSpaceTop;
    int result = TRUE;

    while (scanner < heapSpaceTop) {
        long size = SIZE(*scanner);
        GCT_ObjectType type = TYPE(*scanner);

        /*  A valid header will always have a non-zero size */
        if (size == 0) {
            result = FALSE;
            break;
        }

        /*  Check if the scanner is pointing to a badly formed object header */
        if (!ISFREECHUNK(*scanner) && (type < GCT_FIRSTVALIDTAG ||
                                       type > GCT_LASTVALIDTAG)) {
            result = FALSE;
            break;
        }
        if (ISMARKED(*scanner)) {
            result = FALSE;
            break;
        }

        /*  Jump to the next object/chunk header in the heap */
        scanner += size + HEADERSIZE;
    }

    /*  The scanner should now be pointing to exactly the heap top */
    if (result == FALSE || scanner != heapSpaceTop) {
        fatalVMError("Heap is corrupted");
    }
}

#endif /*  INCLUDEDEBUGCODE */


/*=========================================================================
 * Deferred objects, as part of markChildren.
 *
 * These functions create a queue of objects that are waiting to be
 * scanned so that their children can be marked.  The active part of the
 * starts at startDeferredObjects and ends just before endDeferredObjects.
 * This queue is circular.
 *
 * The two main functions are putDeferredObject() and getDeferredObject()
 * which add and remove objects from the queue.  putDeferredObject()
 * correctly handles overflow.  getDeferredObject() presumes that the user
 * has first checked that there is an object to be gotten.
 *=======================================================================*/


/*=========================================================================
 * FUNCTION:      initializeDeferredObjectTable
 * TYPE:          scanning the heap
 * OVERVIEW:      Initialize the deferred object table so that it correctly
 *                behaves like a queue.
 * INTERFACE:
 *   parameters:  <none>
 *=======================================================================*/

static void initializeDeferredObjectTable(void) {
    startDeferredObjects = endDeferredObjects = deferredObjectTable;
    deferredObjectCount = 0;
    deferredObjectTableOverflow = FALSE;
}

/*=========================================================================
 * FUNCTION:      putDeferredObject
 * TYPE:          scanning the heap
 * OVERVIEW:      Add an object to the end of the queue.  If the queue is
 *                full, just set the variable deferredObjectTableOverflow
 *                and ignore the request.
 * INTERFACE:
 *   parameters:  value: An object to put on the queue
 *=======================================================================*/

static void
putDeferredObject(cell *value)
{
    if (deferredObjectCount >= DEFERRED_OBJECT_TABLE_SIZE) {
        deferredObjectTableOverflow = TRUE;
    } else {
        if (endDeferredObjects == endDeferredObjectTable) {
            endDeferredObjects = deferredObjectTable;
        }
        *endDeferredObjects++ = value;
        deferredObjectCount++;
    }
#if ENABLEPROFILING
    if (deferredObjectCount > MaximumGCDeferrals) {
        MaximumGCDeferrals = deferredObjectCount;
    }
    TotalGCDeferrals++;
#endif
}

/*=========================================================================
 * FUNCTION:      getDeferredObject
 * TYPE:          scanning the heap
 * OVERVIEW:      Get the next object from the front of the queue.
 *                The user should check the value of deferredObjectCount
 *                before calling this function.
 * INTERFACE:
 *   parameters:  none
 *   returns:     The next object on the queue.
 *=======================================================================*/

static cell*
getDeferredObject(void)
{
    if (startDeferredObjects == endDeferredObjectTable) {
        startDeferredObjects = deferredObjectTable;
    }
    deferredObjectCount--;
    return *startDeferredObjects++;
}

