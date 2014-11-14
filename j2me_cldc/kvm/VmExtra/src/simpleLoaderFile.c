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
 *	      Tasneem Sayeed, Consumer & Embedded division
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jar.h>

#define NOJARFILE
#define SEEK_CUR 0
#define SEEK_SET 1
#define SEEK_END 2
#define PATH_SEPARATOR ':'

struct stat {
	int k;
};

int getc(FILE *p) {return -1;}
int fseek(FILE*p, int b, int a) {return 0;}
int ftell(FILE*p) {return 0;}
void fclose(FILE*p) {}
int fread(void*a,int b,int c,FILE*p) {return 0;}
FILE*fopen(const char*b, const char*a) {return 0;}
#define S_ISDIR(x) 0
int stat(const char * n, struct stat * st) {return -1;}

char _toupper(char c)
{
return c;
}

struct filePointerStruct {
    bool_t isJarFile;		    /* Determines type of file: */
				    /*            JAR or classfile */
				    /* If set to true, indicates a JAR file */
    union { 
	FILE    *filePointer;	    /* Pointer to class file */
	JAR_DataStreamPtr jdstream; /* Pointer to JAR data stream */
    } u;
};

struct classPathEntryStruct { 
    union { 
	struct { 
	    unsigned long locpos;
	    unsigned long cenpos;
	} jar;
        /* We keep it a union, so that we can other possibilities later. */
    } u;
    char  type;
    char  name[1];
};


/*=========================================================================
 * Forward declarations
 *=======================================================================*/
static char* formatClassfileName(const char* className);
static FILEPOINTER openClassfileInternal(const char *fileName);


/*=========================================================================
 * Dynamic heap variables
 *=======================================================================*/

/* A table for storing the individual directory paths along classpath.
 * Each entry is one element in the list.
 *
 * An entry consists of the letter 'j' or 'd' followed by more information.
 * 'd' means this is a directory.  It is immediately followed by the name
 *      of the directory.
 * 'j' means a zip file.  It is followed by 3 ignored spaces, then 4 bytes
 *      giving the location of the first local header, 4 bytes giving
 *      the location of the first central header, and then the name of the
 *      zip file.
 */

static POINTERLIST ClassPathTable = NIL;   /*  object pointer (root) */


static unsigned int MaxClassPathTableLength = 0;

char* UserClassPath = NULL;     /* set in main() to classpath environment */

/*=========================================================================
 * JAR file reading operations
 *=======================================================================*/
/*=========================================================================
 * FUNCTION:      JAR_ReadBytes
 * TYPE:          JAR file reading
 * INTERFACE:
 *   parameters:  datastream, buffer, length
 *   returns:     the number of bytes read or -1 if an error is encountered.
 *=======================================================================*/

int JAR_ReadBytes(JAR_DataStreamPtr ds, char *buf, int len) {

    int avail;

    if (ds->mode != JAR_READ) {
        return -1;
    }
    if (len < 0) {
        return -1;
    }

    avail = ds->dataLen - ds->dataIndex;

    if (avail < len) {
        return -1;
    }
    memcpy(buf, ds->data + ds->dataIndex, len);
    ds->dataIndex += len;
    return len;
}


/*=========================================================================
 * FUNCTION:      JAR_WriteBytes
 * TYPE:          JAR file writing
 * OVERVIEW:      Writes out the bytes from the buffer to the data stream. 
 * INTERFACE:
 *   parameters:  datastream, buffer, length
 *   returns:     the number of bytes written or -1 if an error is encountered.
 *==========================================================================*/

int JAR_WriteBytes(JAR_DataStreamPtr ds, char *buf, int len) {
    
    int avail;

    if (ds->mode != JAR_WRITE) {
        return -1;
    }
    if (len < 0) {
        return -1;
    }

    avail = ds->dataLen - ds->dataIndex;

    if (avail < len) {
        return -1;
    }
    memcpy(ds->data + ds->dataIndex, buf, len);
    ds->dataIndex += len;
    return len;
}


