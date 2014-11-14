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
 * FILE:      fields.c
 * OVERVIEW:  Internal runtime structures for storing different kinds 
 *            of fields (constants, variables, methods, interfaces).
 *            Tables of these fields are created every time a new class
 *            is loaded into the virtual machine.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

static void change_Key_to_MethodSignatureInternal(unsigned char **, char **);
static void change_MethodSignature_to_KeyInternal(char **, unsigned char **);

/*=========================================================================
 * Constructors for field tables
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      createFieldTable()
 * TYPE:          constructor
 * OVERVIEW:      Creates a new table for storing runtime field structures
 *                (instance variable entries, constants, static methods...).
 * INTERFACE:
 *   parameters:  number of field table entries as integer
 *   returns:     pointer to the beginning of the new field table
 *=======================================================================*/

FIELDTABLE createFieldTable(int numberOfEntries)
{
    /*  Align the field table so that accessing static variables is safe */
    /*  regardless of the alignment settings of the compiler */
    int size = SIZEOF_FIELDTABLE(numberOfEntries);
    FIELDTABLE fieldTable = (FIELDTABLE) callocObject(size, GCT_FIELDTABLE);
    /*  Store the size of the table at the beginning of the table */
    fieldTable->length = numberOfEntries;
    return fieldTable;
}

/*=========================================================================
 * FUNCTION:      createMethodTable()
 * TYPE:          constructor
 * OVERVIEW:      Creates a new table for storing runtime method
 *                field structures. A new table is created every time a 
 *                new class is added to the system.
 * INTERFACE:
 *   parameters:  number of methods in the table as integer
 *   returns:     pointer to the beginning of the new method table
 *=======================================================================*/

METHODTABLE createMethodTable(int numberOfEntries)
{
    int size = SIZEOF_METHODTABLE(numberOfEntries);
    METHODTABLE methodTable = (METHODTABLE)callocObject(size, GCT_METHODTABLE);
    methodTable->length = numberOfEntries;
    return methodTable;
}

/*=========================================================================
 * Operations on field and method tables
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      lookupField()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Find a field table entry with the given field name.
 * INTERFACE:
 *   parameters:  class pointer, field name pointer
 *   returns:     pointer to the field or NIL
 *=======================================================================*/

FIELD lookupField(INSTANCE_CLASS thisClass, NameTypeKey key) { 
    do { 
        FIELDTABLE fieldTable = thisClass->fieldTable;
        /*  Get the length of the field table */
        FOR_EACH_FIELD(thisField, fieldTable) 
            if (thisField->nameTypeKey.i == key.i) { 
                return thisField;
            }
        END_FOR_EACH_FIELD
            thisClass = thisClass->superClass;
    } while (thisClass != NULL);
    return NULL;
}


/*=========================================================================
 * FUNCTION:      lookupMethod()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Find a method table entry with the given name and type
 *                using a simple linear lookup strategy.
 * INTERFACE:
 *   parameters:  class pointer, method name and signature pointers
 *   returns:     pointer to the method or NIL
 *
 * NOTES:         This is a very naive implementation with linear search.
 *                In most cases this does not matter, however, since inline
 *                caching allows us to avoid method lookup overhead.
 *=======================================================================*/

METHOD lookupMethod(CLASS thisClass, NameTypeKey key)
{
    INSTANCE_CLASS thisInstanceClass = 
        IS_ARRAY_CLASS(thisClass) ? JavaLangObject : (INSTANCE_CLASS)thisClass;
    do {
        METHODTABLE methodTable = thisInstanceClass->methodTable;
        FOR_EACH_METHOD(thisMethod, methodTable) 
            if (thisMethod->nameTypeKey.i == key.i) { 
                return thisMethod;
            }
        END_FOR_EACH_METHOD
            /*  If the class has a superclass, look its methods as well */
            thisInstanceClass = thisInstanceClass->superClass;
    } while (thisInstanceClass != NULL);
    return NULL;
}

/*=========================================================================
 * FUNCTION:      getSpecialMethod()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Find a specific special method (<clinit>, main)
 *                using a simple linear lookup strategy.
 * INTERFACE:
 *   parameters:  class pointer, method name and signature pointers
 *   returns:     pointer to the method or NIL
 *=======================================================================*/

METHOD getSpecialMethod(INSTANCE_CLASS thisClass, NameTypeKey key)
{
    METHODTABLE methodTable = thisClass->methodTable;
    FOR_EACH_METHOD(thisMethod, methodTable) 
        if (    (thisMethod->accessFlags & ACC_STATIC)
             && (thisMethod->nameTypeKey.i == key.i))
            return thisMethod;
    END_FOR_EACH_METHOD
    return NIL;
}

