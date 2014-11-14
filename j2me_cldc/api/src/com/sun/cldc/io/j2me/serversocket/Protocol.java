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

package com.sun.cldc.io.j2me.serversocket;

import java.io.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;


/**
 * StreamConnectionNotifier to the Palm Server Socket API.
 *
 * @author  Nik Shaylor
 * @version 1.0 10/08/99
 */
public class Protocol extends NetworkConnectionBase implements StreamConnectionNotifier {

    /** Server Socket object */
    int handle;

    /**
     * Open the connection
     * @param name the target for the connection
     * @param writeable a flag that is true if the caller expects to write to the
     *        connection.
     * @param timeouts A flag to indicate that the called wants timeout exceptions
     * <p>
     * The name string for this protocol should be:
     * "serversocket://:<port number>" or "socket://:<port number>" (which is the same)
     */
    public native void open(String name, int mode, boolean timeouts) throws IOException;

    /**
     * Returns a connection that represents a server side
     * socket connection
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @return     a socket to communicate with a client.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          input stream.
     */
    synchronized public StreamConnection acceptAndOpen() throws IOException {
        com.sun.cldc.io.j2me.socket.Protocol con;
        while(true) {
            int handle = accept();
            if(handle >= 0) {
                con = new com.sun.cldc.io.j2me.socket.Protocol();
                con.open(handle, Connector.READ_WRITE);
                break;
            }
            Thread.yield();
        }
        return con;
    }
    private native int  accept() throws IOException;

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs when closing the
     *                          connection.
     */
    public native void close() throws IOException;
}
