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


package com.sun.cldc.io;

import java.io.*;
import javax.microedition.io.*;

/**
 * Base class for Network Connection protocols.
 *
 * @author  Frank Yellin
 * @version 1.1 2/3/2000
 */
abstract public class NetworkConnectionBase extends ConnectionBase { 

    /* This class allows one to initialize the network, if necessary, before
     * any networking code is called 
    */
    private static native void initializeInternal();

    static { 
	initializeInternal();
    }

}
