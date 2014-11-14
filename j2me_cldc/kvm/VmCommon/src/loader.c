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
 * SUBSYSTEM: Class loader
 * FILE:      loader.c
 * OVERVIEW:  Structures and operations needed for loading
 *            Java classfiles (class loader).
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Extensive changes for spec-compliance by Sheng Liang
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Macros and constants
 *=======================================================================*/

#define STRING_POOL(i)   (*((char **)(&StringPool->data[i])))

#define RECOGNIZED_CLASS_FLAGS (ACC_PUBLIC | \
                    ACC_FINAL | \
                    ACC_SUPER | \
                    ACC_INTERFACE | \
                    ACC_ABSTRACT)    
       
#define RECOGNIZED_FIELD_FLAGS (ACC_PUBLIC | \
                    ACC_PRIVATE | \
                    ACC_PROTECTED | \
                    ACC_STATIC | \
                    ACC_FINAL | \
                    ACC_VOLATILE | \
                    ACC_TRANSIENT)

#define RECOGNIZED_METHOD_FLAGS (ACC_PUBLIC | \
                     ACC_PRIVATE | \
                     ACC_PROTECTED | \
                     ACC_STATIC | \
                     ACC_FINAL | \
                     ACC_SYNCHRONIZED | \
                     ACC_NATIVE | \
                     ACC_ABSTRACT )


/*=========================================================================
 * Static functions defined later.
 *=======================================================================*/

#if USESTATIC
static void moveClassFieldsToStatic(INSTANCE_CLASS CurrentClass);
#else
# define moveClassFieldsToStatic(CurrentClass)
#endif

static void  ignoreAttributes(FILEPOINTER ClassFile,
                              INSTANCE_CLASS CurrentClass,
			      POINTERLIST StringPool);

static void NoClassDefFoundError(INSTANCE_CLASS thisClass);


/*=========================================================================
 * Java classfile-specific read operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      skipOverFieldName()
 * TYPE:          private class file load operation
 * OVERVIEW:      skip over a legal field name in a get a given string
 * INTERFACE:
 *   parameters:  string: a string in which the field name is skipped
 *                slash_okay: is '/' OK.
 *                length: length of the string
 *   returns:     what's remaining after skipping the field name
 *=======================================================================*/

static char *
skipOverFieldName(char *string, bool_t slash_okay, unsigned short length)
{
    char *p;
    unsigned short ch;
    unsigned short last_ch = 0;
    /* last_ch == 0 implies we are looking at the first char. */
    for (p = string; p != string + length; last_ch = ch) {
        char *old_p = p;
        ch = *p;
        if (ch < 128) {
            p++;
            /* quick check for ascii */
            if ((ch >= 'a' && ch <= 'z') ||
                (ch >= 'A' && ch <= 'Z') ||
                (last_ch && ch >= '0' && ch <= '9')) {
                continue;
            }
        } else {
            /* KVM simplification: we give up checking for those Unicode
               chars larger than 127. Otherwise we'll have to include a
               couple of large Unicode tables, bloating the footprint by
               8~10K. */
            char *tmp_p = p;
            ch = utf2unicode((const char**)&tmp_p);
            p = tmp_p;
            continue;
        }

        if (slash_okay && ch == '/' && last_ch) {
            if (last_ch == '/') {
                return NULL;    /* Don't permit consecutive slashes */
            }
        } else if (ch == '_' || ch == '$') {
            /* continue */
        } else {
            return last_ch ? old_p : NULL;
        }
    }
    return last_ch ? p : NULL;
}

/*=========================================================================
 * FUNCTION:      skipOverFieldType()
 * TYPE:          private class file load operation
 * OVERVIEW:      skip over a legal field signature in a get a given string
 * INTERFACE:
 *   parameters:  string: a string in which the field signature is skipped
 *                slash_okay: is '/' OK.
 *                length: length of the string
 *   returns:     what's remaining after skipping the field signature
 *=======================================================================*/

static char *
skipOverFieldType(char *string, bool_t void_okay, unsigned short length)
{
    unsigned int depth = 0;
    for (;length > 0;) {
        switch (string[0]) {
        case 'V':
            if (!void_okay) return NULL;
            /* FALL THROUGH */
        case 'Z':
        case 'B':
        case 'C':
        case 'S':
        case 'I':
        case 'F':
        case 'J':
        case 'D':
            return string + 1;

        case 'L': {
            /* Skip over the class name, if one is there. */
            char *p = skipOverFieldName(string + 1, TRUE, length - 1);
            /* The next character better be a semicolon. */
	    if (p != NULL && p < string + length && p[0] == ';') { 
                return p + 1;
	    } 
            return NULL;
        }
        
        case '[':
            /* The rest of what's there better be a legal signature.  */
            string++;
            length--;
	    if (++depth == 256) { 
		return NULL;
	    }
            void_okay = FALSE;
            break;

        default:
            return NULL;
        }
    }
    return NULL;
}

/*=========================================================================
 * FUNCTION:      getUTF8String()
 * TYPE:          private class file load operation
 * OVERVIEW:      get a UTF8 string from string pool, check index validity
 * INTERFACE:
 *   parameters:  String pool, index
 *   returns:     Pointer to UTF8 string
 *=======================================================================*/

static char *
getUTF8String(POINTERLIST StringPool, unsigned short index)
{
    if (index >= StringPool->length ||
        STRING_POOL(index) == NULL) {
        fatalError("bad Utf8 index");
    }
    return STRING_POOL(index);
}

/*=========================================================================
 * FUNCTION:      verifyUTF8String()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate a UTF8 string
 * INTERFACE:
 *   parameters:  pointer to UTF8 string
 *   returns:     nothing
 *=======================================================================*/

static void
verifyUTF8String(char* bytes)
{
    int i;
    unsigned short length = (unsigned short)strlen(bytes);
    for (i=0; i<length; i++) {
        unsigned int c = ((unsigned char *)bytes)[i];
        if (c == 0) /* no embedded zeros */
            goto failed;
        if (c < 128)
            continue;
        switch (c >> 4) {
        default:
            break;

        case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
            goto failed;
            
        case 0xC: case 0xD: 
            /* 110xxxxx  10xxxxxx */
            i++;
            if (i >= length)
                goto failed;
            if ((bytes[i] & 0xC0) == 0x80)
                break;
            goto failed;
            
        case 0xE:
            /* 1110xxxx 10xxxxxx 10xxxxxx */
            i += 2;
            if (i >= length)
                goto failed;
            if (((bytes[i-1] & 0xC0) == 0x80) &&
                ((bytes[i] & 0xC0) == 0x80))
                break;
            goto failed;
        } /* end of switch */
    }
    return;
 failed:
    fatalError("bad UTF8");
}

/*=========================================================================
 * FUNCTION:      verifyName()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate a class, field, or method name
 * INTERFACE:
 *   parameters:  pointer to a name, name type
 *   returns:     nothing
 *=======================================================================*/

bool_t
verifyName(char* name, enum verifyName_type type, bool_t abortOnError)
{
    bool_t result;
    unsigned short length = (unsigned short)strlen(name);

    if (length > 0) {
        if (name[0] == '<') {
            result = (type == LegalMethod) && 
                ((strcmp(name, "<init>") == 0) || 
                 (strcmp(name, "<clinit>") == 0));
        } else {
            char *p;
            if (type == LegalClass && name[0] == '[') {
                p = skipOverFieldType(name, FALSE, length);
            } else {
                p = skipOverFieldName(name, 
                                      type == LegalClass,
                                      length);
            }
            result = (p != NULL) && (p - name == length);
        }
    } else {
        result = FALSE;
    }
    if (!result && abortOnError) { 
        fatalError("bad name");
    } 
    return result;
}

/*=========================================================================
 * FUNCTION:      verifyClassFlags()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate class access flags
 * INTERFACE:
 *   parameters:  class access flags
 *   returns:     nothing
 *=======================================================================*/

