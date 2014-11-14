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
 * FILE:      class.h
 * OVERVIEW:  Internal runtime class structures.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998 (added the string pool)
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines the VM-specific internal runtime structures
 * needed for representing classes and their instances in the system.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

extern METHOD RunCustomCodeMethod;
extern INSTANCE OutOfMemoryObject;
extern INSTANCE StackOverflowObject;


/*=========================================================================
 * Class status flags
 *=======================================================================*/

#define CLASS_RAW       0                       /* this value must be 0 */
#define CLASS_LOADING   1
#define CLASS_LOADED    2
#define CLASS_LINKED    3
#define CLASS_VERIFIED  4
#define CLASS_READY     5
#define CLASS_ERROR    -1

/*=========================================================================
 * Global definitions
 *=======================================================================*/

/*  CLASS */
struct classStruct {
    /* Common part from INSTANCE */
    INSTANCE_CLASS ofClass;         /*  'java.lang.Class' (reflection) */
    MONITOR monitor;                /*  The possible monitor */

    /* Note that arrays classes have the same packageName as their base
     * class.  the baseName for arrays is [...[L<className>; or
     * [...[<primitive type>, where [...[ indicates the appropriate
     * number of left brackets for the depth */
    UString packageName;            /* everything before the final '/' */
    UString baseName;		    /* everything after the final '/' */
    CLASS   next;		    /* Next item in this hash table bucket */
    
    unsigned short accessFlags;             /* Access information */
    unsigned short key;                     /* Class key */
} ;

struct instanceClassStruct {
    struct classStruct class;      /* common info */

    /* And specific to instance classes */
    INSTANCE_CLASS superClass;  /* superclass, unless java.lang.Object */
    CONSTANTPOOL constPool;         /*  Pointer to constant pool */
    FIELDTABLE  fieldTable;         /*  Pointer to instance variable table */
    METHODTABLE methodTable;        /*  Pointer to virtual method table */
    unsigned short* ifaceTable; /*  Pointer to interface table */
    POINTERLIST staticFields;       /*  Holds static fields of the class */
    short   instSize;               /*  The size of class instances */
    short status;                   /* Class readiness status */
    THREAD initThread;              /*  Thread performing class initialization */
};

struct arrayClassStruct {
    struct classStruct class;      /* common info */

    /* And stuff specific to an array class */
    union {
	CLASS elemClass;	/*  Element class of object arrays */
	int primType;		/*  Element type for primitive arrays */
    } u;
    int itemSize;		/*  size (bytes) of individual elements */
    				/*  wants to be GCT_ObjectType rather than */
				/*  an int.  But the Palm makes enumerations */
				/*  into "short" */
    int gcType;			/*  GCT_ARRAY or GCT_OBJECTARRAY */
};

#define SIZEOF_INSTANCE_CLASS  StructSizeInCells(instanceClassStruct)
#define SIZEOF_ARRAY_CLASS     StructSizeInCells(arrayClassStruct)

#define className(clazz)  \
        (VERIFY_INSIDE_TEMPORARY_ROOTS, getClassName((CLASS)clazz))

/* A class is considered initialized if it's ready or being initialized
 * by the current thread.
 */
#define CLASS_INITIALIZED(c) \
    ((c)->status == CLASS_READY || (c)->initThread == CurrentThread)

#define IS_ARRAY_CLASS(c) (( ((CLASS)(c))->accessFlags & ACC_ARRAY_CLASS) != 0)


struct objectStruct {
    CLASS   ofClass;     /*  Pointer to the class of the instance */
    MONITOR monitor;     /*  The possible monitor associated with the object */
};


/*  INSTANCE */
struct instanceStruct {
    INSTANCE_CLASS   ofClass;     /*  Pointer to the class of the instance */
    MONITOR monitor;     /*  The possible monitor associated with the object */
    union {
	cell *cellp;
	cell cell;
    } data[1];
};

