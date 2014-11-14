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
 * SYSTEM:    ROMIZER
 * SUBSYSTEM: Global definitions
 * FILE:      rom.h
 * OVERVIEW:  Macros needed by ROMjava.c
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

/*******
 *
 * NOTE:  The code will compile with gcc even if the __GNUC__ conditionals
 * below are removed.
 * 
 * However the gnu C compiler has special syntax that can be used for 
 * initializing a union.  We use that syntax for added type checking.
 * We avoid having to do lots of casts.
 *
 * Some compilers do not allow you to cast a pointer to a long as part of
 * constant initialization.  Hopefully, these compilers do have some
 * means of initializing union types.   Porters can use the __GNUC__ code
 * as a starting point. 
 *********/

/* A character array of a specific length */
#define CHARARRAY_X(len) \
    struct {                  \
      ARRAY_CLASS ofClass;    \
      MONITOR monitor;        \
      cell  length;           \
      unsigned short sdata[len];       \
    }

#define CHARARRAY_HEADER(len) \
    &manufacturedArrayOfChar_Classblock, NULL, len



/* The layout of an originally created string */

#define KVM_INIT_JAVA_STRING(offset, length) \
    { (INSTANCE_CLASS)&java_lang_String_Classblock, (MONITOR)NULL, \
      (SHORTARRAY)&stringCharArrayInternal, offset, length }

#define STRING_LINK(string, nextLink) { \
        (struct String_Hash_Entry *)nextLink, (STRING_INSTANCE)string }

/* A hashtable of a specific length, and its header */

#define HASHTABLE_X(size) \
struct { \
    int bucketCount; \
    int count;       \
    void *bucket[size]; \
}

#define HASHTABLE_HEADER(size, count) \
    size, count

#define HANDLERTABLE_X(len) \
    struct {         \
        long length; \
        struct exceptionHandlerStruct handlers[len]; \
    }

#define HANDLER_HEADER(len) len

#define HANDLER_ENTRY(start, end, handler, exception) \
{ start, end, handler, exception }

#define DECLARE_USTRING_STRUCT(len)   \
    struct UTF_Hash_Entry_ ## len {  \
        UString next;               \
        unsigned short length;       \
        unsigned short key;          \
        char string[len + 1];            \
}

#define DECLARE_USTRING(name, len) \
    CONST struct UTF_Hash_Entry_ ## len name

#define DEFINE_USTRING(key, next, len, string) \
    CONST struct UTF_Hash_Entry_ ## len UString_Item ## key = { \
        (UString)next, len, 0x ## key, string \
}


#define INSTANCE_INFO(package, base, next, key, access, size, status, \
                       super, methods, fields, constants, intfs) \
{ (INSTANCE_CLASS)&java_lang_Class_Classblock, NULL, \
    (UString)package, (UString)base, (CLASS)next, access, key }, \
    (INSTANCE_CLASS)super, (CONSTANTPOOL)constants, (FIELDTABLE)fields, \
    (METHODTABLE)methods, (unsigned short*)intfs, NULL /* statics */, size, status, NULL

#define RAW_CLASS_INFO(package, base, next, key, access, ignore) \
{ (INSTANCE_CLASS)&java_lang_Class_Classblock, NULL, \
    (UString)package, (UString)base, (CLASS)next, access, key }


