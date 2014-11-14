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
 * SUBSYSTEM: Hashtables
 * FILE:      hashtable.c
 * OVERVIEW:  Maintains all the hashtables in the system:
 *               ClassTable:        Mapping from char* to name
 *               InternStringTable: Mapping from char* to String
 *               UTFStringTable:    Mapping from char* to unique char*
 *            
 * AUTHOR:    Frank Yellin
 *=======================================================================*/


#include <global.h>

/*=========================================================================
 * Variables:
 *=======================================================================*/

#if ROMIZING
extern
#endif
HASHTABLE InternStringTable;    /* char* to String */

#if ROMIZING
extern
#endif
HASHTABLE UTFStringTable;       /* char* to unique instance */

#if ROMIZING
extern
#endif
HASHTABLE ClassTable;           /* package/base to CLASS */

/*=========================================================================
 * FUNCTION:      stringHash
 * OVERVIEW:      Returns a hash value for a C string
 * INTERFACE:
 *   parameters:  s:    pointer to a string
 *                len:  length of string, in bytes
 *   returns:     a hash value
 *=======================================================================*/

static unsigned int
stringHash(const char *s, int length)
{
    unsigned raw_hash = 0;
    while (--length >= 0) { 
    int c = *s++;
        raw_hash = raw_hash * 37 + c;
    }
    return raw_hash;
}

    
/*=========================================================================
 * FUNCTION:      createHashTable
 * OVERVIEW:      Creates a hashtable, assigns it to a location, and 
 *                registers that location as a root for GC.
 * INTERFACE:
 *   parameters:  tablePtr:  Address of variable holding location
 *                bucketCount: Size of hash able
 *
 *   returns:     no value, but as a side effect, puts the newly created
 *                hashtable into *tablePtr
 *=======================================================================*/

void
createHashTable(HASHTABLE *tablePtr, int bucketCount) { 
    int objectSize = SIZEOF_HASHTABLE(bucketCount);
    HASHTABLE table = (HASHTABLE)callocObject(objectSize, GCT_HASHTABLE);
    table->bucketCount = bucketCount;
    *tablePtr = table;
    makeGlobalRoot((cell **)tablePtr);
}

void InitializeHashtables() { 
    if (!ROMIZING) { 
        createHashTable(&UTFStringTable, UTF_TABLE_SIZE);
        createHashTable(&InternStringTable, INTERN_TABLE_SIZE);
        createHashTable(&ClassTable, CLASS_TABLE_SIZE);
    }
}


/*=========================================================================
 * FUNCTION:      getUString, getUStringX
 * OVERVIEW:      Returns a unique instance of a given C string
 * INTERFACE:
 *   parameters:  string:       Pointer to an array of characters
 *                stringLength: Length of string (implicit for getUString)
 *   returns:     A unique instance.  If strcmp(x,y)==0, then
 *                 getUString(x) == getUString(y), and
 *                 strcmp(UStringInfo(getUString(x)), x) == 0.
 * 
 * Note that getUString actually allows "string"s with embedded NULLs.
 * Two arrays of characters are equal only if they have the same length 
 * and the same characters.
 *=======================================================================*/

UString
getUString(const char* string) { 
    return getUStringX(string, strlen(string));
}


UString
getUStringX(const char* string, int stringLength)
{
    HASHTABLE table = UTFStringTable;
    /* The bucket in the hash table */
    int index =  stringHash(string, stringLength) % table->bucketCount;
    UTF_HASH_ENTRY *bucketPtr = (UTF_HASH_ENTRY *)&table->bucket[index];
    UTF_HASH_ENTRY bucket;
    /* Search the bucket for the corresponding string. */
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) { 
        const char *key = bucket->string;
        int length = bucket->length;
        if (length == stringLength) { 
            if ((key == string) ||
                  (key[0] == string[0] && memcmp(key, string, length) == 0)) {
                return bucket;
            }
        }
    }
    /* We have to create a new bucket.  Note that the string is embedded
     * into the bucket.  We always append a '\0' to the string so that if
     * the thing we were passed is a C String, it will continue to be a C
     * String */
    bucket = (UTF_HASH_ENTRY)mallocObject(SIZEOF_UTF_HASH_ENTRY(stringLength),
                                          GCT_UTF_HASH_ENTRY);
    /* Install the new item into the hash table */
    bucket->next = *bucketPtr;
    memcpy((char *)bucket->string, string, stringLength);
    bucket->string[stringLength] = '\0';
    
#ifdef NOT_YET_WORKING
    if (USESTATIC) { 
        int size = getObjectSize((cell *)bucket) << log2CELL;
        UTF_HASH_ENTRY newBucket = mallocStaticBytes(size);
        modifyStaticMemory(newBucket, 0, bucket, size);
        bucket = newBucket;
    }
#endif
    /* Give the item a key that uniquely represents it.  On purpose, we assign
     * a key that makes it easy for us to find the right bucket.
     * (key % table->bucketCount) = index.
     */
    bucket->key = table->bucketCount + 
                  ((bucket->next == NULL) ? index : bucket->next->key);
    bucket->length = stringLength;
    *bucketPtr = bucket;
    /* Increment the count, in case we need this information */
    table->count++;
    /* Return the string */
    return bucket;
}