static void 
verifyClassFlags(unsigned short flags)
{
    if (flags & ACC_INTERFACE) {
        if ((flags & ACC_ABSTRACT) == 0)
            goto failed;
        if (flags & ACC_FINAL)
            goto failed;
    } else {
        if ((flags & ACC_FINAL) && (flags & ACC_ABSTRACT))
            goto failed;
    }
    return;
    
 failed:
    fatalError("bad class flags");
}

/*=========================================================================
 * FUNCTION:      verifyFieldFlags()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate field access flags
 * INTERFACE:
 *   parameters:  field access flags, isInterface
 *   returns:     nothing
 *=======================================================================*/

static void 
verifyFieldFlags(unsigned short flags, unsigned short classFlags)
{
    if ((classFlags & ACC_INTERFACE) == 0) {
        /* class fields */
        if ((flags & ACC_PUBLIC) && 
            ((flags & ACC_PROTECTED) || (flags & ACC_PRIVATE)))
            goto failed;
        if ((flags & ACC_PROTECTED) && 
            ((flags & ACC_PUBLIC) || (flags & ACC_PRIVATE)))
            goto failed;
        if ((flags & ACC_PRIVATE) && 
            ((flags & ACC_PROTECTED) || (flags & ACC_PUBLIC)))
            goto failed;
        if ((flags & ACC_FINAL) && (flags & ACC_VOLATILE))
            goto failed;
    } else {
        /* interface fields */
        if (flags & ~(ACC_STATIC | ACC_FINAL | ACC_PUBLIC))
            goto failed;
        if (!(flags & ACC_STATIC) ||
            !(flags & ACC_FINAL) ||
            !(flags & ACC_PUBLIC))
            goto failed;
    }
    return;
    
 failed:
    fatalError("bad field flags");
}

/*=========================================================================
 * FUNCTION:      verifyFieldType()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate field signature
 * INTERFACE:
 *   parameters:  field signature
 *   returns:     nothing
 *=======================================================================*/

static void 
verifyFieldType(char* type)
{
    unsigned short length = (unsigned short)strlen(type);
    char *p = skipOverFieldType(type, FALSE, length);

    if (p == NULL || p - type != length) {
        fatalError("bad field signature");
    }
}

/*=========================================================================
 * FUNCTION:      verifyMethodFlags()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate method access flags
 * INTERFACE:
 *   parameters:  method access flags, isInterface, methodName
 *   returns:     nothing
 *=======================================================================*/

static void 
verifyMethodFlags(unsigned short flags, unsigned short classFlags, char* name)
{
    if (strcmp(name, "<init>") == 0) {
        if (flags & ~(ACC_PUBLIC | ACC_PROTECTED | ACC_PRIVATE))
            goto failed;
    } else {
        if ((classFlags & ACC_INTERFACE) == 0) {
            /* class or instance methods */
            if (flags & ACC_ABSTRACT) {
                if ((flags & ACC_FINAL) ||
                    (flags & ACC_NATIVE) || 
                    (flags & ACC_SYNCHRONIZED))
                    goto failed;
            }
            if ((flags & ACC_PRIVATE) && (flags & ACC_ABSTRACT))
                goto failed;
            if ((flags & ACC_STATIC) && (flags & ACC_ABSTRACT))
                goto failed;        
        } else {
            /* interface methods */
            if (!(flags & ACC_ABSTRACT) ||
                !(flags & ACC_PUBLIC))
                goto failed;            
        }
    }
    if ((flags & ACC_PUBLIC) && 
        ((flags & ACC_PROTECTED) || (flags & ACC_PRIVATE)))
        goto failed;
    if ((flags & ACC_PROTECTED) && 
        ((flags & ACC_PUBLIC) || (flags & ACC_PRIVATE)))
        goto failed;
    if ((flags & ACC_PRIVATE) && 
        ((flags & ACC_PROTECTED) || (flags & ACC_PUBLIC)))
        goto failed;
    return;
    
 failed:
    fatalError("bad method flags");
}

/*=========================================================================
 * FUNCTION:      verifyMethodType()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate method signature
 * INTERFACE:
 *   parameters:  method name, signature
 *   returns:     argument size
 *=======================================================================*/
static unsigned short
verifyMethodType(char *name, char* signature)
{
    unsigned short args_size = 0;
    char *p = signature;
    unsigned short length = (unsigned short)strlen(signature);
    char *next_p;

    /* The first character must be a '(' */
    if ((length > 0) && (*p++ == '(')) {
        length--;
        /* Skip over however many legal field signatures there are */
        while ((length > 0) &&
               (next_p = skipOverFieldType(p, FALSE, length))) {
            args_size++;
            if (p[0] == 'J' || p[0] == 'D')
                args_size++;
            length -= (next_p - p);
            p = next_p;
        }
        /* The first non-signature thing better be a ')' */
        if ((length > 0) && (*p++ == ')')) {
            length --;
            if (strlen(name) > 0 && name[0] == '<') {
                /* All internal methods must return void */
                if ((length == 1) && (p[0] == 'V'))
                    return args_size;
            } else {
                /* Now, we better just have a return value. */
                next_p =  skipOverFieldType(p, TRUE, length);
                if (next_p && (length == next_p - p))
                    return args_size;
            }
        }
    }
    fatalError("bad method signature");
    return 0; /* never reached */
}

/*=========================================================================
 * FUNCTION:      verifyConstantPoolEntry()
 * TYPE:          private class file load operation
 * OVERVIEW:      validate constant pool index
 * INTERFACE:
 *   parameters:  constant pool, index, and expected tag
 *   returns:     nothing
 *=======================================================================*/

static void
verifyConstantPoolEntry(CONSTANTPOOL ConstantPool,
                        unsigned short index,
                        unsigned char tag)
{
    unsigned short length = CONSTANTPOOL_LENGTH(ConstantPool);
    unsigned char tag2 = CONSTANTPOOL_TAGS(ConstantPool)[index];
    if (index >= length || tag2 != tag) {
        fatalError("bad constant index");
    }
}

/*=========================================================================
 * FUNCTION:      loadVersionInfo()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the first few bytes of a Java class file,
 *                checking the file type and version information.
 * INTERFACE:
 *   parameters:  classfile pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void 
loadVersionInfo(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass)
{
    UNUSEDPARAMETER(CurrentClass)
    long magic;

    if (TRACECLASSLOADINGVERBOSE || Verbose_On) 
        fprintf(stdout,"Loading version information\n");

    magic = loadCell(ClassFile);
    if (magic != 0xCAFEBABE) {
        /*  This should actually be an exception rather than a fatal error */
        /*  but it is unlikely that we would be able to continue execution */
        fatalError("Class loading failed: Not a Java classfile!");
    }

    skipBytes(ClassFile, 4);    /* ignore minorVersion and majorVersion */
}

/*=========================================================================
 * FUNCTION:      loadConstantPool()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the constant pool part of a Java class file,
 *                building the runtime class constant pool structure
 *                during the loading process.
 * INTERFACE:
 *   parameters:  classfile pointer
 *   returns:     constant pool pointer
 *=======================================================================*/


static POINTERLIST
loadConstantPool(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass)
{
    unsigned short constantCount = loadShort(ClassFile);
    
    CONSTANTPOOL ConstantPool;
    int cpIndex;
    unsigned char* Tags, *CPTags;
    POINTERLIST StringPool;
    CONSTANTPOOL_ENTRY RawPool;
    int lastNonUtfIndex = -1;

    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading the constant pool ====\n");
        fprintf(stdout,"%d constant pool entries\n", (int)(constantCount - 1));
    } else if (Verbose_On) { 
        fprintf(stdout,"Loading constant pool\n");
    }

    START_TEMPORARY_ROOTS
	StringPool = (POINTERLIST)
            callocObject(SIZEOF_POINTERLIST(constantCount), GCT_POINTERLIST);
        StringPool->length = constantCount;
	MAKE_TEMPORARY_ROOT(StringPool);

