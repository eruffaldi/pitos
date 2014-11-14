/*
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
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
 * KVM 
 *=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Hash table declarations
 * FILE:      hashtable.h
 * OVERVIEW:  This file defines the structures for maintaining 
 *            a hash table.
 * AUTHOR:    Frank Yellin
 *=======================================================================*/

#include <stddef.h>

/*=========================================================================
 * Constants
 *=======================================================================*/

/* The sizes of the various hash tables */

#define UTF_TABLE_SIZE 256
#define CLASS_TABLE_SIZE 32
#define INTERN_TABLE_SIZE 32

/* The declaration of a hashtable.  We make the buckets fairly
 * generic.
 */

typedef struct HashTable {
	int bucketCount;			/* number of buckets */
	int count;					/* number of total items in the table */
	cell *bucket[1];			/* Array of entries */
} *HASHTABLE;


/* The declaration of one bucket that holds unique instances of UTF strings
 */

typedef struct UTF_Hash_Entry { 
    struct UTF_Hash_Entry *next; /* The next bucket */
	unsigned short length;
	unsigned short key;
	char string[1];		 /* The characters of the string */
} *UTF_HASH_ENTRY, *UString;


typedef struct String_Hash_Entry { 
    struct   String_Hash_Entry *next; /* The next bucket */
	STRING_INSTANCE string;
} *STRING_HASH_ENTRY;


/*=========================================================================
 * Size macros
 *=======================================================================*/

/* A hashtable with n buckets */
#define SIZEOF_HASHTABLE(n)     (StructSizeInCells(HashTable) + (n - 1))

/* A UTF_HASH_ENTRY that can hold a string of length n.  Note that
 * this >>does<< include the NULL character, since we've already saved
 * space for one character */
 
#define SIZEOF_UTF_HASH_ENTRY(n) \
       ByteSizeToCellSize(offsetof(struct UTF_Hash_Entry, string) + n + 1)

#define SIZEOF_STRING_HASH_ENTRY (StructSizeInCells(String_Hash_Entry))

/*=========================================================================
 * Variables needed outside
 *=======================================================================*/

extern HASHTABLE InternStringTable;
extern HASHTABLE UTFStringTable;
extern HASHTABLE ClassTable;	/* hashtable containing all the classes */

/*=========================================================================
 * Functions used outside this code.
 *=======================================================================*/

/* Called at startup to initialize the hashtables */
void InitializeHashtables(void);

/* Convert a const char* to a UString, or a UString to a const char */
UString getUString(const char* string);
UString getUStringX(const char* string, int length);

#define UStringInfo(str) str->string

/* Get a unique String for a given char array */
STRING_INSTANCE internString(const char *string, int length);

/* Create a new hash table */
void createHashTable(HASHTABLE *tablePtr, int bucketCount);

/* Convert utf8 to unicode */
short utf2unicode(const char **utf);

/* Conversion between keys and names */

char *change_Key_to_Name(NameKey, int *length);
NameKey change_Name_to_Key(char *, int);

/* Conversion between classes and names */
CLASS change_Name_to_CLASS(UString package, UString base, GCT_ObjectType);
CLASS change_Key_to_CLASS(FieldTypeKey);


#if ROMIZING
void finalizeROMHashTable(HASHTABLE, int offset);
#endif
