/*
 * Copyright (c) 1998-2000 Sun Microsystems, Inc. All Rights Reserved.
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
 * SUBSYSTEM: Main program; compilation options
 * FILE:      main.h
 * OVERVIEW:  Compilation options setup for fine-tuning the system
 *            and for enabling/disabling various debugging/profiling/
 *            tracing options.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            (and many others since then...)
 *=======================================================================*/

/*=========================================================================
 * This file gives the >>DEFAULT<< values for various compile time flags.
 *
 * Any port can override a value given in this file by either:
 *      - Giving it that value in its port-specific "machine_md.h" file
 *      - Giving it a value in a compiler-specific manner (e.g., 
 *        as a command line parameter in a makefile.)
 *
 * The preferred way of changing the values defined in this file
 * is to edit the port-specific "machine_md.h" file rather than
 * change the default values here.
 *=======================================================================*/

/*=========================================================================
 * General compilation options
 *=======================================================================*/

/* 
 * Set this to zero if your compiler does not directly support 64-bit
 * integers.
 */
#ifndef COMPILER_SUPPORTS_LONG
#define COMPILER_SUPPORTS_LONG 1
#endif

/* 
 * Set this to a non-zero value if your compiler requires that 64-bit
 * integers be aligned on 0 mod 8 boundaries.
 */
#ifndef NEED_LONG_ALIGNMENT
#define NEED_LONG_ALIGNMENT 0
#endif

/* Set this to a non-zero value if your compiler requires that double
 * precision floating-pointer numbers be aligned on 0 mod 8 boundaries.
 */
#ifndef NEED_DOUBLE_ALIGNMENT
#define NEED_DOUBLE_ALIGNMENT 0
#endif

/*=========================================================================
 * General system configuration options
 * (Note: many of these flags are commonly overridden from makefiles)
 *=======================================================================*/

/*
 * Includes or excludes non-CLDC classes from the target system.
 * Turning this option on will include also those components
 * (e.g., graphics support, networking protocol implementations)
 * that are not part of the CLDC Specification.
 *
 * This flag is commonly overridden in the makefiles of
 * the Windows and Unix ports.
 */
#ifndef INCLUDE_ALL_CLASSES
#define INCLUDE_ALL_CLASSES 1
#endif

/* Indicates whether floating point operations are supported or not.
 * Should be 0 in CLDC Specification compliant implementations.
 */
#ifndef IMPLEMENTS_FLOAT
#define IMPLEMENTS_FLOAT 0
#endif

/* Indicates that this implementation uses loaderFile.c or a similar
 * implementation of reading files.
*/
#ifndef USES_CLASSPATH
#define USES_CLASSPATH 1
#endif

/*
 * Turns class prelinking/preloading (JavaCodeCompact) support on or off.
 * If this option is on, KVM can prelink system classes into the
 * virtual machine executable, speeding up VM startup considerably.
 * 
 * Note: this flag is commonly overridden from the makefile
 * in our Windows and Unix ports.
 */
#ifndef ROMIZING
#define ROMIZING 1
#endif

/*
 * Includes or excludes the optional Java Application Manager (JAM)
 * component in the virtual machine.  Refer to Porting Guide for
 * details.
 *
 * Note: this flag is commonly overridden from the makefile
 * in our Windows and Unix ports.
 */
#ifndef USE_JAM
#define USE_JAM 0 
#endif

/* Indicates that this port utilizes asynchronous native functions.
 * Refer to Porting Guide for details.
 */
#ifndef ASYNCHRONOUS_NATIVE_FUNCTIONS
#define ASYNCHRONOUS_NATIVE_FUNCTIONS 0
#endif

/*=========================================================================
 * Palm-related system configuration options
 *=======================================================================*/

/* This is a special hack intended only for the Palm and similar
 * implementations with limited dynamic memory.  Unix and Windows both
 * have "fakeStaticMemory.c", but that is intended only to help find bugs
 * in the Palm implementation.
 * 
 * If set to a non-zero value, the implementation will try to move field
 * tables, method tables, and other immutable structures from dynamic memory
 * to "static memory" (storage RAM on Palm), which is easy to read but more 
 * difficult to write. This frees up valuable space in dynamic RAM.
 */