/*========================================================================
 * FUNCTION:      JAR_SkipBytes
 * TYPE:          JAR file parsing
 * OVERVIEW:      Skips the bytes specified in len within the data stream. 
 * INTERFACE:
 *   parameters:  datastream, length
 *   returns:     the number of bytes skipped or -1 if an error is encountered.
 *==========================================================================*/

int JAR_SkipBytes(JAR_DataStreamPtr ds, int len) {

    int avail;

    if (ds->mode != JAR_READ) {
        return -1;
    }
    if (len < 0) {
        return -1;
    }

    avail = ds->dataLen - ds->dataIndex;
    if (avail < len) {
        return -1;
    }
    ds->dataIndex += len;
    return len;
}

/*========================================================================
 * FUNCTION:      JAR_CloseDataStream
 * TYPE:          JAR close data stream
 * OVERVIEW:      Closes the data stream and frees all associated structures. 
 * INTERFACE:
 *   parameters:  datastream
 *   returns:     nothing
 *==========================================================================*/

void JAR_CloseDataStream(JAR_DataStreamPtr ds) {
    if (ds != NULL) {
	removeTransientRootByValue((cell *)ds);
    }
}



#define LOCSIG (('P' << 0) + ('K' << 8) + (3 << 16) + (4 << 24))
#define CENSIG (('P' << 0) + ('K' << 8) + (1 << 16) + (2 << 24))
#define ENDSIG (('P' << 0) + ('K' << 8) + (5 << 16) + (6 << 24))

/* Helper function used by jar loading
 * 
 * It returns TRUE if it is successful, false otherwise.
 * If successful, 
 *      *locposPtr is set to the position of the first local header
 *      *cenposPtr is set to the position of the first central header
 *
 * Note that *locposPtr is the logical "0" of the file.  All offsets
 * extracted need to have this value added to them.
 */

static bool_t findJARDirectories(FILE *file, 
				 unsigned long *locposPtr,
				 unsigned long *cenposPtr)
{
    unsigned char *buffer = (unsigned char *)str_buffer;
    unsigned const int bufferSize = STRINGBUFFERSIZE;
    int length, position, minPosition;
    unsigned char *bp;
    
    /* Read in the last ENDHDRSIZ bytes into the buffer.  99% of the time,
     * the file won't have a comment, and this is the only read we'll need */
    if (   (fseek(file, -ENDHDRSIZ, SEEK_END) < 0)  
        || (fread(buffer, sizeof(char), ENDHDRSIZ, file) != ENDHDRSIZ)) { 
	return FALSE;
    }

    /* Get the length of the file */
    length = ftell(file);	    
    /* Get the position in the file stored into buffer[0] */
    position = length - ENDHDRSIZ; 
    /* Calculate the smallest possible position for the end header.  It
     * can be at most 0xFFFF + ENDHDRSIZ bytes from the end of the file, but
     * the file must also have a local header and a central header 
     */
    minPosition = length - (0xFFFF + ENDHDRSIZ);
    if (minPosition < LOCHDRSIZ + CENHDRSIZ) { 
	minPosition = LOCHDRSIZ + CENHDRSIZ;
    }
    bp = buffer;
    for (;;) { 
	/* "buffer" contains a block of data from the file, starting at
	 * position "position" in the file.  
	 * We investigate whether   position + (bp - buffer)  is the start
	 * of the end header in the zip file.  This file position is at
	 * position bp in the buffer.
	 */
	/* Use simplified version of Knuth Morris Pratt search algorithm. */
	switch(bp[0]) { 
	    case '\006':   /* The header must start at least 3 bytes back */
		bp -= 3; break;
	    case '\005':   /* The header must start at least 2 bytes back  */
		bp -= 2; break;
	    case 'K':	   /* The header must start at least 1 byte back  */
		bp -= 1; break;
	    case 'P':	   /* Either this is the header, or the header must
			    * start at least 4  back */
		if (bp[1] == 'K' && bp[2] == 5 && bp[3] == 6) {  
		    int endpos = position + (bp - buffer);
		    if (endpos + ENDHDRSIZ + ENDCOM(bp) == length) {
			unsigned int cenpos = endpos - ENDSIZ(bp);
			unsigned int locpos = cenpos - ENDOFF(bp);
			*cenposPtr = cenpos;
			*locposPtr = locpos;
			return TRUE;
		    } 
		}
		/* FALL THROUGH */
	    default:	  /* This char isn't in the header signature, so
			   * the header must start at least four chars back */
		bp -= 4;
	}
	if (bp < buffer) { 
	    /* We've moved outside our window into the file.  We must
	     * move the window backwards */
	    int count = position - minPosition; /* Bytes left in file */
	    if (count == 0) { 
		/* Nothing left to read.  Time to give up */
		return FALSE;
	    } else { 
		/* up to ((bp - buffer) + ENDHDRSIZ) bytes in the buffer might
		 * still be part of the end header, so the most bytes we can
		 * actually read are 
		 *      bufferSize - ((bp - buffer) + ENDHDRSIZE).
		 */
		int available = (bufferSize - ENDHDRSIZ) + (buffer - bp);
		if (count > available) { 
		    count = available;
		}
	    }
	    /* Back up, while keeping our virtual position the same */
	    position -= count;
	    bp += count;
	    memmove(buffer + count, buffer, bufferSize - count);
	    if (   (fseek(file, position, SEEK_SET) < 0)
	        || (fread(buffer, sizeof(char), count, file) != (unsigned)count)) { 
		return FALSE;
	    }
	}
    }
}