#define RAW_POOL(i)      (RawPool[i])
#define CP_ENTRY(i)      (ConstantPool->entries[i])
	RawPool = (CONSTANTPOOL_ENTRY)
	    mallocBytes(constantCount * (1 + sizeof(*RawPool)));
	Tags = (unsigned char *)(&RawPool[constantCount]);
	MAKE_TEMPORARY_ROOT(RawPool);

        /*  Read the constant pool entries from the class file */
        for (cpIndex = 1; cpIndex < constantCount; cpIndex++) { 
            unsigned char tag = loadByte(ClassFile);
            Tags[cpIndex] = tag;
            switch (tag) {

            case CONSTANT_String: 
            case CONSTANT_Class: { 
                /* A single 16-bit entry that points to a UTF string */
                unsigned short nameIndex = loadShort(ClassFile);
                RAW_POOL(cpIndex).integer = nameIndex;
                if (TRACECLASSLOADINGVERBOSE) { 
                    char *what = (tag==CONSTANT_Class ? "class" : "String");
                    fprintf(stdout,"%d: %s %d\n", 
                            (int)cpIndex, what, (int)nameIndex);
                }
                break;
            }

            case CONSTANT_Fieldref: 
            case CONSTANT_Methodref: 
            case CONSTANT_InterfaceMethodref: { 
                /* Two 16-bit entries */
                unsigned short classIndex = loadShort(ClassFile);
                unsigned short nameTypeIndex = loadShort(ClassFile);
                RAW_POOL(cpIndex).method.classIndex = classIndex;
                RAW_POOL(cpIndex).method.nameTypeIndex = nameTypeIndex;
                if (TRACECLASSLOADINGVERBOSE) { 
                    char *description = tag == CONSTANT_Fieldref ? "Field"
                                      : tag == CONSTANT_Methodref ? "Method"
                                      : "InterfaceMethod";
                    fprintf(stdout, "%d: %s ofClass:%d nameType:%d\n", 
                            (int)cpIndex, description, 
                            classIndex, nameTypeIndex);
                }
                break;
            }

            case CONSTANT_Integer: 
            case CONSTANT_Float: { 
                /* A signal 32-bit value */
                long value = loadCell(ClassFile);
                RAW_POOL(cpIndex).integer = value;
                if (TRACECLASSLOADINGVERBOSE) { 
                    char *description = (tag == CONSTANT_Integer) ? "Integer" 
                                                                  : "Float";
                    fprintf(stdout,"%d: %s %ld\n", 
                            (int)cpIndex, description, (long)value);
                }
                break;
            }

            case CONSTANT_Long: 
            case CONSTANT_Double:
                /* A 64-bit value */
                RAW_POOL(cpIndex).integer = loadCell(ClassFile);
                cpIndex++;
                if (cpIndex >= constantCount) {
                    fatalError("bad 64-bit constant");
                }
                RAW_POOL(cpIndex).integer = loadCell(ClassFile);
                break;
 
            case CONSTANT_NameAndType: { 
                /* Like Fieldref, etc */
                unsigned short nameIndex = loadShort(ClassFile);
                unsigned short typeIndex = loadShort(ClassFile);
                /* In the second pass, below, these will be replaced with the
                 * actual nameKey and typeKey.  Currently, they are indices
                 * to items in the constant pool that may not yet have been
                 * loaded */
                RAW_POOL(cpIndex).nameTypeKey.nt.nameKey = nameIndex;
                RAW_POOL(cpIndex).nameTypeKey.nt.typeKey = typeIndex;
                if (TRACECLASSLOADINGVERBOSE) { 
                    fprintf(stdout, "%d: NameAndType name:%d desc:%d\n", 
                            (int)cpIndex, nameIndex, typeIndex);
                }
                break;
            }

            case CONSTANT_Utf8: {
                unsigned short length = loadShort(ClassFile);
                char *string = mallocBytes(length + 1);
                STRING_POOL(cpIndex) = string;

                loadBytes(ClassFile, string, length);
                string[length] = '\0';

                verifyUTF8String(string);

                if (TRACECLASSLOADINGVERBOSE) { 
                    fprintf(stdout, "%d: Utf8: '%s'\n", (int)cpIndex, string);
                }
            }
            break;
            
            default: 
                fatalError("Invalid constant pool entry.");
                break;
            }
            
            if (tag != CONSTANT_Utf8) { 
                lastNonUtfIndex = cpIndex;
            }
        }

        ConstantPool = createConstantPool(lastNonUtfIndex + 1);
        CurrentClass->constPool = ConstantPool;
        CPTags = CONSTANTPOOL_TAGS(ConstantPool);
        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout, "Shrinking constant pool from %d to %d\n", 
                    constantCount, lastNonUtfIndex);
        }

	if (USESTATIC) { 
	    makeTransientRoot((cell *)ConstantPool);
	}
	
        /* Now create the constant pool entries */
        for (cpIndex = 1; cpIndex < constantCount; cpIndex++) { 
            unsigned char tag = Tags[cpIndex];
            if (cpIndex <= lastNonUtfIndex) {
                CPTags[cpIndex] = tag;
            }

            switch (tag) {

            case CONSTANT_Class: {
                unsigned short nameIndex = RAW_POOL(cpIndex).integer;
                char *name = getUTF8String(StringPool, nameIndex);
                verifyName(name, LegalClass, TRUE);
                CP_ENTRY(cpIndex).clazz = getRawClass(name, strlen(name));
                break;
            }

            case CONSTANT_String: { 
                unsigned short nameIndex = RAW_POOL(cpIndex).integer;
                char *name = getUTF8String(StringPool, nameIndex);
                STRING_INSTANCE string = internString(name, strlen(name));
                CP_ENTRY(cpIndex).String = string;
                break;
            }
            case CONSTANT_Fieldref: 
            case CONSTANT_Methodref: 
            case CONSTANT_InterfaceMethodref: { 
                /* Two 16-bit entries */
                unsigned short classIndex =
                    RAW_POOL(cpIndex).method.classIndex;
                unsigned short nameTypeIndex =
                    RAW_POOL(cpIndex).method.nameTypeIndex;

                if (classIndex >= constantCount ||
                    Tags[classIndex] != CONSTANT_Class ||
                    nameTypeIndex >= constantCount ||
                    Tags[nameTypeIndex] != CONSTANT_NameAndType) {
                    fatalError("bad field/method ref");
                } else {
                    unsigned short typeIndex = 
                        RAW_POOL(nameTypeIndex).nameTypeKey.nt.typeKey;
                    char* type = getUTF8String(StringPool, typeIndex);
                    if (tag == CONSTANT_Fieldref && type[0] == '(') {
                        fatalError("bad name/type");
                    }
                    CP_ENTRY(cpIndex) = RAW_POOL(cpIndex);
                }
                break;
            }

            case CONSTANT_Long: 
            case CONSTANT_Double:
                CP_ENTRY(cpIndex).integer = RAW_POOL(cpIndex).integer;
                cpIndex++; 
                CPTags[cpIndex] = 0;
                /* fall through */

            case CONSTANT_Integer: 
            case CONSTANT_Float: 
                CP_ENTRY(cpIndex).integer = RAW_POOL(cpIndex).integer;
                break;

            case CONSTANT_NameAndType: { 
                unsigned short nameIndex = 
                    RAW_POOL(cpIndex).nameTypeKey.nt.nameKey;
                unsigned short typeIndex = 
                    RAW_POOL(cpIndex).nameTypeKey.nt.typeKey;
                char* name = getUTF8String(StringPool, nameIndex);
                char* type = getUTF8String(StringPool, typeIndex);
                NameKey nameKey;
                unsigned short typeKey;

                if (type[0] == '(') {
                    verifyName(name, LegalMethod, TRUE);
                    verifyMethodType(name, type);
                    typeKey = change_MethodSignature_to_Key(type,
                                                            strlen(type)); 
                } else {
                    verifyName(name, LegalField, TRUE);
                    verifyFieldType(type);
                    typeKey = change_FieldSignature_to_Key (type,
                                                            strlen(type));
                }
                nameKey = change_Name_to_Key(name, strlen(name));

                CP_ENTRY(cpIndex).nameTypeKey.nt.nameKey = nameKey;
                CP_ENTRY(cpIndex).nameTypeKey.nt.typeKey = typeKey;
                break;
            }

            case CONSTANT_Utf8:
                /* We don't need these after loading time.  So why bother */
                if (cpIndex <= lastNonUtfIndex) {
                    CP_ENTRY(cpIndex).integer = 0;
                    CPTags[cpIndex] = 0;
                }
                break;

            default: 
                fatalVMError("should not happen");
                break;
            }
        }

    END_TEMPORARY_ROOT

    return StringPool;
}
        


