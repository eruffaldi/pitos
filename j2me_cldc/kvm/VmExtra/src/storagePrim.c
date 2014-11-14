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

/*#define STORAGEDEBUG*/

/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */


/**
 * This implements the default "storage://" protocol J2ME
 *
 * @author  Nik Shaylor
 * @version 2.0 2/21/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <async.h>
#include <storage.h>
#include <errno.h>

#ifndef WINDOWS
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef _S_IREAD
#define _S_IREAD 0444
#endif

#ifndef _S_IWRITE
#define _S_IWRITE 0220
#endif

#ifndef O_APPEND
#define O_APPEND 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define Connector_READ        1
#define Connector_WRITE       2
#define Connector_READ_WRITE  (Connector_READ|Connector_WRITE)


/*=========================================================================
 * Protocol Methods
 *=======================================================================*/


#define setHandle(i)        real_setHandle(instance, i)
#define getHandle()         real_getHandle(instance)
#define getDirectory()      real_getDirectory(instance)
#define formName(i)         real_formName(instance, i)
#define openFile(name, wrt) real_openFile(instance, name, wrt)

#define return_exception(s) {*exception=(s); return_normal();}

#define return_string(s)    return instantiateString(s, strlen(s))
#define return_normal()     return 0;
#define return_value(x)     return (x);


/*
 * real_setHandle
 */

static void real_setHandle(INSTANCE instance, int i) {
    FIELD field;
    if(instance == 0) {
            fatalError("real_setHandle instance zero");
    }
    field = lookupField(instance->ofClass, getNameAndTypeKey("handle", "I"));
    if (field != 0) {
        /*  Get slot index and write value there */
        int index = field->u.offset;
        instance->data[index].cell = i;
    } else {
        char buf[100];
        sprintf(buf, "Could not find field handle");
        fatalError(buf);
    }
}


/*
 * real_getHandle
 */
static int real_getHandle(INSTANCE instance) {
    static int index = -1;
    if(instance == 0) {
            fatalError("real_getHandle instance zero");
    }
    if(index == -1) {
        FIELD field = lookupField(instance->ofClass, getNameAndTypeKey("handle", "I"));
        if (field != 0) {
            /*  Get slot index and write value there */
            index = field->u.offset;
            } else {
            fatalError("Could not find field handle");
        }
    }
    return instance->data[index].cell;
}


/*
 * real_getDirectory
 */
static STRING_INSTANCE real_getDirectory(INSTANCE instance) {
    static int index = -1;
    if(index == -1) {
        FIELD field = lookupField(instance->ofClass, getNameAndTypeKey("_directory_", "Ljava/lang/String;"));
        if (field != 0) {
            /*  Get slot index and write value there */
            index = field->u.offset;
        } else {
            fatalError("Could not find field _directory_");
            return_normal();
        }
    }
    return (STRING_INSTANCE)(instance->data[index].cellp);
}


/*
 * mungPath
 */
static char *mungName(char *src) {
    char *start = src;
    char *d;
    char *s;
    int last = 0;
    int ch;

    s = start;
    d = start;

#if WINDOWS
    if(start[0] == '.' && (start[1] == '/' || start[1] == '\\')) {
        s += 2;
    }

    {
        int len = strlen(s);
        if(len > 0 && s[len-1] == '/' || s[len-1] == '\\') {
             s[len-1] = 0;
             --len;
        }
    }
#endif

   /*
    * Remove all double / characters
    */

    while((ch = *s++) != '\0') {
        if(ch != '/' || last != '/') {
            *d++ = ch;
        }
        last = ch;
    }

    *d++ = '\0';

#if WINDOWS
    /*
     * Turn all '/' chars into '\'
     */
    s = start;
    while((ch = *s++) != '\0') {
        if(ch == '/') {
            s[-1] = '\\';
        }
    }
    if(start[0] == 0) {
        start[0] = '.';
        start[1] = 0;
    }
#endif

    return start;
}


/*
 * formName
 */
static char *real_formName(INSTANCE instance, char *name) {
    char *str;
    char buf[PATH_BUFFER_SIZE];
    STRING_INSTANCE dir = getDirectory();
    if(instance == 0) {
            fatalError("real_formName instance zero");
    }
    if(name == 0) {
            fatalError("real_formName name zero");
    }
    if(dir == 0) {
            fatalError("real_formName dir zero");
    }
    str = getStringContentsSafely(dir, buf, PATH_BUFFER_SIZE);
    strcat(str, name);
    if(strlen(buf) > 254) {
        fatalError("Buffer overflow in formName");
    }
    strcpy(name, buf);
    return mungName(name);
}


/*
 * Open a file but create if missing
 */
static int real_openFile(INSTANCE instance, char *name, bool_t writing) {
    if(writing) {
        return open(name, O_CREAT | O_RDWR   | O_BINARY, _S_IREAD | _S_IWRITE);
    } else {
        return open(name, O_RDONLY | O_BINARY);
    }
}



