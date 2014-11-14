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

/**
 * This implements the network protocols for Windows
 *
 * @author  Nik Shaylor
 * @version 1.0 1/17/2000
 */


/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <async.h>

#ifndef NETDEBUG
#define NETDEBUG FALSE
#endif


/*=========================================================================
 * Protocol Methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      setInstanceField()
 * TYPE:          public instance-level operation
 * OVERVIEW:      Get the contents of a field in an object
 * INTERFACE:
 *   parameters:  instance, name, value
 *   returns:     <nothing>
 *=======================================================================*/


static void setInstanceField(INSTANCE instance, char *name, char *type, long value)
{
    FIELD field = lookupField(instance->ofClass, getNameAndTypeKey(name, type));

    if (field != 0) {
        /*  Get slot index and write value there */
        int index = field->u.offset;
        instance->data[index].cell = value;
    } else {
        char buf[100];
        sprintf(buf, "Could not find field %s", name);
        fatalError(buf);
    }
}

/*=========================================================================
 * FUNCTION:      getSocketHandle()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Get the contents of the handle field
 * INTERFACE:
 *   parameters:  instance
 *   returns:     value
 *=======================================================================*/

static int getSocketHandle(INSTANCE instance)
{
    static int index = -1;
#ifdef SAFER
    index = -1;
#endif
    if(index == -1) {
        FIELD field = lookupField(instance->ofClass, getNameAndTypeKey("handle", "I"));
        if (field != 0) {
            /*  Get slot index and write value there */
            index = field->u.offset;
        } else {
            fatalError("Could not find field handle");
            return 0;
        }
    }

    if(NETDEBUG) {
        fprintf(stderr, "getSocketHandle index=%ld handle=%ld\n", 
		(long)index, (long)instance->data[index].cell);
    }

    return instance->data[index].cell;
}



/*=========================================================================
 * FUNCTION:      getPortNumber()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Get the port number from the URL
 * INTERFACE:
 *   parameters:  string
 *   returns:     value
 *=======================================================================*/

static int getPortNumber(char *string)
{
    char *p = string;
    while(TRUE) {
        if(*p == '\0') {
            return -1;
        }
        if(*p++ == ':') {
            break;              /* must have a ':' */
        }
    }

    if(*p < '0' || *p > '9') {
        return -1;              /* must have at least one digit' */
    }

    string = p;

    while(*p != '\0') {
        if(*p < '0' || *p > '9') {
            return -1;
        }
        if(*p++ == ';') {
            break;              /* port numbers may be terminated by EOL or ';' */
        }
    }

    if(NETDEBUG) {
        fprintf(stderr, "getPortNumber string='%s' port=%d\n", string, atoi(string));
    }

    return atoi(string);
}


/*=========================================================================
 * FUNCTION:      getHostName()
 * TYPE:          private instance-level operation
 * OVERVIEW:      Get the name from the URL
 * INTERFACE:
 *   parameters:  string
 *   returns:     string
 *   NOTE: *** The input string is modified by this function ***
 *=======================================================================*/

static char *getHostName(char *string)
{
    char *p = string;

    if(*p++ != '/' || *p++ != '/') {
        return NULL;
    }

    while(TRUE) {
        if(*p == '\0' || *p == ':' || *p == ';') {
            break;
        }
        ++p;
    }

    if(NETDEBUG) {
        fprintf(stderr, "getHostName string='%s'", string);
    }

   *p = '\0';         /* terminate the string there */

    if(NETDEBUG) {
        fprintf(stderr, " name='%s'\n", string+2);
    }

   return string + 2; /* skip past the "//" */
}


