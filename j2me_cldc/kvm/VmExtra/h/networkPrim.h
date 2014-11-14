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


int  prim_com_sun_cldc_io_j2me_datagram_Protocol_open0(int port, char **exception);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_getMaximumLength(void);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_getNominalLength(void);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_getIpNumber(char *host);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_send0(int fd, int ipnumber, int port, char *buffer, int length);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_receive0(int fd, int *ipnumber, int *port, char *buffer, int length);
int  prim_com_sun_cldc_io_j2me_datagram_Protocol_close(int fd);
void prim_com_sun_cldc_io_j2me_datagram_Protocol_setNonBlocking(int fd);
int  prim_com_sun_cldc_io_j2me_socket_Protocol_open0(char *name, int port, char **exception);
int  prim_com_sun_cldc_io_j2me_socket_Protocol_read0(int fd, char *p, int len);
int  prim_com_sun_cldc_io_j2me_socket_Protocol_write0(int fd, char *p, int len);
int  prim_com_sun_cldc_io_j2me_socket_Protocol_close0(int fd);
int  prim_com_sun_cldc_io_j2me_serversocket_Protocol_open0(int port, char **exception);
int  prim_com_sun_cldc_io_j2me_serversocket_Protocol_accept(int fd);
int  prim_com_sun_cldc_io_j2me_serversocket_Protocol_close(int fd);
void networkInit(void);
int  netError(void);

/*#define NETDEBUG TRUE*/