/*
 * prim_realOpen
 */
int prim_realOpen(INSTANCE instance, char **exception, char *name, bool_t writing) {

    int fd = openFile(formName(name), writing);

#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_realOpen '%s' fd = %d errno = %d\n", name, fd, errno);
#endif

    if(fd < 0) {
        return_exception("javax/microedition/io/ConnectionNotFoundException");
    }

    setHandle(fd);
    return_normal();
}


/*
 * prim_close
 */
int prim_close(INSTANCE instance, char **exception) {
    int fd = getHandle();
    if(fd >= 0) {
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_close fd = %d\n", fd);
#endif
        close(fd);
        setHandle(-1);
    }
    return_normal();
}


/*
 * prim_findFirstFile
 */
STRING_INSTANCE prim_findFirstItem(INSTANCE instance, char **exception) {
    fatalError("prim_findFirstItem Not written");
    return_normal();
}


/*
 * prim_findFile
 */
STRING_INSTANCE prim_findItem(INSTANCE instance, char **exception, char *name) {
    int len = strlen(name);
    struct stat s;
    int res;

    name = formName(name);
    res = (stat(name, &s) == 0);

#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_findItem '%s' res = %d\n", name, res);
#endif
    len = strlen(name) - len;
    if(!res) {
        return_value(0);
    } else {
        return_string(name+len);
    }
}


/*
 * prim_findFileAfter
 */
STRING_INSTANCE prim_findItemAfter(INSTANCE instance, char **exception, char *name) {
    fatalError("prim_findItemAfter Not written");
    return_normal();
}


/*
 * prim_availableSpace
 */
long64 prim_availableSpace(INSTANCE instance, char **exception) {
    long64 result;
    SET_LONG_FROM_HALVES(&result, 0x7FFFFFFF, 0xFFFFFFFF);
    return result;
}


/*
 * prim_countItems
 */
int prim_countItems(INSTANCE instance, char **exception) {
    fatalError("prim_countItems Not written");
    return_normal();
}


/*
 * prim_create
 */
STRING_INSTANCE prim_create(INSTANCE instance, char **exception) {
    struct stat s;
    int res;
    int len;
    int i;
    for(i = 1 ; i < 32767 ; i++) {
        int fd;
        char *name;
        char buf[PATH_BUFFER_SIZE];
        sprintf(buf, "%d", i);
        len = strlen(buf);
        name = formName(buf);
        res = (stat(name, &s) == 0);
        if(!res) {
            fd = openFile(name, TRUE);
            if(fd < 0) {
                break;
            }
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_create '%s' fd = %d\n", name, fd);
#endif
            close(fd);
            return_string(name+strlen(name)-len);
        }
    }
    return_exception("java/io/IOException");
}


/*
 * prim_createFile
 */
int prim_createFile(INSTANCE instance, char **exception, char *name) {
    int fd = openFile(formName(name), TRUE);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_createFile '%s' fd = %d\n", name, fd);
#endif
    if(fd < 0) {
        return_exception("java/io/IOException");
    }
    close(fd);
    return_normal();
}


/*
 * prim_createDirectory
 */
int prim_createDirectory(INSTANCE instance, char **exception, char *name) {
    int fd = getHandle();
#ifdef WINDOWS
    int res = mkdir(formName(name));
#else
    int res = mkdir(formName(name), 0777);
#endif
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_createDirectory '%s' res = %d\n", name, res);
#else
    (void)res;
#endif
    if(fd < 0) {
        return_exception("java/io/IOException");
    }
    return_normal();
}


/*
 * prim_deleteItem
 */
bool_t prim_deleteItem(INSTANCE instance, char **exception, char *name) {
    int fd = getHandle();
    int res = (remove(formName(name)) == 0);
    (void)fd;
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_deleteItem '%s' res = %d\n", name, res);
#endif
    return_value(res);
}


/*
 * prim_renameItem
 */
int prim_renameItem(INSTANCE instance, char **exception, char *name, char *name2) {
    fatalError("prim_renameItem Not written");
    return_normal();
    /*return_value(rename(getHandle(), name, name2));*/
}


/*
 * prim_lengthOf
 */
long64 prim_lengthOf(INSTANCE instance, char **exception, char *name) {
    long cur, end, set;
    int fd = getHandle();
    bool_t   cls = FALSE;
    if(fd > 0) {
        fd = openFile(formName(name), FALSE);
        cls = TRUE;
    }

    cur = lseek(fd, 0L,  SEEK_CUR);
    end = lseek(fd, 0L,  SEEK_END);
    set = lseek(fd, cur, SEEK_SET);

#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_lengthOf res = %d\n", set|end|cur);
#endif

    if(cls) {
        close(fd);
    }

    if(cur == -1 || end == -1 || set == -1) {
        return_exception("java/io/IOException");
    }

    return_value(end);
}


/*
 * prim_timeOf
 */
