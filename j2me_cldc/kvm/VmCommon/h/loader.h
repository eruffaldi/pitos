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
 * FILE:      loader.h
 * OVERVIEW:  Internal definitions needed for loading 
 *            Java class files (class loader). 
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *=======================================================================*/

/*=========================================================================
 * COMMENTS:
 * This file defines a JVM Specification compliant classfile reader.
 * It is capable of reading any standard Java classfile, and generating
 * the necessary corresponding VM runtime structures.
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

/*=========================================================================
 * Global variables
 *=======================================================================*/

/*=========================================================================
 * Classfile loading data structures
 *=======================================================================*/

/*  This structure is used for referring to open "files" when  */
/*  loading classfiles (records) from PalmPilot databases. */
/*  It replaces the standard FILE* structure used in C/C++. */

/*  FILEPOINTER */
struct filePointerStruct;

#define SIZEOF_FILEPOINTER       StructSizeInCells(filePointerStruct)

/*=========================================================================
 * Operations
 *=======================================================================*/

void loadClassfile(INSTANCE_CLASS CurrentClass, bool_t fatalErrorIfFail);
CLASS ParseCommandLine(int argc, char* argv[]);
void  InitializeClassPath(void);
FILEPOINTER openClassfile(const char *className);
FILEPOINTER openResourcefile(const char *resourceName);
void closeClassfile(FILEPOINTER);
bool_t classExists(const char* className);


enum verifyName_type {LegalMethod, LegalField, LegalClass};
bool_t verifyName(char* name, enum verifyName_type, bool_t abortOnError);


/*=========================================================================
 * Helper functions
 *=======================================================================*/

char* replaceLetters(char* string, char c1, char c2);

#if USES_CLASSPATH

extern char* UserClassPath;		/* set in main() or else where */

#ifndef PATH_SEPARATOR
#   define PATH_SEPARATOR ':'
#endif
#endif


int  loadByteNoEOFCheck(FILEPOINTER ClassFile);
unsigned char  loadByte(FILEPOINTER ClassFile);
unsigned short loadShort(FILEPOINTER ClassFile);
unsigned long  loadCell(FILEPOINTER ClassFile);
void           loadBytes(FILEPOINTER ClassFile, char *buffer, int length);
void           skipBytes(FILEPOINTER ClassFile, unsigned int i);