/* These is never created directly.  It's what a java.lang.String looks like. */
struct stringInstanceStruct {
    INSTANCE_CLASS ofClass;
    MONITOR monitor;
    SHORTARRAY array;
    cell offset;
    cell length;
};

#define SIZEOF_STRING_INSTANCE  SIZEOF_INSTANCE(3)


/* Not yet used. . . */
struct instanceOfClassStruct {
    INSTANCE_CLASS   ofClass;   /* Always java.lang.Class */
    MONITOR          monitor;   /* If necessary */
    CLASS            class;     /* Class of which this is an instance */
};


/*  ARRAY */
struct arrayStruct {
    ARRAY_CLASS ofClass;	/*  Type of the array */
    MONITOR monitor;		/*  The possible monitor */
    cell  length;		/*  Number of elements */
    union {
	cell *cellp;
	cell cell;
    } data[1];
};



struct pointerListStruct {
    long  length;
    union {
	cell **cellpp;		/* For global roots */
	cell *cellp;
	cell cell;
    } data[1];
};


struct byteArrayStruct {
    ARRAY_CLASS ofClass;	/*  Type of the array */
    MONITOR monitor;		/*  The possible monitor */
    cell  length;		/*  The size of the array (slot count) */
    char  bdata[1];		/*  First (zeroeth) data slot of the array */
};

struct shortArrayStruct {
    ARRAY_CLASS ofClass;	/*  Type of the array */
    MONITOR monitor;		/*  The possible monitor */
    cell  length;		/*  The size of the array (slot count) */
    short sdata[1];		/*  First (zeroeth) data slot of the array */
};

/* Abstract types used by the byte code verifier. */
enum {
    ITEM_Bogus,         /* Unused */
    ITEM_Integer,
    ITEM_Float,
    ITEM_Double,
    ITEM_Long,
    ITEM_Null,		/* Result of aconst_null */
    ITEM_InitObject,    /* "this" is in <init> method, before call to super() */

    ITEM_Object,        /* Extra info field gives name. */
    ITEM_NewObject,     /* Like object, but uninitialized. */

    /* The following codes are used by the verifier but don't actually occur in
     * class files.
     */
    ITEM_Long_2,        /* 2nd word of long in register */
    ITEM_Double_2,      /* 2nd word of double in register */

    ITEM_Category1,
    ITEM_Category2,
    ITEM_DoubleWord,
    ITEM_Reference
};

/* Use the 13th bit to indicate whether a class instance has been
 * initialized.
 */
#define ITEM_NewObject_Flag 0x1000
#define ITEM_NewObject_Mask 0x0FFF

#define ENCODE_NEWOBJECT(pc) ((((pc) & 0x7000)<<1) | 0x1000 | ((pc) & 0x0FFF)) 
#define DECODE_NEWOBJECT(no) ((((no) & 0xE000)>>1) | ((no) & 0x0FFF))

/*=========================================================================
 * Sizes for the above structures
 *=======================================================================*/

#define SIZEOF_CLASS            StructSizeInCells(classStruct)
#define SIZEOF_INSTANCE(n)      (StructSizeInCells(instanceStruct) + (n - 1))
#define SIZEOF_ARRAY(n)         (StructSizeInCells(arrayStruct) + (n - 1))
#define SIZEOF_POINTERLIST(n)   (StructSizeInCells(pointerListStruct) + (n - 1))

#define IS_ROM_CLASS(class) (inHeapSpace(class))


/*=========================================================================
 * Global variables
 *=======================================================================*/

/*=========================================================================
 * These global variables hold references to various Java-specific
 * runtime structures needed for executing certain low-level operations.
 *
 * NOTE: To minimize the footprint of the VM, the internal structures these
 *       variables represent are loaded lazily (as opposed to standard JDK
 *       behaviour where these core classes are loaded upon VM
 *       initialization). This means that care must be taken when using
 *       them to ensure that they are not null references.
 *=======================================================================*/


