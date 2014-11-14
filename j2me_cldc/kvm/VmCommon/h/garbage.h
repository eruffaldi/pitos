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
 * FILE:      garbage.h
 * OVERVIEW:  Memory manager/garbage collector for the KVM
 *            (2nd implementation; intended for use on devices with
 *            strict memory restrictions such as the PalmPilot).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (made collection more exact)
 * NOTE (by Doug):
 *            The garbage collector is now completely safe. That is, only
 *            values that are pointers to objects on the stack are passed
 *            to scanObject. Note that this is still not 100% exactness
 *            as one of these values may well be integer that just happens
 *            to point to a heap object when treated as a pointer.
 *            Theoretically, the test on a value whose type is unknown
 *            (which is only local variables and stack operands in this VM)
 *            can be quite expensive given that it will at most traverse
 *            each object/free chunk on the heap. In practice, if the given
 *            value is not an object pointer, the test is usually quite fast.
 *            The test is significantly slower if the object is a pointer.
 *=======================================================================*/

/*=========================================================================
 * COMMENTS ON THE NEW GARBAGE COLLECTOR:
 * The original KVM garbage collector was based on Cheney's
 * elegant copying collector presented in Communications of the ACM
 * in September 1970.  Although that collector has a lot of benefits
 * (in particular, it uses an iterative rather than recursive algorithm
 * for collecting objects), it has the general problem of copying
 * collectors in that it requires double the memory space than the
 * program is actually using.  That requirement was problematic
 * when porting the KVM to run on devices with very limited
 * memory such as the PalmPilot.
 *
 * In order to make the KVM more suitable to small
 * devices, a new garbage collector has been implemented. The
 * new collector is based on a straightforward, handle-free,
 * non-moving, mark-and-sweep algorithm. Since the new collector
 * does not move objects, it is somewhat simpler than the
 * original collector. However, since the new collector
 * maintains free lists of memory rather than copying and/or
 * compacting memory, object allocation is not as fast as
 * in the original collector, and memory fragmentation can
 * cause the the VM run out of memory easier than with a copying
 * collector. Also, since the marking phase of the new collector
 * uses a recursive (rather than iterative) algorithm, the
 * collector can potentially run out of C stack space when
 * collecting highly nested data structures.
 *
 * In general, the new garbage collector is intended for use
 * only on devices that have very strict memory limitations
 * such as the PalmPilot. For devices with less restrictions,
 * the original copying collector should be used since it
 * provides various other benefits, such as the ability to
 * grow and shrink heap space dynamically at runtime.
 *
 * Note that unlike the original copying collector, the new
 * collector also supports garbage collection of classes and
 * other static structures. This can potentially be very valuable
 * in some embedded applications in which the VM must be able to
 * dispose of classes at runtime without having the reinitialize
 * the whole virtual machine.
 *
 * The current implementation of the new collector is still non-
 * incremental, i.e., the execution of the program is stopped
 * whenever a garbage collection takes place. However, the
 * algorithm could be changed to support incremental collection
 * relatively easily and we plan to do this in the future.
 *
 *=========================================================================
 * JAVA-SPECIFIC COMMENTS:
 * Java's primitive data types (such as int, float, long, double)
 * typically require full precision of machine-level registers
 * and memory locations. Thus, it is generally impossible to use
 * a tagged, language-independent memory system in conjunction with
 * Java.  This implies that a garbage collector for a Java VM must
 * always receive some assistance from the Java type system in order
 * to be able to perform exact garbage collection. Otherwise the
 * garbage collector would have to guess which fields in objects
 * contain pointers and which primitive values -- in fact, guessing
 * is precisely what some Java VM's with "conservative" (= imprecise)
 * garbage collectors do.
 *
 * In our VM, the support for exact garbage collection has been
 * built inside the memory system. Upon object allocation, each
 * newly created object is augmented with an object header that
 * stores information about the type of the object, among other
 * things. The garbage collector then utilizes this information
 * and performs different kinds of pointer scans for different
 * kinds of objects. The result is a precise, predictable, yet
 * rather efficient garbage collection system.
 *
 * Unfortunately, our current garbage collector isn't 100% exact.
 * Since many Java classfiles do not contain type information on
 * local variables and operand stack contents, we have to use
 * heuristics for scanning certain parts of execution stack frames.
 * While the collector may not be 100% exact, it is safe. That is,
 * an object pointer will always be recognized as such whereas
 * a number may be mistaken as an object pointer.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Header word definitions and structures
 *=======================================================================*/

