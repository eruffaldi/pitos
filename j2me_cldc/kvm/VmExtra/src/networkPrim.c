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
 * Note the use of the functions startLongIOActivity() and
 * sndLongIOActivity(). These are used to indicate to the VM
 * that a prelonged period of I/O activity is occuring on the
 * asynchronous thread. This information is used by the garbage
 * collector to know if it is safe to start a collection. The
 * critical thing is that non of the object store be altered between
 * these start and end functions. As this file is not compiled with
 * any of the object storage data structure .h file we know that this
 * is the case.
 *
 * @author  Nik Shaylor
 * @version 1.0 1/17/2000
 */

/*=========================================================================
 * Include files
 *=======================================================================*/
#include <machine_md.h>
#include <main.h>
#include <async.h>

/* -------------------------- PILOT ------------------------ */

#ifdef PILOT
#undef atoi     /* Just because we have to */
#include <networkPrim.h>
#include <sys_types.h>
#include <sys_time.h>
#include <netinet_in.h>
#include <arpa_inet.h>
/*
#include <sys_socketver.h>
*/
#include <unix_netdb.h>
#include <sys_uio.h>
#include <sys_errno.h>
#include <netinet_tcp.h>
#include <unix_fcntl.h>

#include <unix_stdlib.h>

int netError(void);

#define closesocket close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define HENT_BUF_SIZE 1024
#define NONBLOCKING 1
#define O_NONBLOCK    _FNONBLOCK

#include <Pilot_network.h>

#endif /* PILOT */

/* -------------------------- WINDOWS ------------------------ */

#ifdef WINDOWS
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#define NOMSG
#include <windows.h>
#include <process.h>
#include <winsock.h>
WSADATA wsaData;
int netError(void);
#endif

/* -------------------------- UNIX ------------------------ */

#if UNIX
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#define closesocket close
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define HENT_BUF_SIZE 1024
#define NONBLOCKING 1
int netError(void);
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NETDEBUG
#define NETDEBUG FALSE
#endif

/*=========================================================================
 * Protocol Methods
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_open0(int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1;
    int res, i;
int gsnlen = sizeof( addr );
    startLongIOActivity();

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == INVALID_SOCKET) {
        goto ioe;
    }

#ifndef PILOT
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof(int));
#endif

    i = port;

#if defined(UNIX)
   /*
    * Here is a total mystery to me. If I give solaris a port number of zero
    * then it allocated one somewhere above 50000. The problem is that when I
    * do a recvfrom() on a socket that is bound to a port > 32767 it never sees
    * any data.
    *
    * TEMP SOLUTION
    *
    * Start at port 6000 and just go upwards until a free port is found
    */
    if (i == 0) {
        i = 6000;
    }
    res = EADDRINUSE;
    do {
#endif
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons((short)i++);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        res = bind(fd, (struct sockaddr*)&addr, sizeof( addr ));
#if defined(UNIX)
    } while (res == EADDRINUSE && port == 0 && i < 32767);
#endif

getsockname(fd, (struct sockaddr*)&addr, &gsnlen);

    if(res == SOCKET_ERROR) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

/*
fprintf(stderr, "datagram::bound fd=%d p=%u:%u\n", fd, addr.sin_port, addr.sin_port);
*/
    goto done;

ioe:
    *exception = "java/io/IOException";
    goto fail;

fail:
    if(fd >= 0) {
        closesocket(fd);
    }
    fd = -1;
    goto done;

done:

    endLongIOActivity();

    return fd;
}


/*=========================================================================
 * FUNCTION:      getMaximumLength()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram max length
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength(void)
{
#ifdef WINDOWS
    return wsaData.iMaxUdpDg;
#else
    return 0;
#endif
}


/*=========================================================================
 * FUNCTION:      getNominalLength()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Get the datagram nominal length
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength(void)
{
#ifdef WINDOWS
    return wsaData.iMaxUdpDg;
#else
    return 0;
#endif
}


/*=========================================================================
 * FUNCTION:      getIpNumber()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(char *host)
{
    struct sockaddr_in sin;
    struct sockaddr_in* sa = &sin;
    struct hostent *hp;
    struct in_addr addr;

#ifdef UNIX
    struct hostent res;
    char buf[HENT_BUF_SIZE];
    int h_error=0;
#endif

    startLongIOActivity();

#ifdef UNIX
#ifdef __GLIBC__
    gethostbyname_r(host, &res, buf, sizeof(buf), &hp, &h_error);
#else
    hp = gethostbyname_r(host, &res, buf, sizeof(buf), &h_error);
#endif
#else
    hp = gethostbyname(host);
#endif

    if( (hp = gethostbyname(host)) == NULL ) {
        if(NETDEBUG) {
/*----            fprintf(stderr, "getIpNumber host='%s' res=-1\n", host); */
        }
        return -1;
    }

    memset( sa, 0, sizeof(struct sockaddr_in) );
    memcpy(&addr, hp->h_addr_list[0], hp->h_length);

    if(NETDEBUG) {
/*------       fprintf(stderr, "getIpNumber host='%s' res=%08lx\n", host, addr.s_addr); */
    }

    endLongIOActivity();

    return addr.s_addr;
}