/*  Pointers to the most important Java classes needed by the VM */
extern INSTANCE_CLASS JavaLangObject;    /*  Pointer to class 'java.lang.Object' */
extern INSTANCE_CLASS JavaLangClass;     /*  Pointer to class 'java.lang.Class' */
extern INSTANCE_CLASS JavaLangString;    /*  Pointer to class 'java.lang.String' */
extern INSTANCE_CLASS JavaLangSystem;    /*  Pointer to class 'java.lang.System' */
extern INSTANCE_CLASS JavaLangThread;    /*  Pointer to class 'java.lang.Thread' */
extern INSTANCE_CLASS JavaLangThrowable; /*  Pointer to class 'java.lang.Throwable' */
extern ARRAY_CLASS    JavaLangCharArray; /*  Array of characters */

extern NameTypeKey initNameAndType;
extern NameTypeKey clinitNameAndType;
extern NameTypeKey runNameAndType;


/*=========================================================================
 * Constructors
 *=======================================================================*/

void initializeClass(INSTANCE_CLASS);

/*=========================================================================
 * System-level constructors
 *=======================================================================*/

void  InitializeJavaSystemClasses(void);
void  FinalizeJavaSystemClasses(void);
INSTANCE_CLASS getCurrentClass(void);

/*=========================================================================
 * Operations on classes
 *=======================================================================*/

CLASS getClass(const char *name);
ARRAY_CLASS getArrayClass(int depth, INSTANCE_CLASS baseClass, char signCode);

CLASS getRawClass(const char *, int length);

/* Returns the result.  Note, the return value of the second one is
 * the closing '\0' */
char *getClassName(CLASS clazz);
char *getClassName_inBuffer(CLASS clazz, char *result);

/*=========================================================================
 * Operations on (regular, non-array) instances
 *=======================================================================*/

INSTANCE instantiate(INSTANCE_CLASS);
CLASS    getClassOf(OBJECT);
bool_t   isInstanceOf(OBJECT, CLASS);
bool_t   isAssignableTo(CLASS, CLASS);

/*=========================================================================
 * Operations on array instances
 *=======================================================================*/

ARRAY    instantiateArray(ARRAY_CLASS arrayClass, long count);
ARRAY    instantiateMultiArray(ARRAY_CLASS arrayClass, long dims[], int dimensions);
long     arrayItemSize(int arrayType);

ARRAY_CLASS getPrimitiveArrayClass(int typecode);
ARRAY_CLASS getObjectArrayClass(CLASS elementType);

/*=========================================================================
 * Operations on string instances
 *=======================================================================*/

STRING_INSTANCE instantiateString(const char* string, int length);

char*    getStringContents(STRING_INSTANCE string);
char*    getStringContentsSafely(STRING_INSTANCE string, char *buf, int lth);

char*    bufferToByteArray(const char *buffer, int length);

/*=========================================================================
 * Debugging, monitoring
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void  printClass(CLASS thisClass);
void  printAllClasses(void);
void  printInstance(INSTANCE);
void  printArray(ARRAY);
#else
#  define  printClass(CLASS)
#  define  printAllClasses()
#  define  printInstance(instance)
#  define  printArray(ARRAY)
#endif

#if ENABLEPROFILING
long getClassSize(CLASS thisClass);
int   countClasses(long *memoryUsed);
#else
#  define getClassSize(thisClass) 0
#  define  countClasses(memoryUsed)
#endif

#define FOR_ALL_CLASSES(__clazz__)                                   \
    {                                                                \
     HASHTABLE __table__ = ClassTable;                               \
     int __count__ = __table__->bucketCount;                         \
     while (-- __count__ >= 0) {                                     \
         CLASS __bucket__ = (CLASS)__table__->bucket[__count__];     \
         for ( ; __bucket__ != NULL; __bucket__ = __bucket__->next) { \
              CLASS __clazz__ = __bucket__ ; 

#define END_FOR_ALL_CLASSES   \
                        }     \
                }             \
        }