/*=========================================================================
 * OBJECT STRUCTURES:
 * In our VM, each heap-allocated structure in memory is preceded with
 * an object header that provides detailed information on the type and
 * size of the object. The length of the header is one word (32 bits).
 *
 * The basic heap object structure is as follows:
 *
 *                   +---------------+
 *                   ! header word 1 ! (object length, type and other info)
 *                   +---------------+
 * Object pointer -> !               !    ^
 *                   +---------------+    !
 *                   .               .    !
 *                   .               .  data
 *                   .               .    !
 *                   !               !    !
 *                   +---------------+    !
 *                   !               !    v
 *                   +---------------+
 *
 * IMPORTANT:
 * Note that there are no object handles in our VM. Thus, unlike in
 * many other JVM implementations, all the memory references are
 * direct rather than indirect.
 *
 * It is important to notice that references are only allowed to
 * memory locations that are immediately preceded by an object header;
 * this means that you may not create a pointer that refers, e.g., to
 * data fields of an object directly. This restriction simplifies the
 * garbage collection process substantially (but could be removed
 * relatively easily if necessary).
 *=========================================================================
 * OBJECT HEADERS:
 * An object header is a 32-bit structure that contains important
 * administrative information. In particular, the header stores
 * the length of the object, the type of the object and two
 * bits for administrative purposes.
 *                                                             +-- Static bit
 *                                                             | +-- Mark bit
 * <------------------ length -------------------> <-- type--> v v
 *|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|
 *| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |0|0|
 *|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-|
 *
 * Length holds the length of the object in cells, excluding the
 * object header. For instance, length = 1 means that the data
 * area of the object is 4 bytes.  The minimum physical size of
 * an object is 8 bytes (4 byte header + 4 byte data).
 *
 * Type field holds the garbage collection type of the object,
 * instructing the garbage collector to scan the data fields of
 * the object correctly upon garbage collection. Garbage collection
 * types (GCT_*) are defined below.
 *
 * Static bit is unused in the 2nd generation garbage collector.
 * In the original KVM collector this bit was used for
 * denoting that an object was allocated in the static heap,
 * i.e., the object was immovable.
 *
 * Mark bit is used for marking live (non-garbage) objects during
 * garbage collection. During normal program execution this bit
 * should always be 0 in every heap object.
 *
 * Note that unlike the original copying collector, the second
 * generation KVM garbage collector does not use identity
 * fields. In the original copying collector every object header
 * also contained an identity field to store object identity.
 * This field was necessary, because in a copying collector
 * the physical address of the object can change and thus it
 * cannot be used for identifying an object uniquely.  In the
 * new collector objects don't move, and thus no identity fields
 * are needed.
 *=======================================================================*/

#define MARKBIT         0x00000001
#define STATICBIT       0x00000002
#define TYPEMASK        0x000000FC

/*  HEADER */
/*  struct headerStruct { */
/*  int size      :24; */
/*  int type      :6; */
/*  int markBit   :1; */
/*  int staticBit :1; */
/*  }; */

/*  The amount of bits that will have to be shifted away
 *  in order to read the object length correctly
 */

#define TYPEBITS        8

/* The number of bits that we have to shift to get the type */
#define TYPE_SHIFT      2

/*=========================================================================
 * Operations on header words
 *=======================================================================*/

