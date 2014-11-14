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

package com.sun.cldc.io.j2me.socket;

import java.io.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;

/**
 * Connection to the J2ME socket API.
 *
 * @author  Nik Shaylor
 * @version 1.0 1/16/2000
 */

public class Protocol extends NetworkConnectionBase implements StreamConnection {

    /** Socket object used by native code */
    int handle;

    /** Access mode */
    private int mode;

    /** Open count */
    int opens = 0;

    /** Connection open flag */
    private boolean copen = false;

    /** Input stream open flag */
    protected boolean isopen = false;

    /** Output stream open flag */
    protected boolean osopen = false;

    /**
     * Open the connection
     */
    public void open(String name, int mode, boolean timeouts) throws IOException {
        throw new RuntimeException("Should not be called");
    }

    /**
     * Open the connection
     * @param name the target for the connection
     * @param writeable a flag that is true if the caller expects to write to the
     *        connection.
     * @param timeouts A flag to indicate that the called wants timeout exceptions
     * <p>
     * The name string for this protocol should be:
     * "socket://<name or IP number>:<port number>
     *
     * We allow "socket://:nnnn" to mean "serversocket://:nnnn" the native
     * code will spot this and signal back with an InterruptedException
     * to tell the calling Java code about this.
     */
    public Connection openPrim(String name, int mode, boolean timeouts) throws IOException {
        try {
            open0(name, mode, timeouts);
            opens++;
            copen = true;
            this.mode = mode;
            return this;
        } catch(InterruptedException x) {
            com.sun.cldc.io.j2me.serversocket.Protocol con;
            con = new com.sun.cldc.io.j2me.serversocket.Protocol();
            con.open(name, mode, timeouts);
            return con;
        }
     }

    /**
     * Open the connection
     * @param handle an already formed socket handle
     * <p>
     * This function is only used by com.sun.cldc.io.j2me.socketserver;
     */
    public void open(int handle, int mode) throws IOException {
        this.handle = handle;
        opens++;
        copen = true;
        this.mode = mode;
    }

    /**
     * Ensure connection is open
     */
    void ensureOpen() throws IOException {
        if(!copen) {
            throw new IOException("Connection closed");
        }
    }

    /**
     * Returns an input stream for this socket.
     *
     * @return     an input stream for reading bytes from this socket.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          input stream.
     */
    synchronized public InputStream openInputStream() throws IOException {
        ensureOpen();
        if((mode&Connector.READ) == 0) {
            throw new IOException("Connection not open for reading");
        }
        if(isopen) {
            throw new IOException("Input stream already opened");
        }
        isopen = true;
        InputStream in = new PrivateInputStream(this);
        opens++;
        return in;
    }

    /**
     * Returns an output stream for this socket.
     *
     * @return     an output stream for writing bytes to this socket.
     * @exception  IOException  if an I/O error occurs when creating the
     *                          output stream.
     */
    synchronized public OutputStream openOutputStream() throws IOException {
        ensureOpen();
        if((mode&Connector.WRITE) == 0) {
            throw new IOException("Connection not open for writing");
        }
        if(osopen) {
            throw new IOException("Output stream already opened");
        }
        osopen = true;
        OutputStream os = new PrivateOutputStream(this);
        opens++;
        return os;
    }

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs when closing the
     *                          connection.
     */
    synchronized public void close() throws IOException {
        if(copen) {
            copen = false;
            realClose();
        }
    }

    /**
     * Close the connection.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized void realClose() throws IOException {
        if(--opens == 0) {
             close0();
        }
    }

   /*
    * A note about read0() and write0()
    *
    * This function can be called with a null buffer and it will behave like a single
    * character read routine. In this form it return a character, -1, or -2. In the
    * form with a buffer it will return the read length, or -1,
    *
    * -1 means that EOF was reached.
    * -2 means that the read would have blocked and to allow other threads
    *    in the system to run a Thread.yield() should be performed.
    *    This is a simple form of asynchronous I/O, but is east to implement.
    *
    *    For read(byte[], int, int) calls if the returned length was less than
    *    that requested, again the calling thread should yield and then try again.
    *
    * write0() works the same way. It returns the number of bytes written. If the
    * number is less than the number requested this is because the implementation
    * would block and instead if blocking in native code the calling Java thread
    * should yield to let another thread run.
    *
    * Both of the above cases highlight how a very simple asynchronous I/O mechanism
    * can be made to work. There are other ways that are a little better where the
    * Java thread is blocked and unblocked in native code, but this is harder to
    * implement.
    */