/*=========================================================================
 * FUNCTION:      jarCRC32
 * OVERVIEW:      Returns the CRC of an array of bytes, using the same
 *                algorithm as used by the JAR reader.
 * INTERFACE:
 *   parameters:  data:     pointer to the array of bytes
 *                length:   length of data, in bytes
 *   returns:     CRC
 *=======================================================================*/




static unsigned long
jarCRC32(unsigned char *data, unsigned long length) { 
    unsigned long crc = 0xFFFFFFFF;
    unsigned int j;
    for ( ; length > 0; length--, data++) { 
	crc ^= *data;
	for (j = 8; j > 0; --j) { 
	    crc = (crc & 1) ? ((crc >> 1) ^ 0xedb88320) : (crc >> 1);
	}
    }
    return ~crc;
}


/*=========================================================================
 * FUNCTION:      loadJARfile()
 * TYPE:          load JAR file  
 * OVERVIEW:      Internal function used by openClassfileInternal().
 *   
 *  This function reads the specified file from the JAR file.  The
 *  result is returned as a JAR_DataStream*.  NULL is returned if it
 *  cannot find the file, or there is some error.
 *
 * INTERFACE:
 *   parameters:  JARfile:  name of the JAR file
 *                locpos:   location of first local header in jar file
 *                cenpos:   location of first central header in jar file
 *                filename: filename to search for
 *   returns:     JAR_DataStream* for saving the JAR file info, or NULL.
 *=======================================================================*/