#define ISMARKED(n)     ((n) & MARKBIT)
#define ISSTATIC(n)     ((n) & STATICBIT)
#define SIZE(n)         (((cell)(n)) >> TYPEBITS)
#define TYPE(n)         (GCT_ObjectType)(((n) & TYPEMASK) >> TYPE_SHIFT)
#define ISFREECHUNK(n)  (((n) & (TYPEMASK | MARKBIT | STATICBIT)) == 0)

/*  Header is 1 word long */
#define HEADERSIZE      1

/*=========================================================================
 * FREE LIST STRUCTURES:
 * The new garbage collector maintains a free list of available memory.
 * Every available memory chunk is preceded by a free list header with
 * the following information:
 *
 *     +--------------+
 *     !     size     ! (the amount of memory in this chunk)
 *     +--------------+
 *     !     next     ! (pointer to the next chunk in the free list)
 *     +--------------+
 *
 * The size information is stored in cells (32 bit words). The size
 * information is stored in the same format as in regular object headers,
 * i.e., only the highest 24 bits are used (see above). The 6 low bits
 * must all be zero for a word to be recognized as a chunk header. The size
 * excludes the size field (HEADERSIZE) itself, i.e., if the physical
 * size of the chunk is 3 words, the size field actually contains 2.
 *
 * As obvious, the next field is NIL (0) if this is the last
 * free chunk in memory.
 *
 * Chunks are always at least 2 words (64 bits) long, because this
 * is the minimum size needed for allocating new objects. Free memory
 * areas smaller than this are automatically merged with other objects
 * (thus creating permanent garbage). In practice this happens rarely,
 * though.
 *=======================================================================*/

/*  CHUNK */
struct chunkStruct {
    long  size;     /*  The size of the chunk in words (including the header) */
    CHUNK next;     /*  Pointer to the next chunk (NIL if none) */
};

/*=========================================================================
 * As a Palm specific option, the static portion of a Palm device's memory
 * can be used to store relatively static structures such as the constant
 * pool and code of a class. By storing these structures in static memory
 * a substantial amount of room is freed up for use by the garbage
 * collected heap. Given that we expect anything put into static memory to
 * live for the lifetime of a VM execution, we can implement the management
 * of any static memory chunks simply as a list of chunks that are all
 * freed upon the end of execution. As such the basic structure of a
 * statically allocated chunk will be as follows:
 *
 *                   +---------------+
 *                   ! prev chunk ptr! (bit 0: bump bit)
 *                   +---------------+
 * Object pointer -> !               !    ^
 *                   +---------------+    !
 *                   .               .    !
 *                   .               .  data
 *                   .               .    !
 *                   !               !    !
 *                   +---------------+    !
 *                   !               !    v
 *                   +---------------+
 *
 * These chunks are allocated via the mallocStaticBytes operation and are
 * all collected at once by the FinalizeStaticMemory operation.
 *=======================================================================*/

/* Since the PalmOS memory returns two-byte aligned addresses, */
/* we must occasionally bump the statically allocated object */
/* addresses upwards in the memory (by two bytes). This is  */
/* indicated by raising a special "bump bit" stored in the */
/* least significant bit in the previous chunk pointer field  */
/* (shown above). */

#define STATICBUMPBIT  0x00000001
#define STATICBUMPMASK 0xFFFFFFFE


/*=========================================================================
 * Garbage collection types of objects
 *=========================================================================
 * These types are used for instructing the garbage collector to
 * scan the data fields of objects correctly upon garbage collection.
 * Since two low-end bits of the first header field are used for
 * flags, we don't use these in type declarations.
 *=======================================================================*/

