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
 * SUBSYSTEM: Network function definitions
 * FILE:      storage.h
 * AUTHOR:    Nik Shaylor
 *=======================================================================*/

#define PATH_BUFFER_SIZE 256

int prim_realOpen(INSTANCE instance, char **exception, char *name, bool_t writing);
int prim_close(INSTANCE instance, char **exception);
STRING_INSTANCE prim_findFirstItem(INSTANCE instance, char **exception);
STRING_INSTANCE prim_findItem(INSTANCE instance, char **exception, char *name);
STRING_INSTANCE prim_findItemAfter(INSTANCE instance, char **exception, char *name);
long64 prim_availableSpace(INSTANCE instance, char **exception);
int prim_countItems(INSTANCE instance, char **exception);
STRING_INSTANCE prim_create(INSTANCE instance, char **exception);
int prim_createFile(INSTANCE instance, char **exception, char *name);
int prim_createDirectory(INSTANCE instance, char **exception, char *name);
bool_t prim_deleteItem(INSTANCE instance, char **exception, char *name);
int prim_renameItem(INSTANCE instance, char **exception, char *name, char *name2);
long64 prim_lengthOf(INSTANCE instance, char **exception, char *name);
long64 prim_timeOf(INSTANCE instance, char **exception, char *name);
bool_t prim_exists(INSTANCE instance, char **exception, char *name);
bool_t prim_isDirectory(INSTANCE instance, char **exception, char *name);
bool_t prim_canRead(INSTANCE instance, char **exception, char *name);
bool_t prim_canWrite(INSTANCE instance, char **exception, char *name);
bool_t prim_setReadable(INSTANCE instance, char **exception, char *name, bool_t tf);
bool_t prim_setWritable(INSTANCE instance, char **exception, char *name, bool_t tf);
int prim_seek(INSTANCE instance, char **exception, long64 pos);
long64 prim_getPosition(INSTANCE instance, char **exception);
int prim_read(INSTANCE instance, char **exception);
int prim_readBytes(INSTANCE instance, char **exception, char *b, int offset, int length);
int prim_write(INSTANCE instance, char **exception, int b);
int prim_writeBytes(INSTANCE instance, char **exception, char *b, int offset, int length);


#ifndef PILOT

int Fake_DmCreateDatabase(int cardNumber, char *rawName, 
			  int creatorID, int typeID, int resourceDB);
long Fake_DmOpenDatabaseByTypeCreator(int typeID, int creatorID, int mode);
int Fake_DmCloseDatabase(long dbRef);
int Fake_DmNumRecords(long dbRef);
int Fake_DmGetRecord(long dbRef, int recordNumber, char *buffer);
int Fake_DmWrite(long dbRef, int recordNumber, char *data, int length);
int Fake_DmRemoveRecord(long dbRef, int recordNumber);

#endif