#ifndef NOJARFILE
JAR_DataStreamPtr
loadJARfile(const char *JARfile, unsigned long locpos, unsigned long cenpos, 
	    const char* filename) 
{

    unsigned filenameLength = strlen(filename);
    char *p = str_buffer;	/* temporary storage */

    FILE *file;
    int offset = 0;

    file = fopen(JARfile, "rb");
    if (file == NULL) { 
	goto error;
    }

    offset = cenpos;		/* Go to the start of the central headers */
    for (;;) { 
	unsigned int nameLength;

	if (/* Go to the next central header */
	       (fseek(file, offset, SEEK_SET) < 0) 
	    /* Read the bytes */
	    || (fread(p, sizeof(char), CENHDRSIZ, file) != CENHDRSIZ) 
	    /* Make sure it is a header */
	    || (GETSIG(p) != CENSIG)) { 
	    goto error;
	}
	/* Get the nameLength */
	nameLength = CENNAM(p);

	if (nameLength == filenameLength) { 
	    if (fread(p + CENHDRSIZ, sizeof(char), nameLength, file) 
		   != nameLength) {
		goto error;
	    }
	    if (memcmp(p + CENHDRSIZ, filename, nameLength) == 0) { 
		break;
	    } 
	}

	/* Set offset to the next central header */
	offset += CENHDRSIZ + nameLength + CENEXT(p) + CENCOM(p);
    }

    /* p points at the central header for the file */
    { 
	unsigned long decompLen = CENLEN(p); /* the decompressed length */
	unsigned long compLen   = CENSIZ(p); /* the compressed length */
	unsigned long method    = CENHOW(p); /* how it is stored */
	unsigned long expectedCRC = CENCRC(p); /* expected CRC */

	unsigned char *decompData;
	JAR_DataStreamPtr jdstream;

	/* Make sure file is not encrypted */
	if ((CENFLG(p) & 1) == 1) {
	    fprintf(stderr, "Entry is encrypted");
	    return FALSE;
	}
	
	if (/* Go to the beginning of the LOC header */
	      (fseek(file, locpos + CENOFF(p), SEEK_SET) < 0) 
	    /* Read it */
	    || (fread(p, sizeof(char), LOCHDRSIZ, file) != LOCHDRSIZ) 
	    /* Skip over name and extension, if any */
	    || (fseek(file, LOCNAM(p) + LOCEXT(p), SEEK_CUR) < 0)) {
	    goto error;
	}
	jdstream = 
	    (JAR_DataStreamPtr)mallocBytes(sizeof(JAR_DataStream) + decompLen); 
	decompData = (unsigned char *)(jdstream + 1);

	switch (method) { 
	    case STORED:
		if (compLen != decompLen) {
		    return NULL;
		}
		fread(decompData, sizeof(char), decompLen, file);
		break;

	    case DEFLATED:
		START_TEMPORARY_ROOTS
		   MAKE_TEMPORARY_ROOT(jdstream);
#if JAR_INFLATER_USES_STDIO
		   if (!inflate(file, compLen, decompData, decompLen)) { 
		       jdstream = NULL;
		   }
#else 
		   {

		       unsigned char *compData =  (unsigned char *)
			   mallocBytes(compLen + INFLATER_EXTRA_BYTES);
		       MAKE_TEMPORARY_ROOT(compData);
		       if (/* Read the bytes */
			   (fread(compData, sizeof(char), compLen, file) 
			          != compLen)
			   /* Inflate them into a buffer */
			   || !inflate(compData, compLen, 
				       decompData, decompLen)){
			   jdstream = NULL;
		       }
		   }
#endif
		END_TEMPORARY_ROOTS
		break;

	    default:
		jdstream = NULL;
		break;
	}

	fclose(file);
	{ 
	    unsigned long actualCRC = jarCRC32(decompData, decompLen);
	    if (actualCRC != expectedCRC) { 
		fprintf(stdout,"Unexpected CRC value");
	    }
	}

	if (jdstream != NULL) { 
	    jdstream->type = JAR_RESOURCE;
	    jdstream->data = decompData;
	    jdstream->dataLen = decompLen;
	    jdstream->dataIndex = 0;
	    jdstream->mode = JAR_READ;
	}
	return jdstream;
    }

 error:
    if (file != NULL) { 
	fclose(file);
    }
    return NULL;
}

#endif
/*=========================================================================
 * Generic file reading operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      openClassFile
 * TYPE:          class reading
 * OVERVIEW:      Returns a FILEPOINTER object to read the bytes of the 
 *                given class name
 * INTERFACE:
 *   parameters:  className:  Name of class to read
 *   returns:     a FILEPOINTER, or error if none exists
 *=======================================================================*/