/*=========================================================================
 * FUNCTION:      utf2unicode
 * OVERVIEW:      Converts UTF8 string to unicode char.
 *
 *   parameters:  utfstring_ptr: pointer to a UTF8 string. Set to point 
 *                to the next UTF8 char upon return.
 *   returns      unicode char
 *=======================================================================*/

short utf2unicode(const char **utfstring_ptr) {
    unsigned char *ptr = (unsigned char *)(*utfstring_ptr);
    unsigned char ch, ch2, ch3;
    int length = 1;     /* default length */
    short result = 0x80;    /* default bad result; */
    switch ((ch = ptr[0]) >> 4) {
        default:
        result = ch;
        break;

    case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
        /* Shouldn't happen. */
        break;

    case 0xC: case 0xD: 
        /* 110xxxxx  10xxxxxx */
        if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
        unsigned char high_five = ch & 0x1F;
        unsigned char low_six = ch2 & 0x3F;
        result = (high_five << 6) + low_six;
        length = 2;
        } 
        break;

    case 0xE:
        /* 1110xxxx 10xxxxxx 10xxxxxx */
        if (((ch2 = ptr[1]) & 0xC0) == 0x80) {
        if (((ch3 = ptr[2]) & 0xC0) == 0x80) {
            unsigned char high_four = ch & 0x0f;
            unsigned char mid_six = ch2 & 0x3f;
            unsigned char low_six = ch3 & 0x3f;
            result = (((high_four << 6) + mid_six) << 6) + low_six;
            length = 3;
        } else {
            length = 2;
        }
        }
        break;
    } /* end of switch */

    *utfstring_ptr = (char *)(ptr + length);
    return result;
}

/*=========================================================================
 * FUNCTION:      change_Name_to_Key change_Key_to_Name
 * OVERVIEW:      Converts between an array of bytes, and a unique 16-bit
 *                number (the Key)
 *
 * INTERFACE:   change_Name_to_key
 *   parameters:  string:       Pointer to an array of characters
 *                stringLength: Length of array
 *   returns      A unique 16-bit key
 *
 * INTERFACE:   change_key_to_Name
 *   parameters:  key: A unique 16-bit key
 *   lengthP:     If non-NULL, the length of the result is returned here.
 *
 *   returns      A pointer to the array of characters.
 *=======================================================================*/

NameKey
change_Name_to_Key(char *name, int length) { 
    UString UName = getUStringX(name, length);
    return UName->key;
}

char *
change_Key_to_Name(NameKey key, int *lengthP) { 
    HASHTABLE table = UTFStringTable;
    int index =  key % table->bucketCount;
    UTF_HASH_ENTRY *bucketPtr = (UTF_HASH_ENTRY *)&table->bucket[index];
    UTF_HASH_ENTRY bucket;
    /* Search the bucket for the corresponding string. */
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) { 
        if (key == bucket->key) { 
            if (lengthP) { 
                *lengthP = bucket->length;
            }
            return UStringInfo(bucket);
        }
    }
    return NULL;
}


/*=========================================================================
 * FUNCTION:      change_Name_to_CLASS
 * OVERVIEW:      Internal function used by internString and internClass.
 *                It maps a C string to a specific location capable of
 *                holding a value.  This location is initially NULL.
 * INTERFACE:
 *   parameters:  table:  A hashtable whose buckets are MAPPING_HASH_ENTRY.
 *                string:  A C string
 *   returns:     A unique location, capable of holding a value, corresponding
 *                to that string.
 *=======================================================================*/

CLASS
change_Name_to_CLASS(UString packageName, UString baseName, GCT_ObjectType typeCode) { 
    HASHTABLE table = ClassTable;
    unsigned int hash;
    unsigned int index;
    unsigned int lastKey = 0;
    CLASS *clazzPtr, clazz;

    hash = stringHash(UStringInfo(baseName), baseName->length) + 37;
    if (packageName != NULL) { 
        hash += stringHash(UStringInfo(packageName), packageName->length) * 3;
    } 
    index = hash % table->bucketCount;
    clazzPtr = (CLASS *)&table->bucket[index];

    for (clazz = *clazzPtr; clazz != NULL; clazz = clazz->next) { 
        if (clazz->packageName == packageName && clazz->baseName == baseName) { 
            return clazz;
        }
        if (lastKey == 0) { 
            unsigned thisKey = clazz->key;
            int pseudoDepth = thisKey >> FIELD_KEY_ARRAY_SHIFT;
            if (pseudoDepth == 0 || pseudoDepth == MAX_FIELD_KEY_ARRAY_DEPTH) { 
                lastKey = thisKey & 0x1FFF;
            }
        }
    }
    if (typeCode == GCT_INSTANCE_CLASS) { 
        clazz = (CLASS)callocObject(SIZEOF_INSTANCE_CLASS, typeCode);
    } else { 
        clazz = (CLASS)callocObject(SIZEOF_ARRAY_CLASS, typeCode);
    }
    clazz->next = *clazzPtr;
    *clazzPtr = clazz;

    clazz->packageName = packageName;
    clazz->baseName = baseName;
    /* The caller may change this value to be one of the values with 
     * depth 1 <= depth <= MAX_FIELD_KEY_ARRAY_DEPTH. */
    clazz->key = lastKey == 0 ? (256 + index) : (lastKey + table->bucketCount);
    table->count++;

    return clazz;

}
    
