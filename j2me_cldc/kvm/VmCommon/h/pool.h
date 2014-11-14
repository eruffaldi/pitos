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
 * SUBSYSTEM: Constant pool
 * FILE:      pool.h
 * OVERVIEW:  Constant pool management definitions.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines a JVM Specification compliant implementation
 * of constant pool structures.  A constant pool is a generic, 
 * relocatable, symbol table -like structure that contains all the
 * symbolic information of a Java class and its references to the 
 * outside world. 
 * 
 * Constant pool is a rather inefficient class representation 
 * format from the implementation point of view. However, since 
 * Java bytecodes access data and methods solely via constant pool
 * indices, a runtime constant pool structure must be maintained 
 * for every class in the system.  In practice, constant pool
 * access is so inefficient that other, additional runtime 
 * structures must be introduced to speed up the system.
 *=======================================================================*/

/*=========================================================================
 * IMPORTANT: 
 * When compiling the system, you must use 4-byte struct alignment 
 * rather than 8-byte alignment, or otherwise long integers 
 * and double floating point values (64 bit, 8 byte numbers)
 * won't work correctly!  Depending on hardware architecture,
 * 1 or 2 byte struct alignment may also work. (2 byte alignment
 * is fine on the PalmPilot).
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Variables
 *=======================================================================*/

/*=========================================================================
 * Field and method access flags (from JVM Specification)
 *=======================================================================*/

#define ACC_PUBLIC       0x0001
#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_FINAL        0x0010
#define ACC_SYNCHRONIZED 0x0020
#define ACC_SUPER        0x0020
#define ACC_VOLATILE     0x0040
#define ACC_TRANSIENT    0x0080
#define ACC_NATIVE       0x0100
#define ACC_INTERFACE    0x0200
#define ACC_ABSTRACT     0x0400

/* The following flags are not part of the JVM specification.  We add then
 * when the structure is created. 
 */

#define ACC_ARRAY_CLASS   0x1000  /* Class is an array class */
#define ACC_ROM_CLASS     0x2000  /* ROM class */
                                  /* A ROM class in which neither it nor any
								   * of its superclasses has a <clinit> */
#define ACC_ROM_NON_INIT_CLASS 0x4000  


/* These are the same values as the JDK.  Makes ROMizer simpler */
#define ACC_DOUBLE        0x4000	 /* Field uses two words*/
#define ACC_POINTER       0x8000  /* Field is a pointer */



/*=========================================================================
 * Array types / type indices from JVM Specification (p. 320)
 *=======================================================================*/

#define T_BOOLEAN   4
#define T_CHAR      5
#define T_FLOAT     6
#define T_DOUBLE    7
#define T_BYTE      8
#define T_SHORT     9
#define T_INT       10
#define T_LONG      11



/*  Our own extensions to array types */
#define T_VOID      0
#define T_REFERENCE 1
#define T_CLASS     1			/* another name */
#define T_LASTPRIMITIVETYPE 11

/*=========================================================================
 * Tag values of constant pool entries from JVM Specification
 *=======================================================================*/

#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref          10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType        12

/*  Cache bit */
#define CP_CACHEBIT  0x80
#define CP_CACHEMASK 0x7F

/*=========================================================================
 * Generic constant pool table entry structure
 *=======================================================================*/

/*=========================================================================
 * COMMENT:
 * Even though the size of constant pool entries depends on the
 * actual type of the entry (types are listed above), in the runtime
 * constant pool of each class all the entries are of the same size.
 *
 * The size of all the entries must be the same, since we want 
 * to be able to address constant pool entries via indexing 
 * rather than more time-consuming lookups.
 *
 * Keep in mind that LONG and DOUBLE constant pool entries consume
 * two entries in the constant pool (see page 98 in JVM Specification).
 *=======================================================================*/

/* CONSTANTPOOLENTRY */

/* Each of these represents one entry in the constant pool */
union constantPoolEntryStruct {
	struct { 
		unsigned short classIndex;
		unsigned short nameTypeIndex;
	}               method;		/* also used by Fields */
	CLASS           clazz;
	STRING_INSTANCE String;
	cell           *cache;		/* either clazz or String */
	cell            integer;
	long            length;
	NameTypeKey     nameTypeKey;
	NameKey         nameKey;
	UString         ustring;
};

/* CONSTANTPOOL */

/* The constant pool structure is slightly complicated.  The first
 * element contains the actual number of entries in the constant pool.
 * 
 * Subsequent entries (1 -- length - 1) follow
 * Immediately after the entry[length - 1] is an array of bytes indicating 
 * The i'th byte is the tag for the i'th entry in the table.
 */


struct constantPoolStruct { 
	union constantPoolEntryStruct entries[1];
};


#define CONSTANTPOOL_LENGTH(cp)      (cp->entries[0].length)
#define CONSTANTPOOL_TAGS(cp)        ((unsigned char *)&(cp)->entries[CONSTANTPOOL_LENGTH(cp)])
#define CONSTANTPOOL_TAG(cp, index)  ((CONSTANTPOOL_TAGS(cp))[index])

#define SIZEOF_CONSTANTPOOL_ENTRY  UnionSizeInCells(constantPoolEntryStruct)




/*=========================================================================
 * Constructors
 *=======================================================================*/

CONSTANTPOOL createConstantPool(int numberOfEntries);

/*=========================================================================
 * Constant pool access methods
 *=======================================================================*/

CLASS    resolveClassReference (CONSTANTPOOL, unsigned int cpIndex, INSTANCE_CLASS fromClass);
FIELD    resolveFieldReference (CONSTANTPOOL, unsigned int cpIndex, bool_t isStatic, int opcode);
METHOD   resolveMethodReference(CONSTANTPOOL, unsigned int cpIndex, bool_t isStatic);

/*=========================================================================
 * Constant pool printing and debugging
 *=======================================================================*/

#if ENABLEPROFILING
int getPoolSize(CONSTANTPOOL);
#else
#define getPoolSize(CONSTANTPOOL);
#endif

#if INCLUDEDEBUGCODE
void printConstantPool(CONSTANTPOOL constantPool);
#else 
#define printConstantPool(CONSTANTPOOL);
#endif