/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, mode, append, port
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_open0)
{
    long            port      = ASYNC_POP_STACK();
    long            append    = ASYNC_POP_STACK();
    long            mode      = ASYNC_POP_STACK();
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;
    int fd;

    (void)mode;
    (void)append;

    if (NETDEBUG) {
        fprintf(stderr, "datagram::open0 p=%ld\n", port);
    }

    fd = prim_com_sun_cldc_io_j2me_datagram_Protocol_open0(port, &exception);

    if(NETDEBUG) {
        fprintf(stderr, "datagram::open0 p=%ld fd=%ld exception='%s' ne=%ld\n",
		port, (long)fd, (exception == NULL) ? "null" : exception, 
		(long)netError());
    }

    if(exception == NULL) {
       prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(fd); /* For non-blocking systems only */
       goto done;
    }

    ASYNC_EXCEPTION(exception);
    goto fail;
fail:
    fd = -1;
done:

    setInstanceField(instance, "handle", "I", fd);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      getAddress()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram endpoint address
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     string
 *=======================================================================*/

/***
ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_getAddress)
{
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    ASYNC_PUSH_STACK_AS_TYPE(STRING_INSTANCE, string);
    ASYNC_RESUME_THREAD();
}
***/

/*=========================================================================
 * FUNCTION:      getMaximumLength0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram max length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength0)
{
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    int len = prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength();

    (void)instance;

    if (NETDEBUG) {
        fprintf(stderr, "datagram::getMaximumLength len=%d\n", len);
    }

    ASYNC_PUSH_STACK(len);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      getNominalLength0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram nominal length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength0)
{
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    int len = prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength();

    (void)instance;
    if (NETDEBUG) {
        fprintf(stderr, "datagram::getNominalLength len=%d\n", len);
    }

    ASYNC_PUSH_STACK(len);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      getIpNumber()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, string
 *   returns:     int
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber)
{
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char            buf[100];
    char           *name      = getStringContentsSafely(string, (char *)&buf, 100);

    int ipn = prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(name);

    (void)instance;
    if (NETDEBUG) {
        fprintf(stderr, "datagram::getIpNumber name='%s' ipn=%08lx\n", 
		name, (long)ipn);
    }

    ASYNC_PUSH_STACK(ipn);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      send0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Send a datagram
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, iaddr, port, buffer, offset, length
 *   returns:     length
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_send0)
{
    long            length   = ASYNC_POP_STACK();
    long            offset   = ASYNC_POP_STACK();
    BYTEARRAY       buffer   = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    long            port     = ASYNC_POP_STACK();
    long            ipnumber = ASYNC_POP_STACK();
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int res;

    if(NETDEBUG) {
        fprintf(stderr, "datagram::send0 b=%08lx o=%ld l=%ld p=%ld ip=%08lx fd=%ld\n",
		(long)buffer, offset, length, port, ipnumber, fd);
    }

    res = prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(fd, ipnumber, port, (buffer->bdata)+offset, length);

    if(NETDEBUG) {
        fprintf(stderr, "datagram::send0 res=%d ne=%d\n", res, netError());
    }

    if(res < 0) {
        if(res == -3) {
            ASYNC_EXCEPTION("java/io/InterruptedIOException");
        } else {
            ASYNC_EXCEPTION("java/io/IOException");
        }
    }

    ASYNC_PUSH_STACK(res);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      receive0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Receive a datagram
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, buffer, offset, length
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_receive0)
{
    long            length   = ASYNC_POP_STACK();
    long            offset   = ASYNC_POP_STACK();
    BYTEARRAY       buffer   = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    int ipnumber, port, res;
    ulong64 lres;
    long fd = getSocketHandle(instance);

    if(NETDEBUG) {
        fprintf(stderr, "datagram::receive0 b=%08lx o=%ld l=%ld fd=%ld\n",
		(long)buffer, offset, length, fd);
    }

    res = prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(fd, &ipnumber, &port, (buffer->bdata)+offset, length);

    if(NETDEBUG) {
        fprintf(stderr, "datagram::receive0 res=%ld ip=%08lx p=%ld ne=%ld\n", 
		(long)res, (long)ipnumber, (long)port, (long)netError());
    }

    if(res < 0) {
        if(res == -3) {
            ASYNC_EXCEPTION("java/io/InterruptedIOException");
        } else {
            ASYNC_EXCEPTION("java/io/IOException");
        }
    }

    if(res > 0xFFFF) {
        fatalError("Cannot receive datagrams > 65535");
    }

    lres = (((ulong64)ipnumber) << 32) + ((port&0xFFFF) << 16) + (res&0xFFFF);
    ASYNCPUSHLONG(lres);

    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_datagram_Protocol_close0)
{
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int  res = 0;

    if(fd != -1L) {
        res = prim_com_sun_cldc_io_j2me_datagram_Protocol_close(fd);
        setInstanceField(instance, "handle", "I", -1);
    }

    if (NETDEBUG) {
        fprintf(stderr, "datagram::close res=%ld fd=%ld ne=%ld\n", 
		(long)res, (long)fd, (long)netError());
    }

    if(res < 0) {
        ASYNC_EXCEPTION("java/io/IOException");
    }

    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, name, mode, append
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_socket_Protocol_open0)
{
    long            append    = ASYNC_POP_STACK();
    long            mode      = ASYNC_POP_STACK();
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;
    int             fd        = -1;
    char            buf[100];
    char           *name      = getStringContentsSafely(string, (char *)&buf, 100);
    char           *name2;
    int             port;

   /*
    * The name string has the following format: "//<host>:<port>"
    */

    (void)mode;
    (void)append;

    port = getPortNumber(name);    /* Must be called before getHostName() */
    if(port < 0 || port > 0xFFFF) {
        if(NETDEBUG) {
            fprintf(stderr, "socket::open0 name='%s' port=%d (BAD)\n",name, port);
        }
        goto illarg;
    }

    name2 = getHostName(name);      /* Must be the last parsing function */
    if(name2 == NULL) {
        if(NETDEBUG) {
            fprintf(stderr, "socket::open0 name='%s' name2=null (BAD)\n",name);
        }
        goto illarg;
    }

   /*
    * We allow "socket://:nnnn" to mean "serversocket://:nnnn" but we throw an
    * InterruptedException to tell the calling Java code about this
    */

    if(*name2 == '\0') {
        ASYNC_EXCEPTION("java/lang/InterruptedException");
        goto fail;
    }

    if(NETDEBUG) {
        fprintf(stderr, "socket::open0 name='%s' name2='%s' port=%d\n",name, name2, port);
    }

    fd = prim_com_sun_cldc_io_j2me_socket_Protocol_open0(name2, port, &exception);

    if(NETDEBUG) {
        fprintf(stderr, "socket::open0 name='%s' name2='%s' port=%d fd = %d exception = '%s' ne=%d\n",
                        name, name2, port, fd, (exception == NULL) ? "null" : exception, netError());
    }

    if(exception == NULL) {
       prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(fd); /* For non-blocking systems only */
       goto done;
    }

    ASYNC_EXCEPTION(exception);
    goto fail;

illarg:
    ASYNC_EXCEPTION("java/lang/IllegalArgumentException");
fail:
    fd = -1;
done:
    setInstanceField(instance, "handle", "I", fd);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      read0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Read from a TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, buffer, offset, length
 *   returns:     number of chars read,
 *                or a single character if buffer was NULL
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_socket_Protocol_read0)
{
    long       length = ASYNC_POP_STACK();
    long       offset = ASYNC_POP_STACK();
    BYTEARRAY  buffer = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int  res;

    if (NETDEBUG) {
        fprintf(stderr, "socket::read0 b=%08lx o=%ld l=%ld fd=%ld\n", 
		(long)buffer, offset, length, fd);
    }

    if(buffer == NULL) {
        char buf[1];
        res = prim_com_sun_cldc_io_j2me_socket_Protocol_read0(fd, buf, 1);

        if(NETDEBUG) {
            fprintf(stderr, "socket::read0 res=%d ne=%d\n", res, netError());
        }

        if(res < 0 && res != -2) {
            ASYNC_EXCEPTION("java/io/IOException");
        }

        if(res == -3) {
            ASYNC_EXCEPTION("java/io/InterruptedIOException");
        }

        if(res == 1) {
            res = buf[0] & 0xFF;
        } else if(res == 0) {
            res = -1;
        }
    } else {
        char *p = buffer->bdata;

        if ((offset < 0) || (offset > (long)buffer->length) || (length < 0) ||
           ((offset + length) > (long)buffer->length) || ((offset + length) < 0)) {
            ASYNC_EXCEPTION("java/lang/IndexOutOfBoundsException");
            res = 0;
            goto done;
        }

        res = prim_com_sun_cldc_io_j2me_socket_Protocol_read0(fd, p+offset, length);

        if(NETDEBUG) {
            fprintf(stderr, "socket::read0 res=%d ne=%d\n", res, netError());
        }

        if(res == -2) {
            res = 0;
            goto done;
        }

        if(res < 0) {
            if(res == -3) {
                ASYNC_EXCEPTION("java/io/InterruptedIOException");
            } else {
                ASYNC_EXCEPTION("java/io/IOException");
            }
        }

        if(res == 0) {
            res = -1;
        }
    }

done:
    ASYNC_PUSH_STACK(res);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      write0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Write to a TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, buffer, offset, length
 *   returns:     the length written
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_socket_Protocol_write0)
{
    long       length = ASYNC_POP_STACK();
    long       offset = ASYNC_POP_STACK();
    BYTEARRAY  buffer = ASYNC_POP_STACK_AS_TYPE(BYTEARRAY);
    INSTANCE instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int  res;

    if (NETDEBUG) {
        fprintf(stderr, "socket::write0 b=%08lx o=%ld l=%ld fd=%ld\n", 
		(long)buffer, (long)offset, (long)length, fd);
    }

    if(buffer == NULL) {
        char buf[1];
        buf[0] = (char)offset;
        res = prim_com_sun_cldc_io_j2me_socket_Protocol_write0(fd, buf, 1);
        length = 1;
    } else {
        char *p = buffer->bdata;

        if ((offset < 0) || (offset > (long)buffer->length) || (length < 0) ||
           ((offset + length) > (long)buffer->length) || ((offset + length) < 0)) {
            ASYNC_EXCEPTION("java/lang/IndexOutOfBoundsException");
            res = 0;
            goto done;
        }

        res = prim_com_sun_cldc_io_j2me_socket_Protocol_write0(fd, p+offset, length);
    }

    if (NETDEBUG) {
        fprintf(stderr, "socket::write0 res=%ld l=%ld ne=%ld\n", 
		(long)res, (long)length, (long)netError());
    }

    if(res < 0) {
        if(res == -3) {
            ASYNC_EXCEPTION("java/io/InterruptedIOException");
        } else {
            ASYNC_EXCEPTION("java/io/IOException");
        }
    }

done:
    ASYNC_PUSH_STACK(res);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      available0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Return available length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     length
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_socket_Protocol_available0)
{
    INSTANCE        instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    int             len      = 0;

    (void)instance;
    if(NETDEBUG) {
        fprintf(stderr, "socket::available0 len=%d\n", len);
    }

    ASYNC_PUSH_STACK(len);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_socket_Protocol_close0)
{
    INSTANCE instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int res = 0;

    if(fd != -1L) {
        res = prim_com_sun_cldc_io_j2me_socket_Protocol_close0(fd);
        setInstanceField(instance, "handle", "I", -1);
    }

    if (NETDEBUG) {
        fprintf(stderr, "socket::close res=%ld fd=%ld ne=%ld\n", 
		(long)res, fd, (long)netError());
    }

    if(res < 0) {
        ASYNC_EXCEPTION("java/io/IOException");
    }

    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      open()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a listening TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this, name, mode, append
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_serversocket_Protocol_open)
{
    long            append    = ASYNC_POP_STACK();
    long            mode      = ASYNC_POP_STACK();
    STRING_INSTANCE string    = ASYNC_POP_STACK_AS_TYPE(STRING_INSTANCE);
    INSTANCE        instance  = ASYNC_POP_STACK_AS_TYPE(INSTANCE);
    char           *exception = NULL;
    int             fd        = -1;
    char            buf[100];
    char           *name      = getStringContentsSafely(string, (char *)&buf, 100);
    char           *name2;
    int             port;

    (void)mode;
    (void)append;

    if(strlen(name) >= 100) {
        fatalError("Buffer overflow in serversocket::open");
    }
    strcpy((char *)&buf, name);
    name = (char *)&buf;

   /*
    * The name string has the following format: "//<host>:<port>"
    */

    port = getPortNumber(name);    /* Must be called before getHostName() */
    if(port < 0 || port > 0xFFFF) {
        if(NETDEBUG) {
            fprintf(stderr, "serversocket::open name='%s' port=%d (BAD)\n",name, port);
        }
        goto illarg;
    }

    name2 = getHostName(name);      /* Must be the last parsing function */
    if(name2 == NULL || *name2 != '\0') {
        if(NETDEBUG) {
            fprintf(stderr, "serversocket::open name='%s' name2='%s' (BAD)\n",
                       name, (name2==NULL) ? "null" : name2);
        }
        goto illarg;                    /* must start "serversocket://:" */
    }

    if(NETDEBUG) {
        fprintf(stderr, "serversocket::open name='%s' name2='%s' port=%d\n", name, name2, port);
    }

    fd = prim_com_sun_cldc_io_j2me_serversocket_Protocol_open0(port, &exception);

    if(NETDEBUG) {
        fprintf(stderr, "serversocket::open name='%s' name2='%s' port=%d fd = %d exception = '%s' ne=%d\n",
                        name, name2, port, fd, (exception == NULL) ? "null" : exception, netError());
    }

    if(exception == NULL) {
        prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(fd); /* For non-blocking systems only */
        goto done;
    }

    ASYNC_EXCEPTION(exception);
    goto fail;

illarg:
    ASYNC_EXCEPTION("java/lang/IllegalArgumentException");
fail:
    fd = -1;
done:
    setInstanceField(instance, "handle", "I", fd);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      accept()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Accept and open a connection
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     socket handle
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_serversocket_Protocol_accept)
{
    INSTANCE instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    long res;

    if (NETDEBUG) {
        fprintf(stderr, "serversocket::accept fd=%ld\n", fd);
    }

    res = prim_com_sun_cldc_io_j2me_serversocket_Protocol_accept(fd);

    if (NETDEBUG) {
        fprintf(stderr, "serversocket::accept res=%ld fd=%ld ne=%ld\n", 
		res, fd, (long)netError());
    }

    if(res < 0 && res != -2) {
        ASYNC_EXCEPTION("java/io/IOException");
    }

    ASYNC_PUSH_STACK (res);
    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      close()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a listening TCP socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *=======================================================================*/

ASYNCFUNCTION(Java_com_sun_cldc_io_j2me_serversocket_Protocol_close)
{
    INSTANCE instance = ASYNC_POP_STACK_AS_TYPE(INSTANCE);

    long fd = getSocketHandle(instance);
    int res = 0;

    if(fd != -1L) {
        res = prim_com_sun_cldc_io_j2me_serversocket_Protocol_close(fd);
        setInstanceField(instance, "handle", "I", -1);
    }

    if(NETDEBUG) {
        fprintf(stderr, "serversocket::close res=%ld fd=%ld ne=%ld\n", 
		(long)res, fd, (long)netError());
    }

    if(res < 0) {
        ASYNC_EXCEPTION("java/io/IOException");
    }

    ASYNC_RESUME_THREAD();
}


/*=========================================================================
 * FUNCTION:      initalizeInternal();
 * CLASS:         com.sun.cldc.io.NetworkConnectionBase()
 * TYPE:          virtual native function
 * OVERVIEW:      Initialize the network
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     none
 *
 * This is a static initializer in NetworkConnectionBase, the parent of
 * all ..../Protocol classes that access the network.
 *=======================================================================*/


void 
Java_com_sun_cldc_io_NetworkConnectionBase_initializeInternal()
{
     networkInit();
}