long64 prim_timeOf(INSTANCE instance, char **exception, char *name) {
    int fd = getHandle();
    if(fd >= 0) {
    } else {
    }
    fatalError("prim_timeOf Not written");
    return_normal();
}


/*
 * prim_exists
 */
bool_t prim_exists(INSTANCE instance, char **exception, char *name) {
    struct stat s;
    int res = (stat(formName(name), &s) == 0);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_exists '%s' res = %d\n", name, res);
#endif
    return (res);
}


/*
 * prim_isDirectory
 */
bool_t prim_isDirectory(INSTANCE instance, char **exception, char *name) {
    struct stat s;
    int res = (stat(formName(name), &s) == 0);
    if(res) {
        res = (S_ISDIR(s.st_mode) ? TRUE : FALSE);
    } else {
        res = FALSE;
    }
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_isDirectory '%s' res = %d\n", name, res);
#endif
    return_value(res);

}


/*
 * prim_canRead
 */
bool_t prim_canRead(INSTANCE instance, char **exception, char *name) {
    fatalError("prim_canRead Not written");
    return_normal();
}


/*
 * prim_canWrite
 */
bool_t prim_canWrite(INSTANCE instance, char **exception, char *name) {
    fatalError("prim_canWrite Not written");
    return_normal();
}


/*
 * prim_setReadable
 */
bool_t prim_setReadable(INSTANCE instance, char **exception, char *name, bool_t tf) {
    fatalError("Not written");
    return_normal();
}


/*
 * prim_setWritable
 */
bool_t prim_setWritable(INSTANCE instance, char **exception, char *name, bool_t tf) {
    return FALSE;
}

/*
 * prim_seek
 */
int prim_seek(INSTANCE instance, char **exception, long64 pos) {
    int fd = getHandle();
    long set = lseek(fd, (long)pos, SEEK_SET);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_seek res = %d\n", set);
#endif
    if(set == -1) {
        return_exception("java/io/IOException");
    }
    return_normal();
}


/*
 * prim_getPosition
 */
long64 prim_getPosition(INSTANCE instance, char **exception) {
    int fd = getHandle();
    long set = lseek(fd, 0, SEEK_CUR);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_getPosition res = %d\n", set);
#endif
    if(set == -1) {
        return_exception("java/io/IOException");
    }
    return_value(set);
}


/*
 * prim_read
 */
int prim_read(INSTANCE instance, char **exception) {
    int fd = getHandle();
    unsigned char ch;
    int length = read(fd, &ch, 1);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_read res = %d ch = %02x\n", length, ch);
#endif
    return_value((length == 0) ? -1 : ch);
}


/*
 * prim_readBytes
 */
int prim_readBytes(INSTANCE instance, char **exception, char *b, int offset, int length) {
    int fd = getHandle();
    length = read(fd, b + offset, length);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_readBytes res = %d\n", length);
#endif
    return_value((length == 0) ? -1 : length);
}


/*
 * prim_writeBytes
 */
int prim_write(INSTANCE instance, char **exception, int b) {
    int fd = getHandle();
    char ch = (char)b;
    int res = write(fd, &ch, 1);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_write res = %d\n", res);
#endif
    if(res != 1) {
        return_exception("java/io/IOException");
    }
    return_normal();
}


/*
 * prim_write
 */
int prim_writeBytes(INSTANCE instance, char **exception, char *b, int offset, int length) {
    int fd = getHandle();
    int res = write(fd, b + offset, length);
#ifdef STORAGEDEBUG
    fprintf(stderr, "prim_writeBytes res = %d\n", length);
#endif
    if(res != length) {
        return_exception("java/io/IOException");
    }
    return_normal();
}






#ifndef PILOT

static char *prefix = "Database";
static char name[100];

int Fake_DmCreateDatabase(int cardNumber, char *rawName, int creatorID, int typeID, int resourceDB) {
    return TRUE;
}

long Fake_DmOpenDatabaseByTypeCreator(int typeID, int creatorID, int mode) {
    return 1;
}

int Fake_DmCloseDatabase(long dbRef) {
    return TRUE;
}

int Fake_DmNumRecords(long dbRef) {
   return -1;
}

int Fake_DmGetRecord(long dbRef, int recordNumber, char *buffer) {
    int length;
    int fd;
    sprintf(name, "%s_%d", prefix, recordNumber);

    fd = real_openFile(0, name, FALSE);
    if(fd < 0) {
        return -1;
    }
    length = read(fd, buffer, 65536);
    close(fd);
    return length;
}

int Fake_DmWrite(long dbRef, int recordNumber, char *data, int length) {
    int fd;
    sprintf(name, "%s_%d", prefix, recordNumber);
    fd = real_openFile(0, name, TRUE);
    if(fd < 0) {
        return FALSE;
    }
    write(fd, data, length);
    close(fd);
    return TRUE;
}

int Fake_DmRemoveRecord(long dbRef, int recordNumber) {
    sprintf(name, "%s_%d", prefix, recordNumber);
    return (remove(name) == 0);
}
#endif
