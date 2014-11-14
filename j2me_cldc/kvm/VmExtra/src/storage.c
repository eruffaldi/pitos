/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 *
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */

/**
 * This implements the "storage:" protocol for Windows/unix
 *
 * @author  Nik Shaylor
 * @version 1.0 2/23/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <async.h>
#include <storage.h>

/*=========================================================================
 * Protocol Methods
 *=======================================================================*/


/*
 * native void prim_realOpen(String name, boolean writing) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1realOpen) {
    long            writing   = ASYNC_POP_STACK();
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    prim_realOpen(instance, &exception, name, writing);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_RESUME_THREAD();
}


/*
 * native void prim_close() throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1close) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    prim_close(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_findFirstFile();
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1findFirstItem) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    STRING_INSTANCE s = prim_findFirstItem(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, s);
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_findFile(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1findItem) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    STRING_INSTANCE s = prim_findItem(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, s);
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_findFileAfter(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1findItemAfter) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    STRING_INSTANCE s = prim_findItemAfter(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, s);
    ASYNC_RESUME_THREAD();
}


/*
 * native long prim_availableSpace();
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1availableSpace) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    long64 v = prim_availableSpace(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_LONG(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native int prim_countItems();
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1countItems) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    int v = prim_countItems(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_create() throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1create) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    STRING_INSTANCE s = prim_create(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, s);
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_createFile(String name) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1createFile) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    prim_createFile(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, string);
    ASYNC_RESUME_THREAD();
}


/*
 * native String prim_createDirectory(String name) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1createDirectory) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    prim_createDirectory(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, string);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_deleteItem(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1deleteItem) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    int res = prim_deleteItem(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(res);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_renameItem(String name, String name2);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1renameItem) {
    STRING_INSTANCE string2   = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    STRING_INSTANCE string1   = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf1[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string1, buf1, PATH_BUFFER_SIZE);
    char            buf2[PATH_BUFFER_SIZE];
    char           *name2 = getStringContentsSafely(string2, buf2, PATH_BUFFER_SIZE);

    int res = prim_renameItem(instance, &exception, name, name2);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(res);
    ASYNC_RESUME_THREAD();
}


/*
 * native long prim_lengthOf(String name) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1lengthOf) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    long64 v = prim_lengthOf(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_LONG(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native long prim_timeOf(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1timeOf) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    long64 v = prim_timeOf(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_LONG(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_exists(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1exists) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_exists(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_isDirectory(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1isDirectory) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_isDirectory(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_canRead(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1canRead) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_canRead(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}

/*
 * native boolean prim_canWrite(String name);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1canWrite) {
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_canWrite(instance, &exception, name);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}

/*
 * native boolean prim_setReadable(String name, boolean tf);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1setReadable) {
    long            val       = ASYNC_POP_STACK();
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_setReadable(instance, &exception, name, val);
    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native boolean prim_setWritable(String name, boolean tf);
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1setWritable) {
    long            val       = ASYNC_POP_STACK();
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    char            buf[PATH_BUFFER_SIZE];
    char           *name = getStringContentsSafely(string, buf, PATH_BUFFER_SIZE);

    bool_t v = prim_setWritable(instance, &exception, name, val);
    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native void prim_seek(long pos) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1seek) {
    INSTANCE        instance;
    ulong64         offset;
    char           *exception = NULL;
    ASYNC_POP_LONG(offset);
    instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    prim_seek(instance, &exception, offset);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_RESUME_THREAD();
}


/*
 * native long prim_getPosition() throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1getPosition) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    long64 v = prim_getPosition(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_LONG(v);
    ASYNC_RESUME_THREAD();
}


/*
 * native int prim_read() throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1read) {
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    int v = prim_read(instance, &exception);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}

/*
 * native int prim_readBytes(byte b[], int off, int len) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1readBytes) {
    long            length = ASYNC_POP_STACK();
    long            offset = ASYNC_POP_STACK();
    BYTEARRAY       buffer = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    int v = prim_readBytes(instance, &exception, (buffer->bdata), offset, length);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_PUSH_STACK(v);
    ASYNC_RESUME_THREAD();
}

/*
 * native int prim_write(int ch) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1write) {
    long            ch        = ASYNC_POP_STACK();
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    prim_write(instance, &exception, ch);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_RESUME_THREAD();
}

/*
 * native int prim_writeBytes(byte b[], int off, int len) throws IOException;
 */