typedef enum {
    GCT_FREE = 0,
    /*  Objects which have no pointers inside them */
    /*  (can be ignored safely during garbage collection) */
    GCT_NOPOINTERS,

    /*  Java-level objects which may have mutable pointers inside them */
    GCT_INSTANCE,
    GCT_INSTANCE_OF_CLASS,      /* an object of type java.lang.Class */
    GCT_ARRAY,
    GCT_OBJECTARRAY,

    /*  Internal VM objects which may have mutable pointers inside them */
    GCT_POINTERLIST,
    GCT_INSTANCE_CLASS,
    GCT_ARRAY_CLASS,
    GCT_FIELDTABLE,
    GCT_METHODTABLE,
    GCT_CONSTANTPOOL,
    GCT_INLINECACHE,
    GCT_EXECSTACK,
    GCT_THREAD,
    GCT_MONITOR,
    GCT_GLOBAL_ROOTS,
    GCT_HASHTABLE,
    GCT_UTF_HASH_ENTRY,
    GCT_STRING_HASH_ENTRY
} GCT_ObjectType;

#define GCT_FIRSTVALIDTAG GCT_NOPOINTERS
#define GCT_LASTVALIDTAG  GCT_STRING_HASH_ENTRY

#define GCT_TYPENAMES  { \
        "FREE",          \
        "NOPOINTERS",    \
        "INSTANCE",      \
        "INSTANCE OF CLASS", \
        "ARRAY",         \
        "OBJECTARRAY",   \
        "POINTERLIST",   \
        "INSTANCE CLASS",\
        "ARRAY CLASS",   \
        "FIELDTABLE",    \
        "METHODTABLE",   \
        "CONSTANTPOOL",  \
        "INLINECACHE",   \
        "EXECSTACK",     \
        "THREAD",        \
        "MONITOR",       \
        "GLOBAL_ROOTS",  \
        "HASH TABLE",    \
        "UTF HASH ENTRY", \
        "STRING HASH ENTRY"  \
    }


#define inHeapSpace(ptr) (((cell *)ptr >= HeapSpace) && ((cell *)ptr < HeapSpaceTop))



/*=========================================================================
 * Dynamic heap variables
 *=======================================================================*/

extern long   HeapSize;         /*  Heap size */
extern cell* HeapSpace;        /*  Heap bottom */
extern cell* HeapSpaceTop;          /*  Current heap top */

/*  First free chunk in the free list */
extern CHUNK FirstFreeChunk;

/*  A table for storing temporary root objects */
/*  for the garbage collector */
extern POINTERLIST TemporaryRoots;
extern POINTERLIST TransientRoots;
extern POINTERLIST GlobalRoots;

/*=========================================================================
 * Dynamic heap operations (private)
 *=======================================================================*/

/*  Internal memory allocation operations */
cell* allocateFreeChunk(long size);

/*=========================================================================
 * Dynamic heap operations (public)
 *=======================================================================*/

/*  Memory allocation operations */
cell* mallocHeapObject(long size, GCT_ObjectType type);

cell* mallocObject(long size, GCT_ObjectType type);
cell* callocObject(long size, GCT_ObjectType type);
char* mallocBytes(long size);
cell* reallocObject(cell* object, long newSize);

/*  Garbage collection operations */
void  garbageCollect(int moreMemory);

/* Handling of temporary roots */
#ifdef INCLUDEDEBUGCODE 
# define VERIFY_INSIDE_TEMPORARY_ROOTS (_tmp_roots_ = _tmp_roots_)
# define INDICATE_DYNAMICALLY_INSIDE_TEMPORARY_ROOTS int _tmp_roots_ = 0;
#else 
# define VERIFY_INSIDE_TEMPORARY_ROOTS 
# define INDICATE_DYNAMICALLY_INSIDE_TEMPORARY_ROOTS
#endif

#define START_TEMPORARY_ROOTS   { int _tmp_roots_ = TemporaryRoots->length; 

#define START_TEMPORARY_ROOT(x){ int _tmp_roots_ = TemporaryRoots->length; \
                                 MAKE_TEMPORARY_ROOT(x);

#define MAKE_TEMPORARY_ROOT(x)   (VERIFY_INSIDE_TEMPORARY_ROOTS, \
                                  makeTemporaryRoot((cell *)(x)))

#define END_TEMPORARY_ROOTS      TemporaryRoots->length = _tmp_roots_;  }