    protected native void open0(String name, int mode, boolean timeouts) throws IOException, InterruptedException;
    protected native int  read0(byte b[], int off, int len) throws IOException;
    protected native int  write0(byte b[], int off, int len) throws IOException;
    protected native int  available0() throws IOException;
    protected native void close0() throws IOException;
}


/**
 * Input stream for the connection
 */
class PrivateInputStream extends InputStream {

    /**
     * Pointer to the connection
     */
    private Protocol parent;

    /**
     * End of file flag
     */
    boolean eof = false;

    /**
     * Constructor
     * @param pointer to the connection object
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateInputStream(Protocol parent) throws IOException {
        this.parent = parent;
    }

    /**
     * Check the stream is open
     *
     * @exception  IOException  if it is not.
     */
    void ensureOpen() throws IOException {
        if(parent == null) {
            throw new IOException("Stream closed");
        }
    }

    /**
     * Reads the next byte of data from the input stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public int read() throws IOException {
        int res;
        ensureOpen();
        if(eof) {
            return -1;
        }
        while(true) {
            res = parent.read0(null, 0, 1);
            if(res != -2) {
                break;
            }
            Thread.yield();
        }
        if(res == -1) {
            eof = true;
        }
        if(parent == null) {
            throw new InterruptedIOException();
        }
        return res;
    }


    /**
     * Reads up to <code>len</code> bytes of data from the input stream into
     * an array of bytes.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset in array <code>b</code>
     *                   at which the data is written.
     * @param      len   the maximum number of bytes to read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public int read(byte b[], int off, int len) throws IOException {
        ensureOpen();
        if(eof) {
            return -1;
        }
        if (b == null) {
            throw new NullPointerException();
        }
        if(len == 0) {
            return 0;
        }
        int n = 0;
        while (n < len) {
            int count = parent.read0(b, off + n, len - n);
            if(count == -1) {
                eof = true;
                if(n == 0) {
                    n = -1;
                }
                break;
            }
            n += count;
            if(n == len) {
                break;
            }
            Thread.yield();
        }
        if(parent == null) {
            throw new InterruptedIOException();
        }
        return n;
    }

    /**
     * Returns the number of bytes that can be read (or skipped over) from
     * this input stream without blocking by the next caller of a method for
     * this input stream.
     *
     * @return     the number of bytes that can be read from this input stream.
     * @exception  IOException  if an I/O error occurs.
     */
    synchronized public int available() throws IOException {
        ensureOpen();
        return parent.available0();
    }

    /**
     * Close the stream.
     *
     * @exception  IOException  if an I/O error occurs
     */
    public void close() throws IOException {
        if(parent != null) {
            ensureOpen();
            parent.realClose();
            parent.isopen = false;
            parent = null;
        }
    }
}


/**
 * Output stream for the connection
 */
class PrivateOutputStream extends OutputStream {

    /**
     * Pointer to the connection
     */
    private Protocol parent;

    /**
     * Constructor
     * @param pointer to the connection object
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public PrivateOutputStream(Protocol parent) throws IOException {
        this.parent = parent;
    }

    /**
     * Check the stream is open
     *
     * @exception  IOException  if it is not.
     */
    void ensureOpen() throws IOException {
        if(parent == null) {
            throw new IOException("Stream closed");
        }
    }

    /**
     * Writes the specified byte to this output stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b   the <code>byte</code>.
     * @exception  IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> may be thrown if the
     *             output stream has been closed.
     */
    synchronized public void write(int b) throws IOException {
        ensureOpen();
        while(true) {
            int res = parent.write0(null, b, 1);
            if(res != 0) {
                return;
            }
            Thread.yield();
        }
    }

    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this output stream.
     * <p>
     * Polling the native code is done here to allow for simple
     * asynchronous native code to be written. Not all implementations
     * work this way (they block in the native code) but the same
     * Java code works for both.
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs. In particular,
     *             an <code>IOException</code> is thrown if the output
     *             stream is closed.
     */
    synchronized public void write(byte b[], int off, int len) throws IOException {
        ensureOpen();
        if (b == null) {
            throw new NullPointerException();
        }
        if(len == 0) {
            return;
        }
        int n = 0;
        while (true) {
            n += parent.write0(b, off + n, len - n);
            if(n == len) {
                break;
            }
            Thread.yield();
        }
    }

    /**
     * Close the stream.
     *
     * @exception  IOException  if an I/O error occurs
     */
    public void close() throws IOException {
        if(parent != null) {
            ensureOpen();
            parent.realClose();
            parent.osopen = false;
            parent = null;
        }
    }

}