#ifndef USESTATIC
#define USESTATIC 0
#endif

/* Instructs KVM to use an optimization which allows KVM to allocate
 * the Java heap in multiple chunks or segments. This makes it possible
 * to allocate more Java heap space on memory-constrained target 
 * platforms such as the Palm.
 */
#ifndef CHUNKY_HEAP
#define CHUNKY_HEAP 0
#endif

/* This is a special form of ROMIZING (JavaCodeCompacting) that is used 
 * only by the Palm. It allows the rom'ed image to be relocatable even 
 * after it is built.  The ROMIZING flag is commonly provided 
 * as a command line parameter from a makefile. 
 */
#ifndef RELOCATABLE_ROM
#define RELOCATABLE_ROM 0
#endif

/*=========================================================================
 * Memory allocation settings
 *=======================================================================*/

/*=========================================================================
 * COMMENT: The default sizes for overall memory allocation and
 * resource allocation for individual threads are defined here.
 *
 * As a general principle, KVM allocates all the memory it needs
 * upon virtual machine startup.  At runtime, all memory is allocated
 * from within these preallocated areas.  However, note that on 
 * many platforms the native functions linked into the VM (e.g., 
 * graphics operations) often perform dynamic memory allocation
 * outside the Java heap.
 *=======================================================================*/

/* The Java heap size that KVM will allocate upon virtual machine
 * startup (in bytes).  Note that the Palm implementation ignores
 * this value and uses an alternative memory allocation strategy.
 *
 * Also note that this definition is commonly overridden from
 * the makefile in the Windows and Unix ports.
 */
#ifndef MAXIMUMHEAPSIZE
#define MAXIMUMHEAPSIZE 65024   /* 0xFE00 */
#endif

/* Maximum size of the master inline cache (# of ICACHE entries) 
 * Remember that maximum size should not exceed 65535, because the 
 * length of inlined bytecode parameters is only two bytes (see cache.h).
 * This macro is meaningful only if the ENABLEFASTBYTECODES option
 * is turned on.   
 */
#ifndef INLINECACHESIZE
#define INLINECACHESIZE   40
#endif

/* The execution stacks of Java threads in KVM grow and shrink 
 * at runtime. This value determines the default size of a new
 * stack frame chunk when more space is needed.
 */
#ifndef STACKCHUNKSIZE
#define STACKCHUNKSIZE    64
#endif

/* The size (in bytes) of a statically allocated area that the
 * virtual machine uses internally in various string operations.
 */
#ifndef STRINGBUFFERSIZE
#define STRINGBUFFERSIZE 512
#endif

extern char str_buffer[];   /*  shared string buffer */

/*=========================================================================
 * Garbage collection options
 *=======================================================================*/

/*  This option turns on code in the garbage collector that ensures completely 
 *  safe garbage collection. This has been made a compile time option for 
 *  now as the extra safety may have a high performance cost. The 
 *  algorithm used to ensure safeness is described here so that the value of 
 *  TESTNONOBJECT_DEPTH is made clear. A test is done on values for which there 
 *  is no type information. At this stage, only local variables and stack
 *  operands fall into this category. For each of these values, a few quick
 *  heuristic tests are performed before doing a final extensive (and 
 *  potentially expensive) test. These initial tests include: 
 *     1) does it have the right alignment to be a heap addresses 
 *     2) is it not 0 (null pointers aren't scanned) 
 *     3) is the object header at the address pointed to well formed 
 *     4) has the object at this address already been scanned
 *     5) is it an address within the range of the heap 
 *  If all these tests succeed, the value is still not guaranteed to be an
 *  object pointer. 
 *  If it is not, then a short scan from the address pointed to should most 
 *  likely turn up an object with a badly formed header. The depth of this 
 *  scan is determined by the TESTNONOBJECT_DEPTH macro. Should this test 
 *  fail (i.e. the value could still be an object pointer), a 
 *  scan the heap from the bottom up to the address given by the value is 
 *  then performed which will confirm whether or not the value is an object
 *  pointer. Making TESTNONOBJECT_DEPTH large will slow down the test when
 *  the value is indeed a valid pointer. 
 */
#ifndef SAFEGARBAGECOLLECTION
#define SAFEGARBAGECOLLECTION 1
#endif

