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
 * FILE:      fields.h
 * OVERVIEW:  Internal runtime structures for storing different kinds 
 *            of fields (constants, variables, methods, interfaces).
 *            Tables of these fields are created every time a new class
 *            is loaded into the virtual machine.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines the VM-specific internal runtime structures
 * needed for representing fields, methods and interfaces. For 
 * simplicity, all these structures are allocated in static (C/C++) 
 * heap rather than dynamic garbage collected heap.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * FIELD is the basic structure that is used for representing
 * information about instance variables, static variables, constants
 * and static methods. It is used as a component in constant pool
 * and instance variable tables stored in CLASS structures.
 *
 * Method tables use a slightly larger structure (METHOD)
 * which is a subtype of FIELD. 
 *
 * Note: to speed up stack frame generation at runtime, we calculate
 * the number of parameters (argCount) and local variables (localCount)
 * for each method in advance upon class loading.
 *=======================================================================*/


/*  FIELD */
struct fieldStruct {
    NameTypeKey nameTypeKey;
    long        accessFlags; /*  Access control indicators (e.g., public/private) */
    INSTANCE_CLASS ofClass;      /*  Backpointer to the class owning the field */
    union { 
        long  offset;           /* for dynamic objects */
        void *staticAddress;    /* pointer to static.  */
    } u;
};

struct fieldTableStruct { 
    long length;
    struct fieldStruct fields[1];
};


/*  METHOD */
struct methodStruct {
    NameTypeKey   nameTypeKey;
    union { 
        struct {
            BYTE* code;
            HANDLERTABLE handlers;
            STACKMAP stackMaps;        /* stack maps */
            unsigned short codeLength;
            unsigned short unused;
        } java;
        struct { 
            void (*code)(void);
            void *info;
        } native;
    } u;
    long  accessFlags;  /*  Access control indicators (e.g., public/private) */
    INSTANCE_CLASS ofClass;     /*  Backpointer to the class owning the field */
    unsigned short frameSize;   /*  Method frame size (arguments + local variables) */
    unsigned short argCount;    /*  Method argument (parameter) count */
    unsigned short localCount;  /*  Local variable count */
    unsigned short maxStack;    /*  Maximum number of words on operand stack */
    cell* mayThrow;     /*  List of exceptions that the method may throw */
};

struct methodTableStruct { 
    long length;
    struct methodStruct methods[1];
};

struct stackMapStruct {
    unsigned short nEntries;
    struct stackMapEntryStruct {
        unsigned short offset;
        unsigned short stackMapKey;
    } entries[1];
};

/*=========================================================================
 * Sizes of the above structures
 *=======================================================================*/

#define SIZEOF_FIELD             StructSizeInCells(fieldStruct)
#define SIZEOF_METHOD            StructSizeInCells(methodStruct)


#define SIZEOF_METHODTABLE(n)  \
       (StructSizeInCells(methodTableStruct) + (n - 1) * SIZEOF_METHOD)

#define SIZEOF_FIELDTABLE(n)   \
       (StructSizeInCells(fieldTableStruct) + (n - 1) * SIZEOF_FIELD)


/*=========================================================================
 * Constructors for field tables
 *=======================================================================*/

FIELDTABLE createFieldTable(int numberOfEntries);
METHODTABLE createMethodTable(int numberOfEntries);

/*=========================================================================
 * Operations on field and method tables
 *=======================================================================*/

METHOD lookupMethod(CLASS thisClass, NameTypeKey key);
METHOD getSpecialMethod(INSTANCE_CLASS thisClass, NameTypeKey key);
FIELD  lookupField(INSTANCE_CLASS thisClass, NameTypeKey key);

#if ENABLEPROFILING
int    getMethodTableSize(METHODTABLE methodTable);
#else 
# define getMethodTableSize(table) 0
#endif

#if INCLUDEDEBUGCODE
void   printFieldTable(FIELDTABLE fieldTable);
void   printMethodTable(METHODTABLE methodTable);
#else 
# define printFieldTable()
# define printMethodTable()
#endif

/*=========================================================================
 * Operations on individual fields and methods
 *=======================================================================*/

#define FOR_EACH_METHOD(__var__, methodTable) {                            \
    if (methodTable != NULL) {                                             \
         METHOD __first_method__ = methodTable->methods;                   \
         METHOD __end_method__ = __first_method__ + methodTable->length;   \
         METHOD __var__;                                                   \
         for (__var__ = __first_method__; __var__ < __end_method__; __var__++) {

#define END_FOR_EACH_METHOD } } }

#define FOR_EACH_FIELD(__var__, fieldTable) {                            \
     if (fieldTable != NULL) {                                           \
         FIELD __first_field__ = fieldTable->fields;                     \
         FIELD __end_field__ = __first_field__ + fieldTable->length;     \
         FIELD __var__;                                                  \
         for (__var__ = __first_field__; __var__ < __end_field__; __var__++) { 

#define END_FOR_EACH_FIELD } } }


void setField(FIELD field, char* fieldName, char* signature,
	      INSTANCE_CLASS ofClass, short accessFlags);

void setMethod(METHOD method, char* methodName, char* signature,
               INSTANCE_CLASS ofClass, short accessFlags);

/*=========================================================================
 * Type checking operations
 *=======================================================================*/

int countMethodParameters(METHOD method);
long getParameterSlotsPointerMask(METHOD method);
int getReturnSize(METHOD method);


/* To be defined */
#define methodName(method)      \
     (change_Key_to_Name((method)->nameTypeKey.nt.nameKey, NULL))

#define methodSignature(method) \
     (VERIFY_INSIDE_TEMPORARY_ROOTS, \
      change_Key_to_MethodSignature((method)->nameTypeKey.nt.typeKey))

#define fieldName(field) \
     (change_Key_to_Name((field)->nameTypeKey.nt.nameKey, NULL))

#define fieldSignature(field) \
    (VERIFY_INSIDE_TEMPORARY_ROOTS, \
     change_Key_to_FieldSignature((field)->nameTypeKey.nt.typeKey))

NameTypeKey getNameAndTypeKey(char* name, char* type);

char *change_Key_to_MethodSignature(MethodTypeKey);
char *change_Key_to_MethodSignature_inBuffer(MethodTypeKey, char *result);

char *change_Key_to_FieldSignature(FieldTypeKey);
char *change_Key_to_FieldSignature_inBuffer(FieldTypeKey, char *result);


FieldTypeKey change_FieldSignature_to_Key(char *signature, int length);
MethodTypeKey change_MethodSignature_to_Key(char *signature , int length);

#define FIELD_KEY_ARRAY_SHIFT 13
#define MAX_FIELD_KEY_ARRAY_DEPTH 7