FILEPOINTER
openClassfile(const char *className) { 
    char *fileName = formatClassfileName(className);
    FILEPOINTER ClassFile = openClassfileInternal(fileName);
    if (ClassFile == NULL) { 
        if (Verbose_On) {
            sprintf(str_buffer, "Class %s not found.", className);
        }
    }
    return ClassFile;
}

/*=========================================================================
 * FUNCTION:      openResourceFile
 * TYPE:          resource reading
 * OVERVIEW:      Returns a FILEPOINTER object to read the bytes of the 
 *                given resource name
 * INTERFACE:
 *   parameters:  resourceName:  Name of resource to read
 *   returns:     a FILEPOINTER, or error if none exists
 *=======================================================================*/


FILEPOINTER
openResourcefile(const char *resourceName) { 
    FILEPOINTER ClassFile = openClassfileInternal(resourceName);
    if (ClassFile == NULL) {
        if (Verbose_On) {
            sprintf(str_buffer, "Resource %s not found.", resourceName);
        }
    }
    return ClassFile;
}


/*=========================================================================
 * FUNCTION:      openClassfileInternal()
 * TYPE:          class reading
 * OVERVIEW:      Internal function used by openClassfile() and classExists()
 *                It returns a FILE* for reading the class if a file exists.
 *                NULL otherwise.
 * INTERFACE:
 *   parameters:  fileName:  Name of class file to read
 *   returns:     FILE* for reading the class, or NULL.
 *=======================================================================*/

static FILEPOINTER 
openClassfileInternal(const char *filename) { 
    int filenameLength = strlen(filename);
    POINTERLIST pathTable = ClassPathTable;
    int paths = pathTable->length;
    char *fullname;
    int i;
    FILE *file = NULL;
    JAR_DataStreamPtr jdstream;
    FILEPOINTER fp = NULL;

    START_TEMPORARY_ROOTS
	MAKE_TEMPORARY_ROOT(filename);
        fullname = 
	    (char*)mallocBytes(MaxClassPathTableLength + filenameLength + 2);
	MAKE_TEMPORARY_ROOT(fullname); /* probably not necessary */

        for (i = 0; i < paths && fp == NULL; i++) { 
	    struct classPathEntryStruct *entry = 
		(struct classPathEntryStruct *)pathTable->data[i].cellp;
	    /* The first letter of each path tells whether it is a jar file
	     * or a directory.  Everything past the first letter is the
	     * actual file and/or directory name.
	     */
	    switch (entry->type) { 

	    case 'd':		/* A directory */
                sprintf(fullname, "%s/%s", entry->name, filename);
                file = fopen(fullname, "rb");
                if (file != NULL) {
		    fp = (FILEPOINTER)callocObject(
			    sizeof(struct filePointerStruct)/4, GCT_NOPOINTERS);
                    fp->u.filePointer = file;
		    fp->isJarFile = FALSE;
                }
		break;

	    case 'j':  /* a jar file */
#ifndef NOJARFILE
                jdstream = loadJARfile(entry->name, 
				       entry->u.jar.locpos, entry->u.jar.cenpos,
				       filename);
#endif
		if (jdstream != NULL) { 
		    makeTransientRoot((cell *)jdstream);
		    fp = (FILEPOINTER)callocObject(
			    sizeof(struct filePointerStruct)/4, GCT_NOPOINTERS);
                    fp->isJarFile = TRUE;
                    fp->u.jdstream = jdstream;
		}
		break;
            }
        }
    END_TEMPORARY_ROOTS
    return fp;
}


/*=========================================================================
 * FUNCTION:      loadByte(), loadShort(), loadCell()
 * TYPE:          private class file reading operations
 * OVERVIEW:      Read the next 1, 2 or 4 bytes from the 
 *                given class file.
 * INTERFACE:
 *   parameters:  classfile pointer
 *   returns:     unsigned char, short or integer
 * NOTE:          For safety it might be a good idea to check
 *                explicitly for EOF in these operations.
 *=======================================================================*/

