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

package com.sun.cldc.io.j2me.events;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;

/**
 * This implements the default "events:" protocol KVM
 *
 * @author  Nik Shaylor
 * @version 1.0 1/15/2000
 */
public class Protocol extends ConnectionBase implements InputConnection {

    /**
     * @param name The target of the connection
     * @param mode A specifier of the access mode
     * @param timeouts A flag to indicate that the called wants timeout exceptions
     */
    public void open(String name, int mode, boolean timeouts) throws IOException {
    }

    /**
     * Returns an input stream for a database record
     *
     * @return     an input stream for reading bytes from this record.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          input stream.
     */
    public InputStream openInputStream() throws IOException {
        return new PrivateInputStream();
    }

    public void close() throws IOException {
    }



// Start of temporary hack

    /**
     * Reads from the stream a representation of a byte array.
     *
     * @return     a byte array.
     * @exception  EOFException   if the input stream reaches the end before all the bytes.
     * @exception  IOException    if an I/O error occurs.
     */
    public byte[] readByteArray(DataInput in) throws IOException {
        return ((PrivateInputStream)in).readByteArray();
    }

// End of temporary hack
}


/**
 * Input stream class for the above connection
 */
class PrivateInputStream extends InputStream implements DataInput {

    /**
     * Constructor
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateInputStream() throws IOException {
        open();
    }
    private native void open() throws IOException;

    /**
     * Reads the next byte of data from the input stream.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public int read() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }

    /**
     * See the general contract of the <code>readInt</code>
     * method of <code>DataInput</code>.
     *
     * @return     the next four bytes of this input stream, interpreted as an
     *             <code>int</code>.
     * @exception  EOFException  if this input stream reaches the end before
     *               reading four bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    public native int readInt() throws IOException;

    /**
     * Reads from the stream a representation of a byte array.
     *
     * @return     a byte array.
     * @exception  EOFException   if the input stream reaches the end before all the bytes.
     * @exception  IOException    if an I/O error occurs.
     */
    public native byte[] readByteArray() throws IOException;

    /**
     * Close the stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public native void close() throws IOException;



    public void readFully(byte b[]) throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public void readFully(byte b[], int off, int len) throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public int skipBytes(int n) throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public boolean readBoolean() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public byte readByte() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public int readUnsignedByte() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public short readShort() throws IOException{
        throw new RuntimeException("events: function not implemented");
    }
    public int readUnsignedShort() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public char readChar() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public long readLong() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
    public String readUTF() throws IOException {
        throw new RuntimeException("events: function not implemented");
    }
}