#ifndef TESTNONOBJECT_DEPTH
#define TESTNONOBJECT_DEPTH 3
#endif

/*
 * The following flag is for debugging. If non-zero, it'll cause a 
 * garbage collection to happen before every memory allocation. 
 * This will help find those pointers that you're forgetting to
 * lock!
 */
#ifndef EXCESSIVE_GARBAGE_COLLECTION
#define EXCESSIVE_GARBAGE_COLLECTION 0
#endif

/*=========================================================================
 * Interpreter execution options
 *=======================================================================*/

/* The value of this macro determines the basic frequency (as number
 * of bytecodes executed) by which the virtual machine performs thread 
 * switching, event notification, and some other periodically needed 
 * operations. A smaller number reduces event handling and thread
 * switching latency, but causes the interpreter to run slower.
 */
#ifndef BASETIMESLICE
#define BASETIMESLICE     500
#endif

/* Turns runtime bytecode replacement (and Deutsch-Schiffman style 
 * inline caching) on/off.  The system will run faster with fast 
 * bytecodes on, but needs a few kilobytes of extra space for
 * the additional code. Note that you can't use bytecode replacement
 * on those platforms in which bytecodes are stored in non-volatile
 * memory (e.g., ROM).
 */
#ifndef ENABLEFASTBYTECODES
#define ENABLEFASTBYTECODES 0
#endif

/* This option can be used for turning on/off constant pool integrity 
 * checking. When turned on, the system checks that the given constant 
 * pool references point to appropriate constant pool entries at 
 * runtime, thus verifying the structural integrity of the class.
 * As obvious, the system runs slightly faster with the integrity
 * checking turned off. 
 */
#ifndef VERIFYCONSTANTPOOLINTEGRITY
#define VERIFYCONSTANTPOOLINTEGRITY 1
#endif

/* This macro makes the virtual machine sleep when it has
 * no better things to do. The default implementation is
 * a busy loop. Most ports usually require a more efficient
 * implementation that allow the VM to utilize the host-
 * specific battery power conservation features.
 *
 */
#ifndef SLEEP_UNTIL
#   define SLEEP_UNTIL(wakeupTime)                              \
        for (;;) {                                              \
                ulong64 now = CurrentTime_md();                 \
                if (ll_compare_ge(now, wakeupTime)) {           \
                        break;                                  \
                }                                               \
    }
#endif

/*=========================================================================
 * Debugging and tracing options
 *=======================================================================*/

/* Includes/eliminates a large amount of optional debugging code 
 * (such as class and stack frame printing operations) in/from
 * the system.  By eliminating debug code, the system will be smaller.
 * Inclusion of debugging code increases the size of the system but 
 * does not introduce any performance overhead unless calls to the
 * debugging routines are added to the source code or various tracing
 * options are turned on.
 */
#ifndef INCLUDEDEBUGCODE
#define INCLUDEDEBUGCODE 0
#endif

/* Turns VM execution profiling and additional safety/sanity 
 * checks on/off.
 */
#ifndef ENABLEPROFILING
#define ENABLEPROFILING 0
#endif

/*=========================================================================
 * Compile-time flags for choosing different tracing/debugging options.
 * All these options make the system very verbose. Turn them all off
 * for normal VM operation.  When turning them on, it is useful to
 * turn INCLUDEDEBUGCODE on, too, since many of the routines utilize
 * debugging code to provide more detailed information to the user.
 *=======================================================================*/

/* Prints out less verbose messages on small machines */
#ifndef TERSE_MESSAGES
#define TERSE_MESSAGES 0
#endif

/*  Turns memory allocation tracing on/off */
#ifndef TRACEMEMORYALLOCATION
#define TRACEMEMORYALLOCATION 0
#endif

/*  Turns garbage collection tracing on/off */
#ifndef TRACEGARBAGECOLLECTION
#define TRACEGARBAGECOLLECTION 0
#endif
#ifndef TRACEGARBAGECOLLECTIONVERBOSE
#define TRACEGARBAGECOLLECTIONVERBOSE 0
#endif