int
loadByteNoEOFCheck(FILEPOINTER ClassFile)
{
    int c;
    
    if (!ClassFile->isJarFile) {
        c = getc(ClassFile->u.filePointer);
    } else {
	unsigned char ch;
        /* load byte from the JAR file */
	if (JAR_ReadBytes(ClassFile->u.jdstream, (char*)(&ch), 1) < 0) {
	    return EOF;
	}
	c = ch;
    }
    return c;
}

unsigned char 
loadByte(FILEPOINTER ClassFile)
{
    int c = loadByteNoEOFCheck(ClassFile);
    if (c == EOF) {
        fatalError("loadByte: truncated class file");
    }
    return (unsigned char)c;
}

unsigned short 
loadShort(FILEPOINTER ClassFile)
{
    unsigned char c1 = loadByte(ClassFile);
    unsigned char c2 = loadByte(ClassFile);
    unsigned short c  = c1 << 8 | c2;
    return c;
}

unsigned long
loadCell(FILEPOINTER ClassFile)
{
    unsigned char c1 = loadByte(ClassFile);
    unsigned char c2 = loadByte(ClassFile);
    unsigned char c3 = loadByte(ClassFile);
    unsigned char c4 = loadByte(ClassFile);
    unsigned int c;
    c  = c1 << 24 | c2 << 16 | c3 << 8 | c4;
    return c;
}

void 
loadBytes(FILEPOINTER ClassFile, char *buffer, int length) 
{
    int n;
    if (!ClassFile->isJarFile) {
        n = fread(buffer, 1, length, ClassFile->u.filePointer);
    } else { 
        /* load bytes from the JAR file */
        n = JAR_ReadBytes(ClassFile->u.jdstream, buffer, length);
    }
    if (n != length) {
        fatalError("loadBytes: truncated class file");
    }
}

void
skipBytes(FILEPOINTER ClassFile, unsigned int i)
{ 
    if (!ClassFile->isJarFile) {
        if (fseek(ClassFile->u.filePointer, i, SEEK_CUR) < 0) {
        fatalError("bad class file");
    } 
    } else {
        /* skip the bytes specified within the JAR file */
        if (JAR_SkipBytes(ClassFile->u.jdstream, i) < 0) {
            fatalError("bad class file");
        }
    }
}


/*=========================================================================
 * FUNCTION:      closeClassfile()
 * TYPE:          private class file load operation
 * OVERVIEW:      Close the given classfile. Ensure that we have 
 *                reached the end of the classfile.
 * INTERFACE:
 *   parameters:  file pointer
 *   returns:     <nothing>
 *=======================================================================*/

void closeClassfile(FILEPOINTER ClassFile)
{
    if (Verbose_On) fprintf(stdout,"Closing classfile\n");
        if (ClassFile->isJarFile) {
        /* Close the JAR datastream */
        JAR_CloseDataStream(ClassFile->u.jdstream);
    } else {
        /* Close the classfile */
        if (ClassFile->u.filePointer)
            fclose(ClassFile->u.filePointer);
    }
}


/*=========================================================================
 * Classfile opening operations
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      formatClassfileName()
 * TYPE:          auxiliary private function
 * OVERVIEW:      Make the given class name a proper classfile name string
 * INTERFACE:
 *   parameters:  class name string
 *   returns:     string to a properly formatted classfile name
 * NOTE:          This operation will create garbage when the pointer
 *                to the created string is later released.
 *=======================================================================*/