/*=========================================================================
 * FUNCTION:      setNonBlocking()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Translate a host name into an ip address
 *=======================================================================*/

void prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(int fd)
{
#ifdef NONBLOCKING
#ifndef PILOT
    int flgs = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flgs | O_NONBLOCK);
#else
        int res;
        Err err;
        int one = 1;

        res = NetLibSocketOptionSet(
                                     AppNetRefnum,
                                     fd,
                                     netSocketOptLevelSocket,
                                     netSocketOptSockNonBlocking,
                                     &one,
                                     sizeof(one),
                                     AppNetTimeout,
                                     &err
                                   );

        if (res < 0) {
            void fatalError(const char *);
            fatalError("Could not set socket non-blocking");
        }
#endif
#endif
}

/*=========================================================================
 * FUNCTION:      send0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Send a datagram
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(int fd, int ipnumber, int port, char *buffer, int length)
{
    struct sockaddr_in addr;
    int res;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((short)port);
    addr.sin_addr.s_addr = ipnumber;

    startLongIOActivity();

    res = sendto(fd, buffer, length, 0, (struct sockaddr*)&addr, sizeof(addr));

    endLongIOActivity();

#ifdef NONBLOCKING
    if(res == -1 && errno == EWOULDBLOCK) {
        res = 0;
    }
#endif


#ifdef EINTR
    if(res == -1 && errno == EINTR) {
        res = -3;
    }
#endif
    return res;
}


/*=========================================================================
 * FUNCTION:      receive0()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Receive a datagram
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(int fd, int *ipnumber, int*port, char *buffer, int length)
{
    struct sockaddr_in addr;
#ifdef PILOT
    Word len = sizeof(struct sockaddr_in);
#else
    int len = sizeof(struct sockaddr_in);
#endif
    int res;

    startLongIOActivity();

    res = recvfrom(fd, buffer, length, 0, (struct sockaddr*)&addr, &len);

    endLongIOActivity();

/*
fprintf(stderr, "-------recvfrom res=%d ", res);
*/

#ifdef NONBLOCKING
    if(res == -1 && errno == EWOULDBLOCK) {
        res = 0;
    }
#endif

#ifdef EINTR
    if(res == -1 && errno == EINTR) {
        res = -3;
    }
#endif

    *ipnumber = (long)addr.sin_addr.s_addr;
    *port     = htons(addr.sin_port);
/*
fprintf(stderr, "ip=%08lx p=%d ne=%d\n", *ipnumber, *port, netError());
*/
    return res;
}


