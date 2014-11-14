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
 * FILE:      pool.c
 * OVERVIEW:  Constant pool management operations (see pool.h).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Global variables and definitions
 *=======================================================================*/

/*=========================================================================
 * Constructors
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      createConstantPool()
 * TYPE:          constructor
 * OVERVIEW:      Create a new constant pool structure for a class.
 * INTERFACE:
 *   parameters:  number of entries in the constant pool
 *   returns:     pointer to the first constant pool entry
 * NOTES:         Remember that the first entry in the constant pool
 *                (at index [0]) is unused. Its first cell is currently
 *                used for storing the size of the constant pool.
 *=======================================================================*/

CONSTANTPOOL createConstantPool(int numberOfEntries)
{
    int size = numberOfEntries + ((numberOfEntries + (CELL - 1)) >> log2CELL);
    CONSTANTPOOL newPool = (CONSTANTPOOL)callocObject(size, GCT_CONSTANTPOOL);

    CONSTANTPOOL_LENGTH(newPool) = numberOfEntries;
    return newPool;
}

/*=========================================================================
 * Constant pool access methods (low level)
 *=======================================================================*/


/*=========================================================================
 * FUNCTION:      cachePoolEntry()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Given an index to a pool entry, and a resolved value
 *                update that entry to have the given value.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index, resolved value 
 *   returns:     <nothing>
 * NOTE:          factoring out caching of resolved values like this means
 *                PalmOS specific code for writing to database memory is
 *                restricted.
 *=======================================================================*/

static void 
cachePoolEntry(CONSTANTPOOL constantPool, unsigned int cpIndex, cell* value)
{
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];

    if (!USESTATIC || inHeapSpace(constantPool)) { 
        /* For !USESTATIC, the constant pool is always in the heap.
         * For USESTATIC, the constant pool can be in the heap when we're 
         *    setting the initial value of a final static String.
         */
        CONSTANTPOOL_TAG(constantPool, cpIndex) |= CP_CACHEBIT;
        thisEntry->cache = value;
    } else { 
        unsigned char *thisTagAddress = &CONSTANTPOOL_TAG(constantPool, cpIndex);
        int entryOffset = (char *)thisEntry - (char *)constantPool;
        int tagOffset =   (char *)thisTagAddress - (char *)constantPool;
        
        unsigned char                 newTag = *thisTagAddress | CP_CACHEBIT;
        union constantPoolEntryStruct newEntry;
        newEntry.cache = value;

        modifyStaticMemory(constantPool, entryOffset, &newEntry, sizeof(newEntry));
        modifyStaticMemory(constantPool, tagOffset,   &newTag,   sizeof(newTag));
    }
}

/*=========================================================================
 * Constant pool access methods (high level)
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      resolveClassReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Class, get the class
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index , current class
 *   returns:     class pointer
 *=======================================================================*/

CLASS resolveClassReference(CONSTANTPOOL constantPool,
			    unsigned int cpIndex,
			    INSTANCE_CLASS currentClass) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    CLASS thisClass;

    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        return (CLASS)(thisEntry->cache);
    }

    if (VERIFYCONSTANTPOOLINTEGRITY && 
             (thisTag & CP_CACHEMASK) != CONSTANT_Class) { 
        fatalError("Class format error: Illegal CONSTANT_Class reference");
    }
    thisClass = thisEntry->clazz;
    if (!IS_ARRAY_CLASS(thisClass) 
            && (((INSTANCE_CLASS)thisClass)->status == CLASS_RAW)){ 
        loadClassfile((INSTANCE_CLASS)thisClass, TRUE);
    }

    if (IS_ARRAY_CLASS(thisClass) ||
	currentClass == NULL ||
	&(currentClass->class) == thisClass ||
	(thisClass->accessFlags & ACC_PUBLIC) ||
	(thisClass->packageName == currentClass->class.packageName)) {
        /* We need to set the bit, even though we don't change the entry */
        cachePoolEntry(constantPool, cpIndex, (cell*)thisClass);
        return thisClass;
    } else {
	START_TEMPORARY_ROOTS
	sprintf(str_buffer, 
	        "Cannot access class %s from class %s",
		className(thisClass), className(currentClass));
	fatalError(str_buffer);
	END_TEMPORARY_ROOTS
	return NULL;		/* keep compilers happy */
    }
}