static char* 
formatClassfileName(const char* className)
{
    int   length = strlen(className);
    char* fileName = (char*)mallocBytes(length + 7);
    strcpy(fileName, className);

    /*  Check if the given class is an array type */
    if (fileName[length-1] == ';') {
        int i;
        /*  If so, make the class name a normal class name */
        fileName[--length] = 0; /*  Remove the ';' */
        /*  Remove everything before first 'L' (start of actual class name) */
        for (i = 0; fileName[i] != 'L'; i++);
        if (i < length) {
            memmove(fileName, fileName+i+1, length-i+1);
            length -= i+1;
        } else {
            fatalError("Class format error: Illegal class signature.");
        }
    }
    
    strcpy(fileName + length, ".class");
    length += 6;
    return fileName;
}

/*=========================================================================
 * FUNCTION:      InitializeClassPath()
 * TYPE:          constructor (kind of)
 * OVERVIEW:      Read the optional environment variable CLASSPATH
 *                and set up the class path for classfile loading
 *                correspondingly.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     nothing directly, but the operation initializes
 *                the global classpath variables.
 *=======================================================================*/

void InitializeClassPath()
{
    char *classpath = UserClassPath;
    int length, pathCount, i;

    int tableIndex;
    int previousI;

    /*  Count the number of individual directory paths along CLASSPATH */
    length = strlen(classpath);
    pathCount = 1;

    for (i = 0; i < length; i++) {
        if (classpath[i] == PATH_SEPARATOR) pathCount++;
    }

    /* We use callocObject() since we'll be allocating new objects to fill
     * up the table */
    ClassPathTable = (POINTERLIST)callocObject(SIZEOF_POINTERLIST(pathCount), 
            GCT_POINTERLIST); 
    makeGlobalRoot((cell **)&ClassPathTable);

    ClassPathTable->length = pathCount;
    MaxClassPathTableLength = 0;

    tableIndex = 0;
    previousI = 0;

    for (i = 0; i <= length; i++) {
       if (classpath[i] == PATH_SEPARATOR || classpath[i] == 0) {
	   struct stat sbuf;
	   /* Create a new string containing the individual directory path. 
	    * We prepend either a 'j' (jar file) or 'd' (directory) indicating
	    * the type of classpath element this is. 
	    */
	   unsigned int length = i - previousI;
	   struct classPathEntryStruct *result = 
	       (struct classPathEntryStruct *)mallocBytes(
			 sizeof(struct classPathEntryStruct) + length);
	   char *suffix;

	   /* Copy the name into the result buffer */
	   memcpy(result->name, classpath + previousI, length);
	   result->name[length] = '\0';
	   result->type = '\0';	/* indicates a problem */
	   
	   if (stat(result->name, &sbuf) < 0) { 
	       /* No need to do anything */
	   } else if (S_ISDIR(sbuf.st_mode)) { 
	       /* This is a directory */
	       /*  Store the newly created string to the classpath table */
	       if (length > MaxClassPathTableLength) { 
		   MaxClassPathTableLength = length;
	       }
	       result->type = 'd';
	   } else if (   length >= 4
		      && ((suffix = result->name + length - 4)[0] == '.')
		      && (    (   (_toupper(suffix[1]) == 'Z') 
			       && (_toupper(suffix[2]) == 'I') 
			       && (_toupper(suffix[3]) == 'P'))
		           || (   (_toupper(suffix[1]) == 'J') 
			       && (_toupper(suffix[2]) == 'A') 
			       && (_toupper(suffix[3]) == 'R')))) { 
	       FILE *file = fopen(result->name, "rb");
	       if (file != NULL) { 
		   /* Note that findJARDirectories can overwrite str_buffer */
		   if (findJARDirectories(file, 
					  &result->u.jar.locpos,
					  &result->u.jar.cenpos)) { 
		       result->type = 'j';
		   }
		   fclose(file);
	       }
	   }
	   if (result->type == '\0') { 
	       /* bad entry  */
	       ClassPathTable->length--;
	   } else { 
	       ClassPathTable->data[tableIndex].cellp = (cell*)result;
	       tableIndex++;
	   }
	   previousI = i+1;
       }
    } 
}
