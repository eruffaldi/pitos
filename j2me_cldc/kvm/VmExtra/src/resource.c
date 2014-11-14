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
 * This implements the "resource:" protocol
 *
 * @author  Nik Shaylor
 * @version @(#)resource.c	1.5 00/05/02
 */

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Forward declarations
 *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_open(void);
void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_close(void);
void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_read(void);



 /*=========================================================================
  * FUNCTION:      Object open(String) (STATIC)
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Open resource stream
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_open(void) {
    STRING_INSTANCE string = popStackAsType(STRING_INSTANCE);
    char            buf[256];
    char           *name   = getStringContentsSafely(string, buf, 256);
    FILEPOINTER     fp     = openResourcefile(name);

    if (fp == NULL) {
	raiseException("java/io/IOException");
	return;
    }
    pushStackAsType(FILEPOINTER, fp);
}

 /*=========================================================================
  * FUNCTION:      void close(Object) static   [Object is fp]
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the resource
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_close(void) {
    FILEPOINTER fp  = popStackAsType(FILEPOINTER);
    if (fp == NULL) {
        raiseException("java/io/IOException");
	return;
    }
    closeClassfile(fp);
    /* Java code will set the handle to  NULL */
}


 /*=========================================================================
  * FUNCTION:      read()I (VIRTUAL)
  * CLASS:         com.sun.cldc.io.j2me.resource.Protocol
  * TYPE:          virtual native function
  * OVERVIEW:      Read an integer from the resource
  * INTERFACE (operand stack manipulation):
  *   parameters:  this
  *   returns:     the integer value
  *=======================================================================*/

void Java_com_sun_cldc_io_j2me_resource_PrivateInputStream_read(void) {
    FILEPOINTER fp  = popStackAsType(FILEPOINTER);
    int result;
    if (fp == NULL) {
        raiseException("java/io/IOException");
	return;
    }
    result = loadByteNoEOFCheck(fp);
    pushStack(result);
}