/*=========================================================================
 * FUNCTION:      resolveFieldReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Fieldref, get the Field
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index, isStatic
 *   returns:     field pointer
 *=======================================================================*/
/* helper function for access checks. */
#define VERIFY_FIELD_ACCESS 1
#define VERIFY_METHOD_ACCESS 2

static void 
verifyFieldOrMethodAccess(void *fieldOrMethod, int kind)
{
    INSTANCE_CLASS currentClass = getCurrentClass();
    INSTANCE_CLASS fieldClass;
    FIELD thisField = NULL;
    METHOD thisMethod = NULL;
    int access;
    if (kind == VERIFY_FIELD_ACCESS) {
	thisField = (FIELD)fieldOrMethod;
	fieldClass = thisField->ofClass;
	access = thisField->accessFlags;
    } else {
	thisMethod = (METHOD)fieldOrMethod;
	fieldClass = thisMethod->ofClass;
	access = thisMethod->accessFlags;
    }
    if (currentClass == NULL ||
        currentClass == fieldClass ||
	(ACC_PUBLIC & access)) {
	return;
    }
    if (ACC_PROTECTED & access) {
	/* See if currentClass is a subclass of fieldClass */
	INSTANCE_CLASS cb;
	for (cb = currentClass->superClass; cb; cb = cb->superClass) {
	    if (cb == fieldClass) {
		return;
	    }
	}
    }
    if (!(ACC_PRIVATE & access) && 
	currentClass->class.packageName == fieldClass->class.packageName) {
        return;
    }
    {
	START_TEMPORARY_ROOTS
	sprintf(str_buffer, 
	        "Cannot access %s.%s from class %s",
		className(fieldClass),
		thisField ? fieldName(thisField) : methodName(thisMethod),
		className(currentClass));
	fatalError(str_buffer);
	END_TEMPORARY_ROOTS
    }
}

FIELD resolveFieldReference(CONSTANTPOOL constantPool, unsigned int cpIndex, 
                            bool_t isStatic, int opcode) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        return (FIELD)(thisEntry->cache);
    } else { 
        unsigned short classIndex = thisEntry->method.classIndex;
        unsigned short nameTypeIndex = thisEntry->method.nameTypeIndex;
        
        CLASS thisClass = resolveClassReference(constantPool,
						classIndex,
						getCurrentClass());
        NameTypeKey nameTypeKey = 
            constantPool->entries[nameTypeIndex].nameTypeKey;

        FIELD thisField = NULL;
        if (!IS_ARRAY_CLASS(thisClass)) { 
            thisField = lookupField((INSTANCE_CLASS)thisClass, nameTypeKey);
        }
            
        /* Cache the value */
        if (thisField) {
            if (isStatic ? ((thisField->accessFlags & ACC_STATIC) == 0)
                         : ((thisField->accessFlags & ACC_STATIC) != 0)) {
                START_TEMPORARY_ROOTS
                    sprintf(str_buffer, "Incompatible class change: %s.%s", 
                            className(thisField->ofClass), 
                            fieldName(thisField));
                    fatalVMError(str_buffer);
                END_TEMPORARY_ROOTS
            }
	    if ((opcode == PUTSTATIC || opcode == PUTFIELD) &&
		(thisField->accessFlags & ACC_FINAL) &&
		(thisField->ofClass != getCurrentClass())) {
		START_TEMPORARY_ROOTS
	        sprintf(str_buffer, 
			"Cannot modify final field %s.%s from class %s",
			className(thisField->ofClass),
			fieldName(thisField),
			className(getCurrentClass()));
		fatalError(str_buffer);
		END_TEMPORARY_ROOTS
	    }
	    verifyFieldOrMethodAccess(thisField, VERIFY_FIELD_ACCESS);
            cachePoolEntry(constantPool, cpIndex, (cell*)thisField);

        } else { 
            if (INCLUDEDEBUGCODE) { 
                START_TEMPORARY_ROOTS
                    fprintf(stdout, "Failed to find field '%s::%s%s'\n", 
                            className((INSTANCE_CLASS)thisClass), 
                            change_Key_to_Name(nameTypeKey.nt.nameKey, NULL), 
                            change_Key_to_FieldSignature(nameTypeKey.nt.typeKey));
                END_TEMPORARY_ROOTS
            }
        }
        return thisField;
    }
}