ASYNC_FUNCTION(Java_com_sun_cldc_io_j2me_storage_Protocol_prim_1writeBytes) {
    long            length = ASYNC_POP_STACK();
    long            offset = ASYNC_POP_STACK();
    BYTEARRAY       buffer = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;

    prim_writeBytes(instance, &exception, (buffer->bdata), offset, length);

    if(exception) {
        ASYNC_EXCEPTION(exception);
    }
    ASYNC_RESUME_THREAD();
}








#ifndef PILOT

#ifdef sp
#undef sp
#undef ip
#undef fp
#endif

void Java_com_sun_kjava_Database_create(void) {
    int resourceDB = popStack();
    int typeID     = popStack();
    int creatorID  = popStack();
    STRING_INSTANCE name  = (STRING_INSTANCE)popStack();
    int cardNumber = topStack;

    char* rawName = getStringContents(name);
    int err;

    err = Fake_DmCreateDatabase(cardNumber, rawName, creatorID, typeID, resourceDB);

    if (err == 0)
         topStack = TRUE;
    else topStack = FALSE;
}

void Java_com_sun_kjava_Database_open(void) {
    long mode      = popStack();
    long creatorID = popStack();
    long typeID    = topStack;

    long dbRef = (long)Fake_DmOpenDatabaseByTypeCreator(typeID, creatorID, mode);

    topStack = dbRef;
}

void Java_com_sun_kjava_Database_close(void) {
    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)popStack();

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Close the database */
    if (dbRef) Fake_DmCloseDatabase(dbRef);
}

void Java_com_sun_kjava_Database_getNumberOfRecords(void) {
    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Read the number of records */
    if (dbRef) {
         topStack = Fake_DmNumRecords(dbRef);
    } else {
         topStack = 0;
    }
}

static char dbuffer[65536];

void Java_com_sun_kjava_Database_getRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    int length = Fake_DmGetRecord(dbRef, recordNumber, dbuffer);

    /* Skip deleted/non-existing records */
    if (length >= 0) {
        /* Instantiate new bytearray */
        ARRAY_CLASS bac = (ARRAY_CLASS)getClass("[B");
        BYTEARRAY byteArray = (BYTEARRAY)instantiateArray(bac, length);

        /* Copy data from record to bytearray */
        int i;
        for (i = 0; i < length; i++) {
            byteArray->bdata[i] = dbuffer[i];
        }

        /* Unlock the record handle and release the record */
        topStack = (cell)byteArray;
    } else {
        topStack = NIL;
    }
}

void Java_com_sun_kjava_Database_setRecord(void) {

    BYTEARRAY byteArray = (BYTEARRAY)popStack();

    unsigned short recordNumber = (unsigned short)popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    topStack = Fake_DmWrite(dbRef, recordNumber, byteArray->bdata, byteArray->length);
}

void Java_com_sun_kjava_Database_deleteRecord(void) {

    int recordNumber = popStack();

    /* Get the database instance pointer from 'this' */
    INSTANCE database = (INSTANCE)topStack;

    /* Read the first instance variable (dbRef) */
    long dbRef = database->data[0].cell;

    /* Assume that operation may fail */
    topStack = FALSE;

    /* Ensure that database is open */
    if (!dbRef) return;

    /* Remove the record from the database */
    topStack = Fake_DmRemoveRecord(dbRef, recordNumber);
}

void Java_com_sun_kjava_Database_readRecordToBuffer(void) {
    fatalError("not implemented");
}

void Java_com_sun_kjava_Database_writeRecordFromBuffer(void) {
    fatalError("not implemented");
}

#endif
