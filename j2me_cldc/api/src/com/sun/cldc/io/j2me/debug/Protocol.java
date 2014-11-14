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

package com.sun.cldc.io.j2me.debug;

import java.io.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;

/**
 * Connection to the J2ME debug stream
 *
 * @author  Nik Shaylor
 * @version 1.0 2/4/2000
 */

public class Protocol extends ConnectionBase implements OutputConnection {

    protected boolean opened = false;

    /**
     * Open the connection
     * @param name the target for the connection
     * @param writeable a flag that is true if the caller expects to write to the
     *        connection.
     * @param timeouts A flag to indicate that the called wants timeout exceptions
     */
     public void open(String name, int mode, boolean timeouts) throws IOException {
         if(!name.equals("")) {
             throw new IllegalArgumentException("Bad protocol option:"+name);
         }
     }

    /**
     * Returns an output stream for this socket.
     *
     * @return     an output stream for writing bytes to this socket.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          output stream.
     */
    public OutputStream openOutputStream() throws IOException {
        if(opened) {
            throw new IOException("Stream already opened");
        }
        opened = true;
        return new PrivateOutputStream(this);
    }
}


/**
 * Output stream for the connection
 */
class PrivateOutputStream extends OutputStream {

    /**
     * Pointer to the connection
     */
    Protocol parent;

    /**
     * Constructor
     *
     * @param pointer to the parent connection
     */
    public PrivateOutputStream(Protocol p) {
        parent = p;
    }

    /**
     * Writes the specified byte to this output stream.
     *
     * @param      b   the <code>byte</code>.
     * @exception  IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> may be thrown if the
     *             output stream has been closed.
     */
    synchronized public void write(int c) throws IOException {
        if(parent == null) {
            throw new IOException("Connection closed");
        }
        putchar((char)c);
    }
    private static native void putchar(char c);

    /**
     * Close the stream
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public void close() {
        if(parent != null) {
            parent.opened = false;
            parent = null;
        }
    }
}