/*=========================================================================
 * FUNCTION:      loadClassInfo()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the access flag, thisClass and superClass
 *                parts of a Java class file.
 * INTERFACE:
 *   parameters:  classfile pointer, current constant pool pointer
 *   returns:     Pointer to class runtime structure
 *=======================================================================*/

static void
loadClassInfo(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass)
{
    INSTANCE_CLASS thisClass;
    INSTANCE_CLASS superClass;
    CONSTANTPOOL ConstantPool = CurrentClass->constPool;

    unsigned short accessFlags = loadShort(ClassFile) & RECOGNIZED_CLASS_FLAGS;

    verifyClassFlags(accessFlags);
    
    {
        unsigned short thisClassIndex = loadShort(ClassFile);
        verifyConstantPoolEntry(ConstantPool, thisClassIndex, CONSTANT_Class);
        thisClass = (INSTANCE_CLASS)CP_ENTRY(thisClassIndex).clazz;

        if (CurrentClass != thisClass) {
	    NoClassDefFoundError(CurrentClass);
	}

    }
    {
        unsigned short superClassIndex = loadShort(ClassFile);
        if (superClassIndex == 0) {
            superClass = NULL;
        } else {
            verifyConstantPoolEntry(ConstantPool, superClassIndex, 
                                    CONSTANT_Class);
            superClass = (INSTANCE_CLASS)CP_ENTRY(superClassIndex).clazz;
        }
    }

    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading class information ====\n");
        fprintf(stdout,"Access flags: %d, thisClass: %d, superClass: %d\n", 
                (int)accessFlags, (int)thisClass, (int)superClass);
    } else if (Verbose_On) { 
        fprintf(stdout,"Loading class info\n");
    }

    if (superClass == NULL && (ROMIZING || CurrentClass != JavaLangObject)) {
	fatalError("Bad superclass");
    }
    CurrentClass->superClass = superClass;
    CurrentClass->class.accessFlags = accessFlags;
    if (Verbose_On) 
        fprintf(stdout,"Class info ok\n");
}

/*=========================================================================
 * FUNCTION:      loadInterfaces()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the interface part of a Java class file.
 * INTERFACE:
 *   parameters:  classfile pointer, current class pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void 
loadInterfaces(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass)
{
    /* For compile time debugging.  MAKE_TEMPORARY_ROOT can normally only be
     * used >>lexically<< inside of a START_TEMPORARY_ROOTS.  This lets
     * the code compile by explicitly indicating we're inside that macro
     * dynamically. 
     */
    CONSTANTPOOL ConstantPool = CurrentClass->constPool;
    unsigned short interfaceCount = loadShort(ClassFile);
    int ifIndex;

    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading interface information ====\n");
        fprintf(stdout, "%d interfaces\n", (int)interfaceCount);
    } else if  (Verbose_On) { 
        fprintf(stdout,"Loading interfaces\n");
    }

    if (interfaceCount == 0) { 
        return;
    }

    /*  Build interface table for storing interface indexes */
    CurrentClass->ifaceTable = (unsigned short *)
        mallocBytes((interfaceCount + 1) * sizeof(unsigned short));

    if (USESTATIC) { 
	makeTransientRoot((cell *)CurrentClass->ifaceTable);
    }

    /*  Store length in the first slot */
    CurrentClass->ifaceTable[0] = interfaceCount;

    for (ifIndex = 1; ifIndex <= interfaceCount; ifIndex++) {
        unsigned short cpIndex = loadShort(ClassFile);
        verifyConstantPoolEntry(ConstantPool, cpIndex, CONSTANT_Class);
        CurrentClass->ifaceTable[ifIndex] = cpIndex;
    }

    if (Verbose_On) 
        fprintf(stdout,"Interfaces loaded ok\n");

}

/*=========================================================================
 * FUNCTION:      loadStaticFieldAttributes()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the "ConstantValue" attribute of a static field,
 *                ignoring all the other possible field attributes.
 * INTERFACE:
 *   parameters:  classfile pointer, constant pool pointer, 
 *                field attribute count, pointer to the runtime field struct
 *   returns:     <nothing>
 * 
 * COMMENTS: this function sets the u.offset field of the field to the
 *           constant pool entry that should contain its initial value, or
 *           to 0 if there is no initial value.
 *=======================================================================*/

static void 
loadStaticFieldAttributes(FILEPOINTER ClassFile, CONSTANTPOOL ConstantPool, 
                          FIELD thisField, POINTERLIST StringPool)
{
    int cpIndex = 0;
    unsigned short attrCount = loadShort(ClassFile);
    unsigned short attrIndex;
    
    /*  See if the field has any attributes in the class file */
    for (attrIndex = 0; attrIndex < attrCount; attrIndex++) {
        unsigned short attrNameIndex = loadShort(ClassFile);
        unsigned long  attrLength    = loadCell(ClassFile);
        
        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout, "Attribute index: %d, length: %ld\n", 
                    (int)attrNameIndex, attrLength);
        }
        /*  Check if the attribute represents a constant value index */
        if (!strcmp(getUTF8String(StringPool, attrNameIndex), "ConstantValue")){

	    if (attrLength != 2) {
		fatalError("bad ConstantValue length");
	    }
	    if (cpIndex != 0) { 
		fatalError("Duplicate ConstantValue attribute");
	    }
            /*  Read index to a constant in constant pool */
            cpIndex = loadShort(ClassFile);
	    if (cpIndex == 0) { 
		fatalError("bad constant index");
	    }
        } else {
            /*  Unrecognized attribute; read the bytes to /dev/null */
            skipBytes(ClassFile, attrLength);
        }
    }
    thisField->u.offset = cpIndex;
}