CLASS
change_Key_to_CLASS(FieldTypeKey key) 
{
    int depth = key >> FIELD_KEY_ARRAY_SHIFT;
    if (depth == 0 || depth == MAX_FIELD_KEY_ARRAY_DEPTH) { 
        HASHTABLE table = ClassTable;
        int index =  ((key & 0x1FFF) - 256) % table->bucketCount;
        CLASS *clazzPtr = (CLASS *)&table->bucket[index];
        CLASS clazz;
        /* Search the clazz for the corresponding string. */
        for (clazz = *clazzPtr; clazz != NULL; clazz = clazz->next) { 
            if (key == clazz->key) { 
                return clazz;
            }
        }
        return NULL;
    } else {
        char *tmpName = change_Key_to_FieldSignature(key);
        UString name = getUString(tmpName);
        return getClass(UStringInfo(name));
    }
}

/*=========================================================================
 * FUNCTION:      internString
 * OVERVIEW:      Returns a unique Java String that corresponds to a
 *                particular char* C string
 * INTERFACE:
 *   parameters:  string:  A C string
 *   returns:     A unique Java String, such that if strcmp(x,y) == 0, then
 *                internString(x) == internString(y).
 *=======================================================================*/

STRING_INSTANCE
internString(const char *utf8string, int length)
{ 
    HASHTABLE table = InternStringTable;
    unsigned int hash = stringHash(utf8string, length);
    unsigned int index = hash % table->bucketCount;
    STRING_HASH_ENTRY bucket, *bucketPtr;

    bucketPtr = (STRING_HASH_ENTRY *)&table->bucket[index];
    
    for (bucket = *bucketPtr; bucket != NULL; bucket = bucket->next) { 
        STRING_INSTANCE JString = bucket->string;
        SHORTARRAY chars = JString->array;
        int offset = JString->offset;
        const char *p = utf8string;
        int i = 0;
        unsigned short unicode_length = 0;
        bool_t matchFirstN = TRUE;
        while (p < utf8string + length) { 
            short unichar = utf2unicode(&p);
            unicode_length++;
            if (unichar != chars->sdata[offset + (i++)]) { 
                matchFirstN = FALSE;
                break;
            }
        }
        /* unicode_length is valid only if matchFirstN == TRUE */
        if (matchFirstN && unicode_length == JString->length) {
            return JString;
        }
    }
    bucket = (STRING_HASH_ENTRY)mallocObject(SIZEOF_STRING_HASH_ENTRY, 
                                             GCT_STRING_HASH_ENTRY);

    /* Let's get this bucket into the table, so we can then do allocation */
    bucket->next = *bucketPtr;
    *bucketPtr = bucket;
    bucket->string = NULL; /* for GC */
    bucket->string = instantiateString(utf8string, length);
    return bucket->string;
}


#if INCLUDEDEBUGCODE

void
printUTFStringTable() { 
    HASHTABLE table = UTFStringTable;
    int count = table->bucketCount;
    while (--count >= 0) { 
        UString bucket = (UString)table->bucket[count];
        for ( ; bucket != NULL; bucket = bucket->next) { 
            int length = bucket->length;
            char *string = bucket->string;
            if (length > 0 && string[0] < 20) { 
                change_Key_to_MethodSignature_inBuffer(bucket->key, str_buffer);
                fprintf(stdout, "***  \"%s\"\n", str_buffer);
            } else { 
                fprintf(stdout, "     \"%s\"\n", string);
            }
        }
    }
}
#endif


#if ROMIZING

/*=========================================================================
 * FUNCTION:      finalizeROMHashTable
 * OVERVIEW:      Return a ROM hashtable back to its "virgin" state.
 * INTERFACE:
 *   parameters:  table:  The hashtable
 *                nextOffset:  Offset of the "next" field within each
 *                entry of the hashtable.
 *
 * This code assumes that the non ROM entries are at the beginning of each
 * chain, and that the ROM entries are at the end.  Hence, it can scan
 * each chain, and just pop off the non-ROM entries until it reaches the end
 * or a ROM entry.
 *=======================================================================*/

void finalizeROMHashTable(HASHTABLE table, int nextOffset) 
{
    int bucketCount = table->bucketCount;
    int i;
    for (i = 0; i < bucketCount; i++) {
        void *p = table->bucket[i];
        while (p != NULL && inHeapSpace(p)) { 
            /* Move to the next bucket */
            p = *(void **)((char *)p + nextOffset);
        }
        table->bucket[i] = p;
    }
}

#endif