#define END_TEMPORARY_ROOT       TemporaryRoots->length = _tmp_roots_;  }



void  makeTemporaryRoot(cell* object);
void  makeGlobalRoot(cell** object);
int   makeTransientRoot(cell* object);

void removeTransientRootByValue(cell *value);
void removeTransientRootByIndex(int index);


int garbageCollecting(void);


/*  Printing and debugging operations */
long   getHeapSize(void);
long   memoryFree(void);
long   memoryUsage(void);

#if INCLUDEDEBUGCODE
void  printHeapContents(void);
#else
#define printHeapContents()
#endif

/*=========================================================================
 * Auxiliary memory management operations
 *=======================================================================*/

cell align(cell address);

/*  Operations on individual heap objects */
unsigned long  getObjectSize(cell* object);
GCT_ObjectType getObjectType(cell* object);



/*=========================================================================
 * Memory management operations on static memory
 *=======================================================================*/

/* Memory allocation operations */

/* Memory deallocation operations */
#if USESTATIC
void FinalizeStaticMemory(void);
void modifyStaticMemory(void *staticMemory, int offset, void *newVal, int size);
void *mallocStaticBytes(int size);
#else
#define FinalizeStaticMemory()
#define modifyStaticMemory(staticMemory, offset, newVal, size) \
           (((void)offset), ((void)newVal))
#define mallocStaticBytes(size)
#endif

/*=========================================================================
 * Global garbage collection operations
 *=======================================================================*/

void InitializeGlobals(void);
void InitializeMemoryManagement(void);
void FinalizeMemoryManagement(void);

#if INCLUDEDEBUGCODE
void checkHeap(void);
void printObject(cell *object);
#else
#define checkHeap()
#define printObject(object);
#endif

#if CHECKMEMORYWRITE
#define VALIDATE_WRITE_ADDRESS(a) \
    if ((cell*)(a) < HeapSpace || (cell*)(a) >= HeapSpaceTop) { \
        fatalVMError("attempting to write out of heap boundary"); \
    } else
#else
#define VALIDATE_WRITE_ADDRESS(a)
#endif


/*=========================================================================
 * Functions that are only called if you are using ROM
 *=======================================================================*/


#if ROMIZING
     /* Initialize the ROM image.  Clean up the ROM image when you're done */
     extern void InitializeROMImage(void);
     extern void FinalizeROMImage(void);

      /* The pointer to the all the static variables that need to be relocated.
       * It's a pointer when using relocation rom, and an actual array when
       * using fixed rom */
#    if RELOCATABLE_ROM
        extern long *KVM_staticDataPtr;
#    else
        extern long KVM_staticData[];
#   endif
#else 
    #define InitializeROMImage()
    #define FinalizeROMImage()
#endif

/* This function creates the ROM image out of its relocatable pieces */

#if ROMIZING && RELOCATABLE_ROM
    void CreateROMImage(void);
    void DestroyROMImage(void);
#else 
#   define CreateROMImage()
#   define DestroyROMImage()
#endif


/* When dealing with relocatable, preloaded classes, we can read preloaded
 * memory directly, but we have to use special functions to write them.
 *
 * We must call the following functions when there is a chance that the 
 * object/class/method might be part of a preloaded class.
 */

#if ROMIZING && RELOCATABLE_ROM
    void setObjectMonitor(OBJECT object, MONITOR mid);
    void setClassInitialThread(INSTANCE_CLASS clazz, THREAD thread);
    void setClassStatus(INSTANCE_CLASS clazz, int status);
#else 

#   define setObjectMonitor(object,  mid)   ((object)->monitor = (mid))
#   define setClassInitialThread(iclazz, thread) \
                                            ((iclazz)->initThread = (thread))
#   define setClassStatus(iclazz, stat)   ((iclazz)->status = (stat))
#endif

#if CHUNKY_HEAP
extern POINTERLIST ChunkyHeapPinnedItems;
#endif