/*=========================================================================
 * FUNCTION:      resolveMethodReference()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Given an index to a CONSTANT_Methodref or 
 *                CONSTANT_InterfaceMethodref, get the Method
 *                that the index refers to.
 * INTERFACE:
 *   parameters:  constant pool pointer, constant pool index 
 *   returns:     method pointer
 *=======================================================================*/

METHOD resolveMethodReference(CONSTANTPOOL constantPool, unsigned int cpIndex, 
                              bool_t isStatic) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag = CONSTANTPOOL_TAG(constantPool, cpIndex);
    if (thisTag & CP_CACHEBIT) { 
        /*  Check if this entry has already been resolved (cached) */
        /*  If so, simply return the earlier resolved class */
        return (METHOD)(thisEntry->cache);
    } else { 
        unsigned short classIndex = thisEntry->method.classIndex;
        unsigned short nameTypeIndex = thisEntry->method.nameTypeIndex;
        
        CLASS thisClass = resolveClassReference(constantPool,
						classIndex,
						getCurrentClass());
        NameTypeKey nameTypeKey;
        METHOD thisMethod;

	if (  (((thisTag & CP_CACHEMASK) == CONSTANT_InterfaceMethodref) &&
	        !(thisClass->accessFlags & ACC_INTERFACE))
	    ||
	      (((thisTag & CP_CACHEMASK) == CONSTANT_Methodref) &&
	        (thisClass->accessFlags & ACC_INTERFACE))) {
	    fatalError("bad methodref");
	}

        nameTypeKey = constantPool->entries[nameTypeIndex].nameTypeKey;
	thisMethod = lookupMethod(thisClass, nameTypeKey);
	if (nameTypeKey.nt.nameKey == initNameAndType.nt.nameKey) { 
	    if (thisMethod != NULL 
		&& thisMethod->ofClass != (INSTANCE_CLASS)thisClass) { 
		thisMethod = NULL;
	    }
	}
        if (thisMethod) {
            if (isStatic ? ((thisMethod->accessFlags & ACC_STATIC) == 0)
                         : ((thisMethod->accessFlags & ACC_STATIC) != 0)) {
                START_TEMPORARY_ROOTS
                    sprintf(str_buffer, "Incompatible class change: %s.%s%s", 
                            className(thisMethod->ofClass), 
                            methodName(thisMethod), 
                            methodSignature(thisMethod));
                    fatalVMError(str_buffer);
                END_TEMPORARY_ROOTS
            }
	    verifyFieldOrMethodAccess(thisMethod, VERIFY_METHOD_ACCESS);
            cachePoolEntry(constantPool, cpIndex, (cell*)thisMethod);
        } else { 
            if (INCLUDEDEBUGCODE && thisMethod == NULL) { 
                START_TEMPORARY_ROOTS
                    fprintf(stdout, "Failed to find method '%s::%s%s'\n", 
                            className(thisClass), 
                            change_Key_to_Name(nameTypeKey.nt.nameKey, NULL),
                            change_Key_to_MethodSignature(nameTypeKey.nt.typeKey));
                END_TEMPORARY_ROOTS
            }
        }
        return thisMethod;
    }
}

/*=========================================================================
 * Constant pool printing and debugging
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      getPoolSize()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Returns the amount of memory consumed by the pool
 * INTERFACE:
 *   parameters:  constant pool pointer
 *   returns:     the amount of memory consumed by the pool
 *=======================================================================*/

#if ENABLEPROFILING
int getPoolSize(CONSTANTPOOL constantPool) 
{
    int cpSize = CONSTANTPOOL_LENGTH(constantPool);
    int memSize;

    memSize = cpSize * SIZEOF_CONSTANTPOOL_ENTRY;
    memSize += ByteSizeToCellSize(cpSize);
    return memSize;
}
#endif /*  ENABLEPROFILING */