/*=========================================================================
 * FUNCTION:      loadFields()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the fields (static & instance variables) defined 
 *                in a Java class file.
 * INTERFACE:
 *   parameters:  classfile pointer, current class pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void 
loadFields(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass, 
           POINTERLIST StringPool)
{
    CONSTANTPOOL ConstantPool = CurrentClass->constPool;
    unsigned short fieldCount  = loadShort(ClassFile);
    FIELDTABLE fieldTable;

    int staticPtrCount = 0;
    int staticNonPtrCount = 0;
    
    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading field information ====\n");
        fprintf(stdout, "%d fields\n", (int)fieldCount);
    } else if (Verbose_On) { 
        fprintf(stdout,"Loading fields\n");
    } 

    if (fieldCount == 0) { 
        return;
    }


    /*  Create a new field table */
    fieldTable = createFieldTable(fieldCount);

    CurrentClass->fieldTable = fieldTable;

    if (USESTATIC) { 
	makeTransientRoot((cell *)CurrentClass->fieldTable);
    }

    FOR_EACH_FIELD(thisField, fieldTable) 
        unsigned short accessFlags =
            loadShort(ClassFile) & RECOGNIZED_FIELD_FLAGS;
        unsigned short nameIndex   = loadShort(ClassFile);
        unsigned short typeIndex   = loadShort(ClassFile);
        char *fieldName = getUTF8String(StringPool, nameIndex);
        char *signature = getUTF8String(StringPool, typeIndex);
        bool_t isStatic = (accessFlags & ACC_STATIC) != 0;

        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout,
                    "Field %d: nameIndex: %d, typeIndex: %d", 
                    thisField - fieldTable->fields, 
                    (int)nameIndex, (int)typeIndex);
        }

        verifyFieldFlags(accessFlags, CurrentClass->class.accessFlags);
        verifyName(fieldName, LegalField, TRUE);
        verifyFieldType(signature);

        /* Check if the field is double length, or is a pointer type.  If so
         * set the appropriate bit in the word */
        switch (signature[0]) { 
            case 'D': case 'J':   accessFlags |= ACC_DOUBLE;   break;
            case 'L': case '[':   accessFlags |= ACC_POINTER;  break;
        }

        /*  Store the corresponding information in the field structure */
        setField(thisField,
                 fieldName,     /*  Field name */
                 signature,     /*  Signature */
                 CurrentClass,  /*  Class backpointer */
                 accessFlags);  /*  Field access flags */

        if (isStatic) { 
            loadStaticFieldAttributes(ClassFile, ConstantPool, 
                                      thisField, StringPool);
            if (accessFlags & ACC_POINTER) { 
                staticPtrCount++;
            } else {
                staticNonPtrCount += (accessFlags & ACC_DOUBLE) ? 2 : 1;
            } 
        } else { 
            ignoreAttributes(ClassFile, CurrentClass, StringPool);
        }
    END_FOR_EACH_FIELD

    /* We know go back and look at each of the static fields again. */
    if (staticPtrCount > 0 || staticNonPtrCount > 0) { 
        /* We put all the statics into a POINTERLIST.  We specifically make
         * a POINTERLIST in which the real length is longer than the value
         * put into the length field.  The garbage collector only will look
         * at the first "length" fields.
         * So all the pointer statics go before all the non-pointer statics.
         *
         * As a hack, loadStaticFieldAttributes() has put into the offset
         * field the constant pool entry containing the static's initial
         * value.  The static field should be initialized appropriately
         * once space for it has been allocated 
         */
        

        /* Allocate space for all the pointers and non pointers.  
         */
        int staticsSize = SIZEOF_POINTERLIST(staticNonPtrCount+staticPtrCount);
        POINTERLIST statics = 
            (POINTERLIST)callocObject(staticsSize, GCT_POINTERLIST);
        /* All the non-pointers go after all the pointers */
        void **nextPtrField = (void **)statics->data;
        void **nextNonPtrField = nextPtrField + staticPtrCount;

        /* Set the length field so that the GC only looks at the pointers */
        statics->length = staticPtrCount;
        CurrentClass->staticFields = statics;

        FOR_EACH_FIELD(thisField, fieldTable) 
            long accessFlags = thisField->accessFlags;
            unsigned short cpIndex;
            if (!(accessFlags & ACC_STATIC)) { 
                continue;
            }
            cpIndex = (unsigned short)(thisField->u.offset);
            if (thisField->accessFlags & ACC_POINTER) { 
                /* The only possible initialization is for a string */
                thisField->u.staticAddress = nextPtrField;
                if (cpIndex != 0) { 
                    verifyConstantPoolEntry(ConstantPool, cpIndex,
                                            CONSTANT_String);
                    *(STRING_INSTANCE *)nextPtrField = CP_ENTRY(cpIndex).String;
                }
                nextPtrField++;
            } else {
                thisField->u.staticAddress = nextNonPtrField;
                if (cpIndex != 0) { 
                    char* signature = change_Key_to_FieldSignature
                        (thisField->nameTypeKey.nt.typeKey);
                    unsigned char tag;
                    switch (signature[0]) {
                    case 'B':
                    case 'C':
                    case 'Z':
                    case 'S':
                    case 'I':
                        tag = CONSTANT_Integer;
                        break;
                    case 'F':
                        tag = CONSTANT_Float;
                        break;
                    case 'D':
                        tag = CONSTANT_Double;
                        break;
                    case 'J':
                        tag = CONSTANT_Long;
                        break;
                    default:
                        fatalError("bad signature");
                    }
                    verifyConstantPoolEntry(ConstantPool, cpIndex, tag);
                    if (accessFlags & ACC_DOUBLE) {                 
                        /* Initialize a double or long */
                        CONSTANTPOOL_ENTRY thisEntry = &CP_ENTRY(cpIndex);
                        unsigned long hiBytes, loBytes;
                        hiBytes = (unsigned long)(thisEntry[0].integer);
                        loBytes = (unsigned long)(thisEntry[1].integer);
                        SET_LONG_FROM_HALVES(nextNonPtrField, hiBytes, loBytes);
                    } else { 
                        *(cell *)nextNonPtrField = CP_ENTRY(cpIndex).integer;
                    }                       
                }
                nextNonPtrField += (accessFlags & ACC_DOUBLE) ? 2 : 1;
            }
        END_FOR_EACH_FIELD
    }

    if (fieldCount >= 2) { 
	/* Check to see if there are two fields with the same name/type */
	FIELD firstField = &fieldTable->fields[0];
	FIELD lastField = firstField + (fieldCount - 1);
	FIELD outer, inner;
	for (outer = firstField; outer < lastField; outer++) { 
	    for (inner = outer + 1; inner <= lastField; inner++) { 
		if (outer->nameTypeKey.i == inner->nameTypeKey.i) { 
		    fatalError("Duplicate field name/type");
		}
	    }
	}
    }





    if (Verbose_On) 
        fprintf(stdout,"Fields loaded ok\n");
}

/*=========================================================================
 * FUNCTION:      loadExceptionHandlers()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the exception handling information associated
 *                with each method in a class file. 
 * INTERFACE:
 *   parameters:  constant pool, classfile pointer, method pointer
 *   returns:     number of characters read from the class file
 *=======================================================================*/

static int 
loadExceptionHandlers(CONSTANTPOOL ConstantPool,
                      FILEPOINTER ClassFile,
                      METHOD thisMethod)
{
    /*  Load the number of exception handlers */
    unsigned short numberOfHandlers = loadShort(ClassFile);
    if (numberOfHandlers > 0) {
        /*  Create an exception handler table */
        HANDLERTABLE handlerTable = 
               createExceptionHandlerTable(numberOfHandlers);
        /*  Associate this table with the class */
        thisMethod->u.java.handlers = handlerTable;
        
        FOR_EACH_HANDLER(thisHandler, handlerTable) 
            unsigned short startPC   = loadShort(ClassFile);
            unsigned short endPC     = loadShort(ClassFile);
            unsigned short handlerPC = loadShort(ClassFile);
            unsigned short exception = loadShort(ClassFile);

            if (startPC >= thisMethod->u.java.codeLength ||
                endPC >= thisMethod->u.java.codeLength ||
		startPC >= endPC ||
                handlerPC >= thisMethod->u.java.codeLength) {
                fatalError("bad handler");
            }
            
            if (exception) {
                verifyConstantPoolEntry(ConstantPool, exception,
                                        CONSTANT_Class);
            }

            if (TRACECLASSLOADINGVERBOSE) { 
                CONSTANTPOOL cp = thisMethod->ofClass->constPool;
                const char *name;
                START_TEMPORARY_ROOTS
                    if (exception == 0) { 
                        name = "<any>";
                    } else { 
                        CLASS clazz = cp->entries[exception].clazz;
                        name = className(clazz);
                    }
                    fprintf(stdout,
                            "Exception handler ('%s'); PC range: %d-%d; "
                            "handler PC: %d\n",
                            name, (int)startPC, (int)endPC, (int)handlerPC);
                END_TEMPORARY_ROOTS
            }

            /*  Store the handler information in the table */
            setExceptionHandler(thisHandler,
                                startPC, endPC, handlerPC, exception);
        END_FOR_EACH_HANDLER
    } else {
        /*  Method has no associated exception handlers */
        thisMethod->u.java.handlers = NIL;
    }
    return (numberOfHandlers * 8 + 2);
}

/*=========================================================================
 * FUNCTION:      loadStackMaps()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the stack maps associated
 *                with each method in a class file. 
 * INTERFACE:
 *   parameters:  classfile pointer, method pointer, constant pool
 *   returns:     number of characters read from the class file
 *=======================================================================*/