#define ARRAY_OF_PRIMITIVE(package, base, next, key, access, typeName) \
{ (INSTANCE_CLASS)&java_lang_Class_Classblock, NULL, \
    (UString)package, (UString)base, (CLASS)next, access, key }, \
   { (CLASS)(T_ ## typeName) }, SIZEOF_T_ ## typeName, GCT_ARRAY


#define ARRAY_OF_OBJECT(package, base, next, key, access, elem) \
{ (INSTANCE_CLASS)&java_lang_Class_Classblock, NULL, \
    (UString)package, (UString)base, (CLASS)next, access, key }, \
    { (CLASS)&elem }, SIZEOF_T_CLASS, GCT_OBJECTARRAY


#ifdef __GNUC__
#   define METHOD_INFO(class, code, handlers, flags, argSize, frameSize, stackSize, codeSize, nameTypeKey) \
	  { nameTypeKey,                                                   \
	    { java:{ code, (HANDLERTABLE)handlers, NULL, codeSize } },     \
	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   frameSize, argSize, frameSize - argSize, stackSize, NULL }

#   define ABSTRACT_METHOD_INFO(class, flags, argSize, nameTypeKey)  \
	  { nameTypeKey,                                                   \
	    { java:{ 0, 0, NULL, 0} },                                     \
	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   0, argSize, 0, 0, NULL }

#   define NATIVE_METHOD_INFO(class, nativeCode, flags, argSize, nameTypeKey) \
	  { nameTypeKey,                                                   \
	    { native:{ nativeCode } },                                     \
 	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   0, argSize, 0, 0, NULL }

#else
#   define METHOD_INFO(class, code, handlers, flags, argSize, frameSize, stackSize, codeSize, nameTypeKey) \
	  { nameTypeKey,                                                   \
	    { {(BYTE*)code, (HANDLERTABLE)handlers, NULL, codeSize } },    \
	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   frameSize, argSize, frameSize - argSize, stackSize, NULL }

#   define ABSTRACT_METHOD_INFO(class, flags, argSize, nameTypeKey)  \
	  { nameTypeKey,                                                   \
	    { { NULL, NULL, NULL, 0} },                                    \
	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   0, argSize, 0, 0, NULL }

#   define NATIVE_METHOD_INFO(class, nativeCode, flags, argSize, nameTypeKey) \
	  { nameTypeKey,                                                   \
	    { { (unsigned char *)nativeCode } },                           \
 	   flags, (INSTANCE_CLASS)&class ## _Classblock,                   \
	   0, argSize, 0, 0, NULL }
#endif


/* Constant Pool */
union ROMconstantPoolEntryStruct {
    long            value;
    const struct classStruct *clazz;
    const struct fieldStruct *field;
    const struct methodStruct *method;
    const struct stringInstanceStruct* String;
}; 

#  define ROM_NTH_METHOD(clazz,index) \
         (&AllMethods.clazz ## _MethodSection.clazz.methods[index])
#  define ROM_NTH_FIELD(clazz,index)  (&AllFields.clazz.fields[index])


#if __GNUC__
#  define ROM_CPOOL_LENGTH(l)            { value: (l)  }
#  define ROM_CPOOL_INT(l)               { value: (l)  } 
#  define ROM_CPOOL_CLASS(c)             { clazz: (&c ## _Classblock.class) }
#  define ROM_CPOOL_STRING(s)            { String: (s) }
#  define ROM_CPOOL_METHOD(clazz,index)  { method: ROM_NTH_METHOD(clazz,index) }
#  define ROM_CPOOL_FIELD(clazz, index)  { field:  ROM_NTH_FIELD(clazz,index) }
#else
#  define ROM_CPOOL_LENGTH(l)            { (long)(l)  }
#  define ROM_CPOOL_INT(l)               { (long)(l)  } 
#  define ROM_CPOOL_CLASS(c)             { (long)(&c ## _Classblock) }
#  define ROM_CPOOL_STRING(s)            { (long)(s) }
#  define ROM_CPOOL_METHOD(clazz, index) { (long)ROM_NTH_METHOD(clazz,index) }
#  define ROM_CPOOL_FIELD(clazz,  index) { (long)ROM_NTH_FIELD(clazz,index)  }
#endif

/* In the KVM, constant pool items are always saved in big-endian order.
 * But this may change in the future.
 */
#define ROM_CPOOL_LONG(a, b)     ROM_CPOOL_INT(a), ROM_CPOOL_INT(b)
#define ROM_CPOOL_DOUBLE(a, b)   ROM_CPOOL_INT(a), ROM_CPOOL_INT(b)


#define ROM_STATIC_INT(a)          a
#define ROM_STATIC_STRING(string) ((INSTANCE)string)

#ifndef ROM_STATIC_LONG
#  if BIG_ENDIAN
#     define ROM_STATIC_LONG(a, b)   a,b
#  elif LITTLE_ENDIAN
#     define ROM_STATIC_LONG(a, b)   b,a
#  else
#     error Must set BIG_ENDIAN or LITTLE_ENDIAN
#  endif
#endif

#ifndef ROM_STATIC_DOUBLE
#  if BIG_ENDIAN
#    define ROM_STATIC_DOUBLE(a, b)   a,b
#  elif LITTLE_ENDIAN
#    define ROM_STATIC_DOUBLE(a, b)   b,a
#  else
#     error Must set BIG_ENDIAN or LITTLE_ENDIAN
#  endif
#endif

#define ROM_CLINIT_METHOD(clazz, index) ROM_NTH_METHOD(clazz, index)

/* frameSize, argCount, localCount, maxStack */ 
  
#define NameAndTypeKey(name, type) { { name, type } }


#if __GNUC__
#   define FIELD_INFO(class, flags, off, nameTypeKey) \
       { nameTypeKey, flags, (INSTANCE_CLASS)&class ## _Classblock, { offset: off} }
#    define STATIC_FIELD_INFO(class, flags, off, nameTypeKey) \
       { nameTypeKey, flags, (INSTANCE_CLASS)&class ## _Classblock, { staticAddress: (KVM_staticData + off) } }
#else
#   define FIELD_INFO(class, flags, offset, nameTypeKey) \
       { nameTypeKey, flags, (INSTANCE_CLASS)&class ## _Classblock, { offset } }
#    define STATIC_FIELD_INFO(class, flags, offset, nameTypeKey) \
       { nameTypeKey, flags, (INSTANCE_CLASS)&class ## _Classblock, { (long)(KVM_staticData + offset) } }
#endif


#define SIZEOF_T_CHAR 2
#define SIZEOF_T_BYTE 1
#define SIZEOF_T_SHORT 2
#define SIZEOF_T_INT 4
#define SIZEOF_T_LONG 8
#define SIZEOF_T_BOOLEAN 1
#define SIZEOF_T_FLOAT 4
#define SIZEOF_T_DOUBLE 8
#define SIZEOF_T_CLASS 4


/*
 * In ROMjava.c we need to make several "forward static" declarations,
 * where the forward declaration has no initializer and the later
 * declaration has an initializer. Some compilers allow you to specify
 * simply "static" in both places. Other compiler treat this as a 
 * redefinition error and require the "forward" declaration to be
 * "external". So, jcc emits the identifier
 * "FORWARD_STATIC_DECLARATION", and each specific compiler will
 * have to define this to be what is appropriate for that compiler.
 *
 * If a compiler has not defined this to be something else, we define
 * it here for the most common case.
 */
#ifndef FORWARD_STATIC_DECLARATION
#define FORWARD_STATIC_DECLARATION static
#endif

#ifndef CONST
#if RELOCATABLE_ROM
#   define CONST
#else
#   define CONST const
#endif
#endif

#if RELOCATABLE_ROM

struct NativeRelocationStruct {
    unsigned short resource;
    unsigned short offset;
    NativeFunctionPtr function;
};

extern struct NativeRelocationStruct NativeRelocations[];



#define NATIVE_RELOCATION_METHOD(clazz, index, nativeCode)           \
   {   clazz ## _MethodSectionNumber,                                \
       offsetof(struct AllMethods_Struct,                            \
		clazz ## _MethodSection.clazz.methods[index]) -      \
	 offsetof(struct AllMethods_Struct, clazz ## _MethodSection), \
       nativeCode                                                    \
   }



/* These are the various resources that are stored as part of the kvm
 * image on the Palm.    Each one is stored as a resource in KVM.prc
 * The resource name is 'jROM'.  The resource ID is 1000 greater than 
 * the resource number indicated in the enumeration below.
 * (.e.g the table of contents is 'jROM' #1000.
 *
 * The StaticDataResource is somewhat special.  Various static fields point
 * into this resource.  However it must be initialized.  It should also be
 * set to 0's.
 */

typedef enum { 
    TOCResource,
    UTFResource,
    StringResource,	
    ClassDefsResource,	
    HandlersResource,	
    FieldsResource,	
    ConstantPoolsResource,
    InterfacesResource,		
    MasterStaticDataResource,	
    StaticDataResource,		
    LastResource = StaticDataResource
    /* Then follow the methodtable resources *
     * ..... 
     * Then follow the code resources
     * .....
     * Then a single unused resource 
     */
} ROMResourceType;

typedef struct ROMTableOfContentsType { 
    int methodTableResourceCount;
    int codeResourceCount;
    HASHTABLE hashtables[3];
    CLASS     classes[7];
    struct { 
	void *startAddress;
	union { 
	    void *endAddress;
	    long size;
	} u;
    } resources[100];
} ROMTableOfContentsType, *ROMTableOfContentsPtr;

#endif