#if INCLUDEDEBUGCODE

/*=========================================================================
 * FUNCTION:      printField()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given runtime field structure
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to a field object
 *   returns:     <nothing>
 *=======================================================================*/

static void 
printField(FIELD thisField) {
    START_TEMPORARY_ROOTS
       fprintf(stdout,"Field %s%s%s'%s.%s%s'\n", 
               ((thisField->accessFlags & ACC_STATIC) ? "static " : ""),
               ((thisField->accessFlags & ACC_FINAL)  ? "final "  : ""),
               ((thisField->accessFlags & ACC_NATIVE) ? "native " : ""), 
               className(thisField->ofClass), 
               fieldName(thisField), fieldSignature(thisField));
        fprintf(stdout,"Access flags....: %lx\n", thisField->accessFlags);
    END_TEMPORARY_ROOTS
}

/*=========================================================================
 * FUNCTION:      printFieldTable()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given field table
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to a field table
 *   returns:     <nothing>
 *=======================================================================*/

void printFieldTable(FIELDTABLE fieldTable) {
    FOR_EACH_FIELD(thisField, fieldTable) 
        printField(thisField);
    END_FOR_EACH_FIELD;
}
    
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      getMethodTableSize()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Return the size of a method table
 * INTERFACE:
 *   parameters:  pointer to a method table
 *   returns:     the size of the given method table
 *=======================================================================*/

#if ENABLEPROFILING
int getMethodTableSize(METHODTABLE methodTable)
{
#if !USESTATIC
    int size = getObjectSize((cell *)methodTable);
    FOR_EACH_METHOD(entry, methodTable) 
        /*  Add the size of the code segment for a non-native method */
        if ((entry->accessFlags & ACC_NATIVE) == 0) { 
            if (entry->u.java.code != NULL) { 
                size += getObjectSize((cell*)entry->u.java.code);
            }
            /*  Add the size of an exception handler table */
            if (entry->u.java.handlers != NIL) {
                size += SIZEOF_HANDLERTABLE(entry->u.java.handlers->length);
            }
        }
        /*  Add the size of the list of exceptions this method may throw */
        if (entry->mayThrow != NIL)
            size = size + *entry->mayThrow + 1;
            
    END_FOR_EACH_METHOD
    return size;
#else
    /* All of the pieces of code mentioned above are in Static memory, and
     * there's no easy way to get its size.  So we just ignore them.
     */
    UNUSEDPARAMETER(methodTable)
    return 0;
#endif
}
#endif /*  ENABLEPROFILING */

#if INCLUDEDEBUGCODE
/*=========================================================================
 * FUNCTION:      printMethod()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given runtime method structure
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to a method object
 *   returns:     <nothing>
 *=======================================================================*/

static void 
printMethod(METHOD thisMethod) {
   START_TEMPORARY_ROOTS
       fprintf(stdout,"Method %s%s%s'%s.%s%s'\n", 
               ((thisMethod->accessFlags & ACC_STATIC) ? "static " : ""),
               ((thisMethod->accessFlags & ACC_FINAL)  ? "final "  : ""),
               ((thisMethod->accessFlags & ACC_NATIVE) ? "native " : ""), 
               className(thisMethod->ofClass), 
               methodName(thisMethod), methodSignature(thisMethod));
       fprintf(stdout,"Access flags....: %lx\n", (long)thisMethod->accessFlags);
       fprintf(stdout,"Code pointer....: %lx\n", (long)thisMethod->u.java.code);
       fprintf(stdout,"Frame size......: %d\n", thisMethod->frameSize);
       fprintf(stdout,"Argument count..: %d\n", thisMethod->argCount);
       fprintf(stdout,"Maximum stack...: %d\n", thisMethod->maxStack);

       if (thisMethod->u.java.handlers) {
           printExceptionHandlerTable(thisMethod->u.java.handlers);
       }
       if (thisMethod->mayThrow) {
           CONSTANTPOOL cp = thisMethod->ofClass->constPool;
           unsigned int i;

           fprintf(stdout,"The method may throw %ld exceptions:\n", 
                   *thisMethod->mayThrow);
           for (i = 1; i <= *thisMethod->mayThrow; i++) {
               CLASS clazz = cp->entries[thisMethod->mayThrow[i]].clazz;
               fprintf(stdout,"%d: %s\n", i, className(clazz));
           }
       }
       fprintf(stdout,"\n");
   END_TEMPORARY_ROOTS
}