/*=========================================================================
 * FUNCTION:      close()
 * CLASS:         com.sun.cldc.io.j2me.datagram.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a datagram socket
 * INTERFACE (operand stack manipulation):
 *   parameters:  this
 *   returns:     int
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_datagram_Protocol_close(int fd)
{
    int res;

    startLongIOActivity();

    res = closesocket(fd);

    endLongIOActivity();

    return res;
}



/*=========================================================================
 * FUNCTION:      open0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_open0(char *name, int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1, ipn, res;

    startLongIOActivity();

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == INVALID_SOCKET) {
        goto ioe;
    }

#ifndef PILOT
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof(int));
#endif

    ipn = prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(name);
    if(ipn == -1) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = ipn;

#ifdef WINDOWS
    do  {
#endif
        res = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
        if(NETDEBUG) {
            fprintf(stderr, "socket::open0-connect ipn=%08x port=%d res=%d ne=%d\n", ipn, port, res, netError());
        }
#ifdef WINDOWS
    } while ((res < 0) && (errno == WSAEINTR || errno == WSAEALREADY));
    if(res < 0 && errno == WSAEISCONN) {
        res = 0;
    }
#endif

    if(res >= 0) {
        goto done;
    }

ioe:
    *exception = "java/io/IOException";
    goto fail;

fail:
    if(fd >= 0) {
        closesocket(fd);
        fd = -1;
    }
    goto done;

done:

    endLongIOActivity();

    return fd;
}


/*=========================================================================
 * FUNCTION:      read0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Read from a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_read0(int fd, char *p, int len)
{
    int res;

    startLongIOActivity();

    res = recv(fd, p, len, 0);

    endLongIOActivity();

#ifdef NONBLOCKING
    if((res == -1) && (errno == EWOULDBLOCK)) {
        res = -2;
    }
#endif

#ifdef EINTR
    if(res == -1 && errno == EINTR) {
        res = -3;
    }
#endif

    return res;
}


/*=========================================================================
 * FUNCTION:      write0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Write to a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_write0(int fd, char *p, int len)

{
    int res;

    startLongIOActivity();

    res = send(fd, p, len, 0);

    endLongIOActivity();

#ifdef NONBLOCKING
    if((res == -1) && (errno == EWOULDBLOCK)) {
        res = 0;
    }
#endif

#ifdef EINTR
    if(res == -1 && errno == EINTR) {
        res = -3;
    }
#endif

    return res;
}


/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.socket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_socket_Protocol_close0(int fd)
{
    int res;

    startLongIOActivity();

    res = closesocket(fd);

    endLongIOActivity();

    return res;
}


/*=========================================================================
 * FUNCTION:      open()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Open a listening TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_open0(int port, char **exception)
{
    int truebuf  = TRUE;
    struct sockaddr_in addr;
    int fd = -1, res;

    startLongIOActivity();

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        goto ioe;
    }

#ifndef PILOT
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&truebuf, sizeof(int));
#endif

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    res = bind(fd, (struct sockaddr*)&addr, sizeof(addr));

    if(res == SOCKET_ERROR) {
        *exception = "javax/microedition/io/ConnectionNotFoundException";
        goto fail;
    }

    res = listen(fd, 3);
    if(res == SOCKET_ERROR) {
        goto ioe;
    }

    goto done;

ioe:
    *exception = "java/io/IOException";
    goto fail;

fail:
    if(fd >= 0) {
        closesocket(fd);
    }
    fd = -1;
    goto done;

done:

    endLongIOActivity();

    return fd;
}


/*=========================================================================
 * FUNCTION:      accept()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Accept and open a connection
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_accept(int fd)
{
    int res;
	struct sockaddr sa;
	int saLen = sizeof(sa);

    startLongIOActivity();

    res = accept(fd, &sa, &saLen);

    endLongIOActivity();

#ifdef NONBLOCKING
    if((res == -1) && (errno == EWOULDBLOCK)) {
        res = -2;
    }
#endif

    return res;
}


/*=========================================================================
 * FUNCTION:      close0()
 * CLASS:         com.sun.cldc.io.j2me.serversocket.Protocol
 * TYPE:          virtual native function
 * OVERVIEW:      Close a TCP socket
 *=======================================================================*/

int prim_com_sun_cldc_io_j2me_serversocket_Protocol_close(int fd)
{
    return prim_com_sun_cldc_io_j2me_socket_Protocol_close0(fd);
}


/*=========================================================================
 * FUNCTION:      wsaInit()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      Initialize the windows socket system
 * INTERFACE:
 *   parameters:  none
 *   returns:     none
 *=======================================================================*/

void networkInit(void) {
    void fatalError(char *);
#ifdef WINDOWS
    if (WSAStartup(0x0101, &wsaData ) != 0) { 
        fatalError("WSAStartup failure");
    }
#endif
#ifdef PILOT
    extern Boolean useNetLib;
    if (!useNetLib) { 
        fatalError("Palm Application must be built with -networking flag");
    }
#endif

 }


/*=========================================================================
 * FUNCTION:      netError()
 * TYPE:          machine-specific implementation of native function
 * OVERVIEW:      return fatal windows socket error
 * INTERFACE:
 *   parameters:  none
 *   returns:     int
 *=======================================================================*/

int netError(void) {
#ifdef WINDOWS
    return WSAGetLastError();
#else
    int res = errno;
    errno = 0;
    return res;
#endif
}