static int 
loadStackMaps(FILEPOINTER ClassFile, METHOD thisMethod, 
              CONSTANTPOOL ConstantPool)
{
    int result = 0;
    unsigned short nStackMaps = loadShort(ClassFile);
    unsigned short stackMapIndex;
    unsigned short* stackMap;

    result += 2;
    thisMethod->u.java.stackMaps = (STACKMAP)
        mallocBytes(sizeof(struct stackMapStruct) + 
        (nStackMaps - 1) * sizeof(struct stackMapEntryStruct));
    thisMethod->u.java.stackMaps->nEntries = nStackMaps;

    stackMap = (unsigned short *)mallocBytes(sizeof(unsigned short) *
                    (thisMethod->maxStack + thisMethod->frameSize + 2));

    START_TEMPORARY_ROOT(stackMap)

    for (stackMapIndex = 0; stackMapIndex < nStackMaps; stackMapIndex++) {
        unsigned short i, index;
        unsigned short offset = loadShort(ClassFile);
        result += 2;

        for (index = 0, i = 0 ; i < 2; i++) {
            unsigned short j;
            unsigned short size = loadShort(ClassFile);
            unsigned short size_delta = 0;
            unsigned short size_index = index++;
            result += 2;
            for (j = 0; j < size; j++) {
                unsigned char stackType = loadByte(ClassFile);
                result += 1;
                if (stackType == ITEM_NewObject) {
                    unsigned short instr = loadShort(ClassFile);
                    if (instr >= thisMethod->u.java.codeLength) {
                        fatalError("bad NewObject");
                    }
                    result += 2;
                    stackMap[index++] = ENCODE_NEWOBJECT(instr);
                } else if (stackType < ITEM_Object) {
                    stackMap[index++] = stackType;
                    if (stackType == ITEM_Long) {
                        stackMap[index++] = ITEM_Long_2;
                        result++;
                        size_delta++;
                    } else if (stackType == ITEM_Double) {
                        stackMap[index++] = ITEM_Double_2;
                        result++;
                        size_delta++;
                    }
                } else if (stackType == ITEM_Object) {
                    unsigned short classIndex = loadShort(ClassFile);
                    CLASS clazz;
                    verifyConstantPoolEntry(ConstantPool, classIndex, 
                                            CONSTANT_Class);
                    clazz = CP_ENTRY(classIndex).clazz;
                    result += 2;
                    stackMap[index++] = clazz->key;
                } else {
                    fatalError("bad stack map");
                }
            }
            stackMap[size_index] = size + size_delta;
        }

        thisMethod->u.java.stackMaps->entries[stackMapIndex].offset = offset;
        thisMethod->u.java.stackMaps->entries[stackMapIndex].stackMapKey =
            change_Name_to_Key((char*)stackMap,
                               index * sizeof(unsigned short));
    }

    END_TEMPORARY_ROOT
            
    return result;
}

/*=========================================================================
 * FUNCTION:      loadMethodAttributes()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the "Code" and "Exceptions" attributes
 *                of a method in a class file, ignoring all the 
 *                other possible method attributes.
 * INTERFACE:
 *   parameters:  classfile pointer, constant pool pointer, 
 *                method attribute count, pointer to the runtime method struct
 *   returns:     <nothing>
 *=======================================================================*/

static void 
loadMethodAttributes(FILEPOINTER ClassFile, CONSTANTPOOL ConstantPool, 
                     METHOD thisMethod, POINTERLIST StringPool)
{
    unsigned short attrCount   = loadShort(ClassFile);
    int attrIndex;
    bool_t needCode = !(thisMethod->accessFlags & (ACC_NATIVE | ACC_ABSTRACT));
    bool_t needExceptionTable = TRUE;  /* always optional */
    
    /*  See if the field has any attributes in the class file */
    for (attrIndex = 0; attrIndex < attrCount; attrIndex++) {
        unsigned short attrNameIndex = loadShort(ClassFile);
        unsigned int   attrLength    = loadCell(ClassFile);
        char*          attrName      = getUTF8String(StringPool, 
                                                     attrNameIndex);

        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout,"Attribute index: %d, length: %d\n", 
                    (int)attrNameIndex, (int)attrLength);
        }

        /*  Check if the attribute contains source code */
        if (!strcmp(attrName, "Code")) { 
            unsigned int codeLength;
            int excLength;
            int nCodeAttrs;
            int codeAttrIndex;

	    if (!needCode) { 
		fatalError("Duplicate code attribute");
	    }
	    needCode = FALSE;

	    /*  Create a code object and store it in the method */
            thisMethod->maxStack    = loadShort(ClassFile); /*  max stack */
            thisMethod->frameSize   = loadShort(ClassFile); /*  frame size */
            codeLength              = loadCell(ClassFile);  /*  Code length */

	    if (codeLength >= 0x7FFF) {
		fatalError("KVM only handles maximum byte code length of 32K");
	    }
            
            /*  Allocate memory for storing the bytecode array */
            thisMethod->u.java.code = (BYTE*)mallocBytes(codeLength);
            thisMethod->u.java.codeLength = codeLength;
            loadBytes(ClassFile, (char *)thisMethod->u.java.code, codeLength);

            /*  Load exception handlers associated with the method */
            excLength = loadExceptionHandlers(ConstantPool, ClassFile, 
                                              thisMethod);

            nCodeAttrs = loadShort(ClassFile);
            for (codeAttrIndex = 0;
                 codeAttrIndex < nCodeAttrs;
                 codeAttrIndex++) {
                unsigned short codeAttrNameIndex = loadShort(ClassFile);
                unsigned int   codeAttrLength    = loadCell(ClassFile);
                char* codeAttrName = getUTF8String(StringPool,
                                                   codeAttrNameIndex);

                if (TRACECLASSLOADINGVERBOSE) { 
                    fprintf(stdout,"Code attribute index: %d, length: %d\n", 
                            (int)codeAttrNameIndex, (int)codeAttrLength);
                }

                /*  Check if the attribute contains stack maps */
                if (!strcmp(codeAttrName, "StackMap")) {
                    loadStackMaps(ClassFile, thisMethod, ConstantPool);
                } else {
                    skipBytes(ClassFile, codeAttrLength);
                }
            }
        } else if (!strcmp(attrName, "Exceptions")) { 
            /*  Check if the attribute contains a list of exceptions */
            /*  that the method may throw */
            /*  Create a list for storing the exceptions */
            unsigned short numberOfExceptions = loadShort(ClassFile);
            unsigned int i;

	    if (!needExceptionTable) { 
		fatalError("Duplicate exception table");
	    } 
	    needExceptionTable = FALSE;

            /*  Create a linear list for storing the exceptions  */
            /*  that the method may throw (represented as constant  */
            /*  pool references. As usual, store length in the  */
            /*  first slot of the list */
            thisMethod->mayThrow = mallocObject(numberOfExceptions + 1, 
                                                GCT_NOPOINTERS);
            *thisMethod->mayThrow = numberOfExceptions;
            for (i = 1; i <= numberOfExceptions; i++) {
                unsigned short cpIndex = loadShort(ClassFile);
                verifyConstantPoolEntry(ConstantPool, cpIndex, CONSTANT_Class);
                thisMethod->mayThrow[i] = cpIndex;
            }
        } else {
            /*  Unrecognized attribute; skip */
            skipBytes(ClassFile, attrLength);
        }
    }
    if (needCode) { 
	fatalError("Missing code attribute");
    }
}