/*=========================================================================
 * FUNCTION:      printMethodTable()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Print the contents of the given method table
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  pointer to a method table
 *   returns:     <nothing>
 *=======================================================================*/

void printMethodTable(METHODTABLE methodTable) {
    FOR_EACH_METHOD(thisMethod, methodTable) 
        printMethod(thisMethod); 
    END_FOR_EACH_METHOD
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Operations on individual fields
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      setField()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Set the contents of a specific field table entry.
 * INTERFACE:
 *   parameters:  pointer to a field table, index, many parameters
 *   returns:     pointer to the changed field
 *=======================================================================*/

void
setField(FIELD thisField, char *name, char *type, 
         INSTANCE_CLASS ofClass, short accessFlags)
{
    thisField->nameTypeKey = getNameAndTypeKey(name, type);
    thisField->ofClass     = ofClass;
    thisField->accessFlags = accessFlags;
}

/*=========================================================================
 * FUNCTION:      setMethod()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Set the contents of a specific method table entry.
 * INTERFACE:
 *   parameters:  pointer to a method table, index, many parameters
 *   returns:     pointer to the changed method
 *=======================================================================*/

void
setMethod(METHOD thisMethod, char *name, char *type, 
          INSTANCE_CLASS ofClass, short accessFlags)
{
    thisMethod->nameTypeKey = getNameAndTypeKey(name, type);
    thisMethod->accessFlags = accessFlags;
    thisMethod->ofClass     = ofClass;
    thisMethod->u.java.handlers = NIL;
    thisMethod->mayThrow    = NIL;

    /*  These values must be initialized later */
    thisMethod->frameSize   = 0;
    thisMethod->maxStack    = 0;
    thisMethod->localCount  = 0;
}

/*=========================================================================
 * FUNCTION:      change_Key_to_FieldSignature()
 *                change_Key_to_FieldSignature_inBuffer()
 *
 * TYPE:          public instance-level operation on runtime classes
 * OVERVIEW:      Converts a FieldType key to its actual signature
 *
 * INTERFACE:
 *   parameters:  key:           An existing FieldType key
 *                resultBuffer:  Where to put the result
 *
 *   returns:     getClassName() returns a pointer to the result
 *                getClassName_inBuffer() returns a pointer to the NULL
 *                     at the end of the result.  The result is in the
 *                     passed buffer.
 * 
 * DANGER:  change_Key_to_FieldSignature_inBuffer() doesn't return what you
 *          expect.
 *
 * DANGER:  change_Key_to_FieldSignature() allocates a byte array, and marks
 *          this array as a temporary root.  This function should only be
 *          called inside START_TEMPORARY_ROOTS ... END_TEMPORARY_ROOTS
 *=======================================================================*/

char *
change_Key_to_FieldSignature(FieldTypeKey key) { 
    char *endBuffer = change_Key_to_FieldSignature_inBuffer(key, str_buffer);
    return bufferToByteArray(str_buffer, endBuffer - str_buffer);
}


char*                   /* returns end of buffer */
change_Key_to_FieldSignature_inBuffer(FieldTypeKey key, char *resultBuffer) { 
    char *p          = resultBuffer;
    int depth        = key >> FIELD_KEY_ARRAY_SHIFT;
    FieldTypeKey baseClassKey = key & 0x1FFF;
    if (depth == MAX_FIELD_KEY_ARRAY_DEPTH) { 
        /* just use whatever the real classname is */
        return getClassName_inBuffer(change_Key_to_CLASS(key), resultBuffer);
    }
    if (depth > 0) { 
        memset(p, '[', depth);
        p += depth;
    }
    if (baseClassKey <= 255) { 
        *p++ = (char)baseClassKey;
    } else { 
        *p++ = 'L';
        p = getClassName_inBuffer(change_Key_to_CLASS(baseClassKey), p);
        *p++ = ';';
    } 
    *p = '\0';
    return p;
}
        
/*=========================================================================
 * FUNCTION:      change_Key_to_MethodSignature()
 *                change_Key_to_MethodSignature_inBuffer()
 *
 * TYPE:          public instance-level operation on runtime classes
 * OVERVIEW:      Converts a MethodType key to its actual signature
 *
 * INTERFACE:
 *   parameters:  key:         An existing MethodType key
 *                resultBuffer:  Where to put the result
 *
 *   returns:     getClassName() returns a pointer to the result
 *                getClassName_inBuffer() returns a pointer to the NULL
 *                     at the end of the result.  The result is in the
 *                     passed buffer.
 * 
 * DANGER:  change_Key_to_MethodSignature_inBuffer() doesn't return what you
 *          expect.
 *
 * DANGER:  change_Key_to_MethodSignature() allocates a byte array, and marks
 *          this array as a temporary root.  This function should only be
 *          called inside START_TEMPORARY_ROOTS ... END_TEMPORARY_ROOTS
 *
 * NOTE: The algorithm for converting a method signature to a key is
 * explained in the documentation below for change_MethodSignature_to_Key()
 *=======================================================================*/

char *
change_Key_to_MethodSignature(MethodTypeKey key) 
{ 
    char *endBuffer = change_Key_to_MethodSignature_inBuffer(key, str_buffer);
    return bufferToByteArray(str_buffer, endBuffer - str_buffer);
}


char*                           /* returns end of string */
change_Key_to_MethodSignature_inBuffer(MethodTypeKey key, char *resultBuffer) {
    int  codedSignatureLength;
    unsigned char *codedSignature = 
        (unsigned char *)change_Key_to_Name(key, &codedSignatureLength);

    unsigned char *from = codedSignature; /* for parsing the codedSignature */
    char *to = resultBuffer;    /* for writing to the result */

    int  argCount = *from++;    /* the first byte is the arg count */

    *to++ = '(';                /* message signatures start with '(' */

    /* Parse the coded signature one argument at a time, and put the 
     * resulting signature into the result.  "from" and "to" are updated. 
     */
    while (--argCount >= 0) { 
        change_Key_to_MethodSignatureInternal(&from, &to);
    }

    /* Now output the closing ')' */
    *to++ = ')';

    /* And the return type */
    change_Key_to_MethodSignatureInternal(&from, &to);

    /* And the closing NULL */
    *to = '\0';

    /* Return the end of the string */
    return to;
}

/* A helper function used above.  
 *
 * It expects the start of the signature of an argument or return type at
 * *fromP.   It encodes that single argument. Both fromP, and toP are updated
 * to point just past the argument just read/written.
 */
static void
change_Key_to_MethodSignatureInternal(unsigned char **fromP, char **toP) { 
    unsigned char *from = *fromP;
    char *to = *toP;
    unsigned char tag = *from++;

    /* normal tags represent themselves */
    if ((tag >= 'A' && tag <= 'Z') && (tag != 'L')) { 
        *to++ = (char)tag;
    } else { 
        FieldTypeKey classKey;
        if (tag == 'L') { 
            tag = *from++;
        }
        classKey = (((int)tag) << 8) + *from++;
        to = change_Key_to_FieldSignature_inBuffer(classKey, to);
    }
    *fromP = from;
    *toP = to;
}


/*=========================================================================
 * FUNCTION:      change_FieldSignature_to_Key()
 *
 * TYPE:          public instance-level operation on runtime classes
 *
 * OVERVIEW:      Converts a field signature into a unique 16-bit value.
 *
 * INTERFACE:
 *   parameters:  type:        A pointer to the field signature
 *                length:      length of signature
 *
 *   returns:     The unique 16-bit key that represents this field type
 * 
 * The encoding is as follows:
 *    0 - 255:    Primitive types.  These use the same encoding as signature.
 *                For example:  int is 'I', boolean is 'Z', etc.
 *
 *  256 - 0x1FFF  Non-array classes.  Every class is assigned a unique 
 *                key value when it is created.  This value is used.  
 *
 * 0x2000-0xDFFF  Arrays of depth 1 - 6.  The high three bits represent the
 *                depth, the low 13-bits (0 - 0x1FFF) represent the type.
 *
 * 0xE000-0xFFFF  Arrays of depth 8 or more.  These are specially encoded
 *                so that the low 13-bits are different that the low 13-bits
 *                of any other class.  The high bits are set so that we'll
 *                recognize it as an array.
 *
 * Note that this scheme lets us encode an array class with out actually
 * creating it.
 * 
 * The algorithm that gives key codes to classes cooperates with this scheme.
 * When an array class is ever created, it is given the appropriate key code.
 *
 * NOTE! [Sheng 1/30/00] The verifier now uses the 13th bit (0x1000) to
 * indicate whether a class instance has been initialized (ITEM_NewObject).
 * As a result, only the low 12 bits (0 - 0xFFF) represent the base class
 * type.
 * 
 *=======================================================================*/

FieldTypeKey
change_FieldSignature_to_Key (char *type, int length) { 
    CLASS clazz;
    char *end = type + length;
    char *p = type;
    
    int depth;

    /* Get the depth, and skip over the initial '[' that indicate the */

    while (*p == '[') { 
        p++;
    }
    depth = p - type;

    /* Arrays of depth greater than 6 are just handled as if they were real
     * classes.  They will automatically be assigned */
    if (depth >= MAX_FIELD_KEY_ARRAY_DEPTH) { 
        clazz = getRawClass(type, length);
        /* I haven't decided yet whether these classes' keys will actually 
         * have their 3 high bits set to 7 or not.  The following will work
         * in either case */
        return (clazz->key) | (short)(MAX_FIELD_KEY_ARRAY_DEPTH << FIELD_KEY_ARRAY_SHIFT);
    } else if (*p != 'L') { 
        /* We have a primitive type, or an array thereof (depth <
           MAX_FIELD_KEY_ARRAY_DEPTH )
        */
        return (*(unsigned char *)p) + (depth << FIELD_KEY_ARRAY_SHIFT);
    } else { 
        /* We have an object, or an array thereof.  With signatures, these
         * always have an 'L' at the beginning, and a ';' at the end.  
         * We're not checking that this is a well-formed signature, but we 
         * could */
        p++;                    /* skip over 'L' for non-arrays */
        end--;                  /* ignore ; at the end */
        clazz = getRawClass(p, end - p);
        return clazz->key + (depth << FIELD_KEY_ARRAY_SHIFT);
    }
} 


/*=========================================================================
 * FUNCTION:      change_MethodSignature_to_Key()
 *
 * TYPE:          public instance-level operation on runtime classes
 *
 * OVERVIEW:      Converts a method signature into a unique 16-bit value.
 *
 * INTERFACE:
 *   parameters:  type:        A pointer to the method signature
 *                length:      length of signature
 *
 *   returns:     The unique 16-bit key that represents this field type
 * 
 * The encoding is as follows:
 * 
 * We first encode the method signature into an "encoded" signature.  This
 * encoded signature is then treated as it were a name, and that name.  We
 * call change_Name_to_Key on it. 
 * 
 * The character array is created as follows
 *   1 byte indicates the number of arguments
 *   1 - 3 bytes indicate the first argument
 *       ..
 *   1 - 3 bytes indicate the nth argument
 *   1 - 3 bytes indicate the return type.
 * 
 *  % If the type is primitive, it is encoded using its signature, like 'I',
 *    'B', etc.  
 *  % Otherwise, we get its 16-bit class key.  
 *     .  If the high byte is in the range 'A'..'Z', we encode it as the 3
 *        bytes,:  'L', hi, lo
 *     .  Otherwise, we encode it as the 2 byte, hi, lo
 *
 * We're saving the whole range 'A'..'Z' since we may want to make 'O' be
 * java.lang.Object, or 'X' be java.lang.String. . . .
 * 
 * Note that the first argument indicates slots, not arguments.  doubles and
 * longs are two slots.
 */

FieldTypeKey 
change_MethodSignature_to_Key(char *type, int length) { 
    /* We're first going to encode the signature, as indicated above. */
    char *buffer = str_buffer;  /* temporary storage */

    char *from = type + 1;      /* pointer into signature */
    char *to = buffer + 1; /* pointer into encoding */
    int argCount = 0;
    int result;

    /* Call a helper function to do the actual work.  This help function
     * converts one field into its encoding.  from and to are updated.  
     * The function returns the number of slots of its argument */
    while (*from != ')') { 
        change_MethodSignature_to_KeyInternal(&from, (unsigned char **)&to);
        argCount++;
    }

    /* Skip over the ')' */
    from++;

    /* And interpret the return type, too */
    change_MethodSignature_to_KeyInternal(&from, (unsigned char **)&to);

    /* Set the slot type. */
    buffer[0] = argCount;
    if (from - type != length) { 
        /* We should have exactly reached the end of the string */
        fatalError("Bad Method signature");
    }

    /* And get the name key for the encoded signature */
    result = change_Name_to_Key(buffer, to - buffer);
    return result;
}

/* Helper function used by the above function.  It parses the single field
 * signature starting at *fromP, and puts the result at *toP.  Both are
 * updated to just past where they were.  The size of the argument is returned.
 */

static void
change_MethodSignature_to_KeyInternal(char **fromP, unsigned char **toP) { 
    char *from = *fromP;
    unsigned char *to = *toP;
    char *endFrom;
    
    switch (*from) { 
        default:
            endFrom = from + 1;
            break;
        case 'L':
            endFrom = strchr(from + 1, ';') + 1;
            break;
        case '[':
            endFrom = from + 1;
            while (*endFrom == '[') endFrom++;
            if (*endFrom == 'L') { 
                endFrom = strchr(endFrom + 1, ';') + 1;             
            } else { 
                endFrom++;
            }
            break;
    }
    if (endFrom == from + 1) { 
        *to++ = (unsigned char)*from;
    } else { 
        FieldTypeKey key = change_FieldSignature_to_Key(from, endFrom - from);
        unsigned char hiChar = ((unsigned)key) >> 8;
        unsigned char loChar = key & 0xFF;
        if (hiChar >= 'A' && hiChar <= 'Z') { 
            *to++ = 'L';
        }
        *to++ = hiChar;
        *to++ = loChar;
    }
    *to = '\0';                 /* always helps debugging */
    *toP = to;
    *fromP = endFrom;
}


/*=========================================================================
 * FUNCTION:      getNameAndTypeKey
 *
 * OVERVIEW:      converts a name and signature (either field or method) to 
 *                a 32bit value. 
 *
 * INTERFACE:
 *   parameters:  name:      Name of method
 *                type:      Field or method signature.
 *
 *   returns:     The unique 32-bit value that represents the name and type.
 * 
 * We use a structure with two 16-bit values.  One 16-bit value encodes the
 * name using standard name encoding.  The other holds the key of the
 * signature.
 * 
 * Note that we cannot do the reverse operation.  Given a type key, we can't
 * tell if its a method key or a field key!
 */



NameTypeKey 
getNameAndTypeKey(char *name, char *type) {
    int typeLength = strlen(type);
    NameTypeKey result;
    result.nt.nameKey = getUString(name)->key;
    result.nt.typeKey = ((type)[0] == '(')
        ?   change_MethodSignature_to_Key(type, typeLength)
          : change_FieldSignature_to_Key(type, typeLength);
    return result;
}


/*=========================================================================
 * Type checking operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getParameterSlotsPointerMask()
 * TYPE:          public operation on method signatures
 * OVERVIEW:      Given a method signature, return an array of booleans
 *                (as bits in an integer) that indicating for slot in the frame
 *                slots corresponding to the method signature if it is a
 *                pointer. We have to do this based on slots as doubles
 *                and longs take 2 slots and so there maybe more slots
 *                for the parameters of a method than there are parameters.
 *                If a method has more than 31 slots, a negative value is
 *                returned. The restriction of 31 slots ensure that a positive
 *                return value means the mask was successfully built.
 * INTERFACE:
 *   parameters:  method
 *   returns:     TRUE if this is a pointer field, FALSE otherwise
 *=======================================================================*/

long getParameterSlotsPointerMask(METHOD method)
{
    MethodTypeKey signatureKey = method->nameTypeKey.nt.typeKey;
    unsigned char *signature = 
        (unsigned char *)change_Key_to_Name(signatureKey, NULL); /* encoded */
    unsigned char *from = signature;
    int argCount = *from++;
    int mask, slot;
    
    for(mask = slot = 0; --argCount >= 0; slot++) { 
        int tag  = *from++;
        if (slot >= 31) { 
            return -1;
        }
        switch (tag) { 
            case 'L':             
                /* These set the mask, and use up two additional bytes */
                mask |= (1 << slot); from += 2; 
                break;
            case 'D': case 'J':
                /* These take up an extra slot */
                slot++; 
                break;
            default:
                if (tag < 'A' || tag > 'Z') { 
                    /* These take up one additional byte, and set the mask */
                    mask |= (1 << slot); from ++; 
                    break; 
                }
        }
    }
    return mask;
}

int getReturnSize(METHOD method)
{
    MethodTypeKey signatureKey = method->nameTypeKey.nt.typeKey;
    unsigned char *signature = 
        (unsigned char *)change_Key_to_Name(signatureKey, NULL); /* encoded */
    unsigned char *from = signature;
    int argCount = *from++;
    
    while (--argCount >= 0) { 
        int tag  = *from++;
        if (tag == 'L') { 
            from += 2;
        } else if (tag < 'A' || tag > 'Z') { 
            from++;
        }
    }
    switch (*from) { 
        case 'V':            return 0;
        default:             return 1;
        case 'D': case 'J':  return 2;
    }
}