/*  Turns tracing of class loading on/off */
#ifndef TRACECLASSLOADING
#define TRACECLASSLOADING 0
#endif
#ifndef TRACECLASSLOADINGVERBOSE
#define TRACECLASSLOADINGVERBOSE 0
#endif

/*  Turns thread creation/switching/activation/suspension tracing on/off */
#ifndef TRACETHREADING
#define TRACETHREADING 0
#endif

/*  Turns bytecode execution tracing on/off */
#ifndef TRACEBYTECODES
#define TRACEBYTECODES 0
#endif

/* Turns method call tracing on/off.  
 * If this value is >= 2, then a >>lot<< more information is printed. 
 */
#ifndef TRACEMETHODCALLS
#define TRACEMETHODCALLS 0
#endif

/*  Turns verifier tracing on/off */
#ifndef TRACEVERIFIER
#define TRACEVERIFIER 0
#endif

/*  Turns exception handling tracing on/off */
#ifndef TRACEEXCEPTIONS
#define TRACEEXCEPTIONS 0
#endif

/*  Turns event handling tracing on/off */
#ifndef TRACEEVENTS
#define TRACEEVENTS 0
#endif

/*  Turns monitor wait list addition/release tracing on/off */
#ifndef TRACEMONITORS
#define TRACEMONITORS 0
#endif

/* Trace stack chunks being allocated and deallocated */
#ifndef TRACE_STACK_CHUNKS
#define TRACE_STACK_CHUNKS 0
#endif

/* Trace stack frames being pushed and popped.  Gives more information
 * than just TRACEMETHODCALLS */
#ifndef TRACE_FRAMES
#define TRACE_FRAMES 0
#endif

/*=========================================================================
 * Networking, storage and event handling options (using Generic Connections)
 *=======================================================================*/

/*
 * Allows the VM to utilize the CLDC Generic Connection mechanism
 * in supporting event handling.
 */
#ifndef GENERICEVENTS
#define GENERICEVENTS 1
#endif

/*
 * Turns on support for networking through Generic Connections.
 */
#ifndef GENERICNETWORK
#define GENERICNETWORK 0
#endif

/*
 * Turns on an experimental mechanism for supporting persistent
 * storage in a generalized fashion.
 */
#ifndef GENERICSTORAGE
#define GENERICSTORAGE 0
#endif

/*=========================================================================
 * Error handling macros
 *=======================================================================*/

#ifndef ERROR_TRY
#  include <setjmp.h>
#  define ERROR_TRY  \
    {  jmp_buf *__prev__ = topJumpBuffer;                       \
           int *__prev__value = topJumpBuffer_value;            \
           jmp_buf __jmp_buf__;                                 \
           int __value__;                                       \
       int __state__;                                           \
       topJumpBuffer = &__jmp_buf__;                            \
       topJumpBuffer_value = &__value__;                        \
       if ((__state__ = setjmp(__jmp_buf__)) == 0) {

#  define ERROR_CATCH(var)                                      \
        }                                                       \
        topJumpBuffer = __prev__;                               \
        topJumpBuffer_value = __prev__value;                    \
        if (__state__ != 0) {                                   \
            long var = __value__;

#  define ERROR_END_CATCH     } }

#  define ERROR_THROW(i)                                        \
        *topJumpBuffer_value = i;                               \
        longjmp(*topJumpBuffer, i)

extern jmp_buf *topJumpBuffer;
extern int     *topJumpBuffer_value;

#define NEED_GLOBAL_JUMPBUFFER 1
#endif

#ifndef FATAL_ERROR_EXIT_CODE
#define FATAL_ERROR_EXIT_CODE 127
#endif

/*=========================================================================
 * Miscellaneous macros and options
 *=======================================================================*/

/* Some functions in the reference implementation take arguments
 * that they do not use. Some compilers will issue warnings; others
 * do not. For those compilers that issue warnings, this macro
 * can be used to indicate that the corresponding variable,
 * though and argument to the function, isn't actually used.
 */
#ifndef UNUSEDPARAMETER
#define UNUSEDPARAMETER(x)
#endif

/*=========================================================================
 * VM startup function prototypes
 *=======================================================================*/

int StartJVM(int argc, char* argv[]);
int KVM_Start(int argc, char* argv[]);
void KVM_Cleanup(void);