/*=========================================================================
 * FUNCTION:      loadMethods()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the methods defined in a Java class file.
 * INTERFACE:
 *   parameters:  classfile pointer, current class pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void 
loadMethods(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass, 
            POINTERLIST StringPool)
{
    unsigned short methodCount = loadShort(ClassFile);
    CONSTANTPOOL ConstantPool = CurrentClass->constPool;
    METHODTABLE methodTable;

    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading method information ====\n");
        fprintf(stdout,"%d methods\n", (int)methodCount);
    } else if (Verbose_On) { 
        fprintf(stdout,"Loading methods\n");
    }

    if (methodCount == 0) { 
        return;
    }

    /*  Create a new field table */
    methodTable = createMethodTable(methodCount);

    /* Make sure the garbage collector sees the methodtable */
    CurrentClass->methodTable = methodTable;

    if (USESTATIC) { 
	makeTransientRoot((cell *)methodTable);
    }
    
    FOR_EACH_METHOD(thisMethod, methodTable) 
        unsigned short accessFlags =
            loadShort(ClassFile) & RECOGNIZED_METHOD_FLAGS;
        unsigned short nameIndex   = loadShort(ClassFile);
        unsigned short typeIndex   = loadShort(ClassFile);
        char *         methodName  = getUTF8String(StringPool, nameIndex);
        char *         signature   = getUTF8String(StringPool, typeIndex);

        if (strcmp(methodName, "<clinit>") == 0) {
            accessFlags = ACC_STATIC;
        } else {
            verifyMethodFlags(accessFlags, CurrentClass->class.accessFlags, 
                              methodName);
        }
        verifyName(methodName, LegalMethod, TRUE);
        
        thisMethod->argCount = verifyMethodType(methodName, signature);

        /*  If this is a virtual method, increment argument counter by one */
        /*  (the first argument is reserved implicitly for 'this' pointer) */
        if (!(accessFlags & ACC_STATIC)) { 
            thisMethod->argCount++;
        }

        if (thisMethod->argCount > 255) {
            fatalError("too many arguments");
        }

        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout,
                    "Method %d: nameIndex: %d, typeIndex: %d\n", 
                    thisMethod - methodTable->methods,
                    (int)nameIndex, (int)typeIndex);
        } else if (Verbose_On) { 
            fprintf(stdout, "Method '%s'\n", methodName);
        }

        /*  Store the corresponding information in the method structure */
        setMethod(thisMethod, 
                  methodName,   /*  Method name */
                  signature,    /*  Signature */
                  CurrentClass, /*  Class backpointer */
                  accessFlags); /*  Method access information */
        
        /*  Read the "Code" and "Exceptions" attributes from classfile */
	loadMethodAttributes(ClassFile, ConstantPool, thisMethod, StringPool);
	
	if (!(thisMethod->accessFlags & (ACC_NATIVE | ACC_ABSTRACT))) {
	    if ( thisMethod->frameSize < thisMethod->argCount) {
		fatalError("bad frame size");
	    }

	    /*  To speed up stack frame generation later on, calculate also  */
	    /*  the storage space needed for local variables in advance */
	    thisMethod->localCount = 
		thisMethod->frameSize - thisMethod->argCount;
	}

        if (accessFlags & ACC_NATIVE) {
            /*  Store native function pointer in the code field */
            thisMethod->u.native.info = NULL;
            START_TEMPORARY_ROOTS
                thisMethod->u.native.code = 
                    getNativeFunction(className(CurrentClass), methodName);
            END_TEMPORARY_ROOTS
        }
    END_FOR_EACH_METHOD
            
    if (methodCount >= 2) { 
	/* Check to see if there are two methods with the same name/type */
	METHOD firstMethod = &methodTable->methods[0];
	METHOD lastMethod = firstMethod + (methodCount - 1);
	METHOD outer, inner;
	for (outer = firstMethod; outer < lastMethod; outer++) { 
	    for (inner = outer + 1; inner <= lastMethod; inner++) { 
		if (outer->nameTypeKey.i == inner->nameTypeKey.i) { 
		    fatalError("Duplicate method name/type");
		}
	    }
	}
    }

    if (Verbose_On) {
        fprintf(stdout,"Methods loaded ok\n");
    }

}

/*=========================================================================
 * FUNCTION:      ignoreAttributes()
 * TYPE:          private class file load operation
 * OVERVIEW:      Load the possible extra attributes (e.g., the 
 *                "SourceFile" attribute) supplied in a Java class file.
 *                The current implementation ignores all these attributes.
 * INTERFACE:
 *   parameters:  classfile pointer, current class pointer
 *   returns:     <nothing>
 *=======================================================================*/

static void 
ignoreAttributes(FILEPOINTER ClassFile, INSTANCE_CLASS CurrentClass, 
		 POINTERLIST StringPool)
{
    UNUSEDPARAMETER(CurrentClass)
    unsigned short attrCount = loadShort(ClassFile);
    int attrIndex;
   
    if (TRACECLASSLOADINGVERBOSE) { 
        fprintf(stdout,"==== Loading attribute information ====\n");
        fprintf(stdout,"%d extra attributes\n", (int)attrCount);
    } else if (Verbose_On) { 
        fprintf(stdout, "Loading extra attributes\n");
    }


    for (attrIndex = 0; attrIndex < attrCount; attrIndex++) {
        unsigned short attrNameIndex = loadShort(ClassFile);
        unsigned int   attrLength = loadCell(ClassFile);

	/* This verifies that the attribute index is legitimate */
	(void)getUTF8String(StringPool, attrNameIndex);

        if (TRACECLASSLOADINGVERBOSE) { 
            fprintf(stdout,  "Attribute index: %d, length: %d\n", 
                    attrNameIndex, attrLength);
        }
        skipBytes(ClassFile, attrLength);
    }

    if (Verbose_On) 
        fprintf(stdout,"Extra attributes loaded\n");

}

/*=========================================================================
 * Classfile opening operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      loadClassfile()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Load the given Java class file in the system.
 * INTERFACE:
 *   parameters:  class structure.  Only the name field is filled in.  All
 *                other fields must be NULL.
 *   returns:     pointer to runtime class structure or NIL if not found
 *=======================================================================*/

static void
loadClassfileHelper(INSTANCE_CLASS CurrentClass, bool_t fatalErrorIfFail)
{
    FILEPOINTER ClassFile;
    POINTERLIST StringPool;

    if (CurrentClass->status != CLASS_RAW) { 
        return;
    } 

    CurrentClass->status = CLASS_LOADING;
    START_TEMPORARY_ROOTS
	ClassFile = openClassfile(className(CurrentClass)); 
    END_TEMPORARY_ROOTS
    if (ClassFile) {
	START_TEMPORARY_ROOTS
	    int ch;

	    /*  Perform the actual classfile loading */
	    /*  following the classfile structure */
	    /*  (see JVM Specification for further info) */
	    MAKE_TEMPORARY_ROOT(ClassFile);

	    if (TRACECLASSLOADING || TRACECLASSLOADINGVERBOSE) { 
		Log->loadClass(className(CurrentClass));
	    }

	    /*  Load version info and magic value */
	    loadVersionInfo(ClassFile, CurrentClass);

	    /*  Load and create constant pool */
	    StringPool = loadConstantPool(ClassFile, CurrentClass);
	    MAKE_TEMPORARY_ROOT(StringPool);

	    /*  Load class identification information */
	    loadClassInfo(ClassFile, CurrentClass);

	    /*  Load interface pointers */
	    loadInterfaces(ClassFile, CurrentClass);

	    /*  Load field information */
	    loadFields(ClassFile, CurrentClass, StringPool);

	    /*  Load method information */
	    loadMethods(ClassFile, CurrentClass, StringPool);

	    /*  Load the possible extra attributes (e.g., debug info) */
	    ignoreAttributes(ClassFile, CurrentClass, StringPool);

	    ch = loadByteNoEOFCheck(ClassFile);
	    if (ch != EOF) { 
		fatalError("Junk at end of class file");
	    }

	    /*  Ensure that EOF has been reached successfully and close file */
	    closeClassfile(ClassFile);

	    /*  Make the class an instance of class 'java.lang.Class' */
	    CurrentClass->class.ofClass =  JavaLangClass;

	    if (TRACECLASSLOADING) {
		fprintf(stdout, "Creating class: '%s'\n", 
			className(CurrentClass));
	    }

	    /*  Raise the class status to 'LOADED' */
	    CurrentClass->status = CLASS_LOADED;

	    if (Verbose_On) {
		fprintf(stdout,"Class loaded ok\n");
	    }
	END_TEMPORARY_ROOTS
    } else { 
	CurrentClass->status = CLASS_ERROR;
	if (fatalErrorIfFail) { 
	    START_TEMPORARY_ROOTS
		sprintf(str_buffer, 
			"Cannot load class %s", className(CurrentClass));
	        fatalError(str_buffer);
	    END_TEMPORARY_ROOTS
	}
    }
}