/*=========================================================================
 * FUNCTION:      printConstantPool()
 * TYPE:          public instance-level operation (mainly for debugging)
 * OVERVIEW:      Print the contents of the given constant pool.
 * INTERFACE:
 *   parameters:  constant pool pointer
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printConstantPool(CONSTANTPOOL constantPool)
{
    unsigned char *tags = CONSTANTPOOL_TAGS(constantPool);
    int cpIndex;
    int cpSize = CONSTANTPOOL_LENGTH(constantPool);

    fprintf(stdout,"Printing constant pool\n");
    fprintf(stdout,"Pool has %d entries\n", cpSize-1);
    
    for (cpIndex = 1; cpIndex < cpSize; cpIndex++) {
        CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
        unsigned char thisTag = tags[cpIndex];

        START_TEMPORARY_ROOTS
            switch (thisTag) {
                
            case CONSTANT_Class | CP_CACHEBIT: 
            case CONSTANT_Class: {
                CLASS clazz = thisEntry->clazz;
                fprintf(stdout,"%d: Class: %s\n", cpIndex, 
                        className((INSTANCE_CLASS)clazz));
                break;
            }

            case CONSTANT_Fieldref: 
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref: {
                char *description = thisTag == CONSTANT_Fieldref ? "Field"
                                  : thisTag == CONSTANT_Methodref ? "Method"
                                  : "InterfaceMethod";
                unsigned short classIndex = thisEntry->method.classIndex;
                unsigned short nameTypeIndex = thisEntry->method.nameTypeIndex;

                CLASS thisClass = resolveClassReference(constantPool,
							classIndex,
							getCurrentClass());
                NameTypeKey nameTypeKey = 
                       constantPool->entries[nameTypeIndex].nameTypeKey;
                char *signature = 
                    thisTag == CONSTANT_Fieldref 
                      ? change_Key_to_FieldSignature(nameTypeKey.nt.typeKey)
                      : change_Key_to_MethodSignature(nameTypeKey.nt.typeKey);
                fprintf(stdout, "%d: %s of Class %s, field %s, type %s\n", 
                        cpIndex, description, 
                        className((INSTANCE_CLASS)thisClass), 
                        change_Key_to_Name(nameTypeKey.nt.nameKey, NULL), 
                        signature);
                break;
            }

            case CONSTANT_String: {
                char *contents = getStringContents(thisEntry->String);
                fprintf(stdout,"%d: String: '%s'\n", cpIndex, contents);
                break;
            }

            case CONSTANT_Integer: {
                int value = thisEntry->integer;
                fprintf(stdout,"%d: Integer: %d\n", cpIndex, value);
                break;
            }

            case CONSTANT_Float: {
                float value = *(float *)&thisEntry->integer;
                fprintf(stdout,"%d: Float: %f\n", cpIndex, value);
                break;
            }

            case CONSTANT_Long: 
            case CONSTANT_Double:
                fprintf(stdout,"%d: %s\n", cpIndex, (thisTag ? "Long" : "Double"));
                cpIndex++;
                break;

            case CONSTANT_NameAndType: { 
                /* Unfortunately, we have no way of knowing if nameTypeKey.nt
                 * is a field signature or a method signature.  It all depends
                 * on who points to us!
                 */
                NameTypeKey nameTypeKey = thisEntry->nameTypeKey;
                fprintf(stdout,"%d: NameAndType: name: %s, type %d\n", 
                        cpIndex,
                        change_Key_to_Name(nameTypeKey.nt.nameKey, NULL), 
                        nameTypeKey.nt.typeKey);
                break;
            }

            case CONSTANT_Utf8: 
                fprintf(stdout,"%d: UTF String\n", cpIndex);
                break;

            case CONSTANT_Fieldref | CP_CACHEBIT: { 
                FIELD field = (FIELD)(thisEntry->cache);
                fprintf(stdout, 
                        "%d: Resolved Field: of Class %s, field %s, type %s\n", 
                        cpIndex, className(field->ofClass), 
                        fieldName(field), fieldSignature(field));
                break;
            }

            case CONSTANT_Methodref  | CP_CACHEBIT: { 
                METHOD method = (METHOD)(thisEntry->cache);
                fprintf(stdout, "%d: Resolved Method: of Class %s, method %s, type %s\n", 
                        cpIndex, className(method->ofClass), 
                        methodName(method), methodSignature(method));
                break;
            }


            default: 
                fatalError("Illegal constant pool tag found.");
                break;
        }
    END_TEMPORARY_ROOTS
    }
    fprintf(stdout,"End of constant pool\n");
}
#endif /*  INCLUDEDEBUGCODE */ 