static INSTANCE_CLASS
findSuperMostUnlinked(INSTANCE_CLASS CurrentClass)
{
    INSTANCE_CLASS result = NULL;
    while (CurrentClass) {
        if (CurrentClass->status < CLASS_LINKED) {
            result = CurrentClass;
        } else {
            break;
        }
        CurrentClass = CurrentClass->superClass;
    }
    return result;
}

void
loadClassfile(INSTANCE_CLASS CurrentClass, bool_t fatalErrorIfFail)
{
    INSTANCE_CLASS clazz = CurrentClass;

    while (clazz && (clazz->status == CLASS_RAW)) {
	loadClassfileHelper(clazz, fatalErrorIfFail);
	if (clazz->status == CLASS_ERROR) {
	    /* If it's the current class that fails, then the loader
	     * handles it.  But if a superclass fails, then this
	     * is serious 
	     */
	    if (clazz != CurrentClass) { 
		NoClassDefFoundError(clazz);
	    }
	    return;
	}
	clazz = clazz->superClass;
    }

    for (clazz = CurrentClass->superClass; clazz != NULL; 
	   clazz = clazz->superClass) { 
	if (clazz == CurrentClass) { 
	    fatalError("Class circularity");
	    /* If we get rid of the fatal error, we have to mark
	     * everything in the loop as status  CLASS_ERROR */
	}
    }

    while ((clazz = findSuperMostUnlinked(CurrentClass)) != NULL) {
	/* Compute instance size and instance field offset */

	if (clazz->superClass) {
	    /* Make the instance size of the new class "inherit" */
	    /* the instance size of the superclass */
	    clazz->instSize = clazz->superClass->instSize;
	} else {
	    /*  Treat 'java.lang.Object' specially (it has no superclass) */
	    clazz->instSize = 0;
	}

	FOR_EACH_FIELD(thisField, clazz->fieldTable) 
	    unsigned short accessFlags = thisField->accessFlags;

	    if ((accessFlags & ACC_STATIC) == 0) {
		thisField->u.offset = clazz->instSize;
		clazz->instSize += (accessFlags & ACC_DOUBLE) ? 2 : 1;
	    }
	END_FOR_EACH_FIELD

	/* Move parts of the class to static memory */

	/* **DANGER** 
	 * While moveClassFieldsToStatic is running, things are in a
	 * very strange state as far as GC is concerned.  It is important
	 * that this function do no allocation, and that we get rid
	 * of the temporary roots registered above before any other
	 * allocation is done.
	 */
	moveClassFieldsToStatic(clazz);
	clazz->status = CLASS_LINKED;
    }   
}

#if USESTATIC

static void
appendToStatic (char *staticMemory, void* valuePtr, int *offset);


static void
moveClassFieldsToStatic(INSTANCE_CLASS CurrentClass)
{
    /* We are going to have to walk the method table, so get info */
    METHODTABLE methodTable   = CurrentClass->methodTable;
    FIELDTABLE  fieldTable    = CurrentClass->fieldTable;
    CONSTANTPOOL constantPool = CurrentClass->constPool;
    unsigned short* ifaceTable          = CurrentClass->ifaceTable;

    char* staticBytes;
    int offset;
    
    int totalSize = getObjectSize((cell *)constantPool) /* never NULL */
             + ((fieldTable == NULL) ? 0 : getObjectSize((cell *)fieldTable))
             + ((ifaceTable == NULL) ? 0 : getObjectSize((cell *)ifaceTable))
             + ((methodTable == NULL) ? 0 : getObjectSize((cell *)methodTable));

    FOR_EACH_METHOD(thisMethod, methodTable)
        if ((thisMethod->accessFlags & ACC_NATIVE) == 0) { 
            if (!ENABLEFASTBYTECODES && thisMethod->u.java.code != NULL) {
                totalSize += getObjectSize((cell *)thisMethod->u.java.code);
            } 
            if (thisMethod->u.java.stackMaps != NULL) { 
                totalSize += getObjectSize((cell *)thisMethod->u.java.stackMaps);
            }
            if (thisMethod->u.java.handlers != NULL) { 
                totalSize += getObjectSize((cell *)thisMethod->u.java.handlers);
            }
        }
        if (thisMethod->mayThrow != NULL) { 
            totalSize += getObjectSize((cell *)thisMethod->mayThrow);
        }
    END_FOR_EACH_METHOD

    staticBytes = mallocStaticBytes(totalSize << log2CELL);
    
    removeTransientRootByValue((cell*)CurrentClass->constPool);
    removeTransientRootByValue((cell*)CurrentClass->fieldTable);
    removeTransientRootByValue((cell*)CurrentClass->ifaceTable);
    removeTransientRootByValue((cell*)CurrentClass->methodTable);

    /* Copy the constant pool */
    offset = 0;

    appendToStatic(staticBytes, &CurrentClass->constPool,  &offset);
    appendToStatic(staticBytes, &CurrentClass->fieldTable, &offset);
    appendToStatic(staticBytes, &CurrentClass->ifaceTable, &offset);
    
    FOR_EACH_METHOD(thisMethod, methodTable)
        if ((thisMethod->accessFlags & ACC_NATIVE) == 0) { 
            if (!ENABLEFASTBYTECODES) { 
                appendToStatic(staticBytes, &thisMethod->u.java.code, &offset);
            }
            appendToStatic(staticBytes, &thisMethod->u.java.stackMaps, &offset);
            appendToStatic(staticBytes, &thisMethod->u.java.handlers, &offset);
        }
        appendToStatic(staticBytes, &thisMethod->mayThrow, &offset);
    END_FOR_EACH_METHOD

    appendToStatic(staticBytes, &CurrentClass->methodTable, &offset);

    if (offset != totalSize << log2CELL) { 
        fatalError("Problem in moving to static");
    }
}



/* This is a helper function for moveClassFieldsToStatic() above, and is
 * only ever called from it. 
 * It appends the object *valuePtr to the allocated memory starting at
 * offset *offsetP.  
 * It then updates *offsetP to include the length of this object, and
 * valuePtr to point to the copy of the object */

static void
appendToStatic (char *staticBytes, void *valuePtr, int *offsetP)
{ 
    cell *value = *(cell **)valuePtr;
    if (value != NULL) { 
        int size = getObjectSize(value) << log2CELL;
        int offset = *offsetP;
        modifyStaticMemory(staticBytes, offset, value, size);
        *offsetP = offset + size;
        *(cell **)valuePtr = (cell *)&staticBytes[offset]; 
    }
}

#endif


/*=========================================================================
 * FUNCTION:      replaceLetters()
 * TYPE:          auxiliary private function
 * OVERVIEW:      Convert all characters c1 in the given string to 
 *                character c2.
 * INTERFACE:
 *   parameters:  string, two characters
 *   returns:     modified string
 *=======================================================================*/

char* replaceLetters(char* string, char c1, char c2)
{
    int length = strlen(string);
    int i;
    char* c = string;

    for (i = 0; i < length; i++, c++) {
        if (*c == c1) *c = c2;
    }

    return string;
}


/*=========================================================================
 * FUNCTION:      NoClassDefFoundError
 * TYPE:          auxiliary private function
 * OVERVIEW:      Fatal error indicating class not found
 * INTERFACE:
 *   parameters:  class whose definition cannot be found
 *   returns:     doesn't.
 *=======================================================================*/


static void 
NoClassDefFoundError(INSTANCE_CLASS thisClass) { 
    START_TEMPORARY_ROOTS
	sprintf(str_buffer, "NoClassDefFoundError: %s", className(thisClass));
        fatalError(str_buffer);
    END_TEMPORARY_ROOTS
}

