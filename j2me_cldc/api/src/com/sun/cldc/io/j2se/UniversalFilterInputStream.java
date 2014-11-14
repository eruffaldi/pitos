/*
 *  Copyright (c) 2000 Sun Microsystems, Inc., 901 San Antonio Road,
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

package com.sun.cldc.io.j2se;

import java.io.*;
import javax.microedition.io.*;
//import com.sun.cldc.io.*;

/**
 * This class is a an extension of the J2SE class FilterInputStream. It has the
 * same interface as FilterInputStream but in addition it extends
 * UniversalInputStream and so provided all the functionality therein. This class
 * is useful to convert a regular InputStream into a UniversalInputStream.
 *
 * @author  Nik Shaylor
 * @version 1.0 1/7/2000
 */
public
class UniversalFilterInputStream extends InputStream {
    /**
     * The input stream to be filtered.
     */
    protected InputStream in;

    /**
     * A flag to show if reset() will seek to the start of the data
     */
    protected boolean marked = false;

    /**
     * Connection object that needs closing
     */
    protected Connection con;

    /**
     * Creates a <code>UniversalFilterInputStream</code>
     * by assigning the  argument <code>in</code>
     * to the field <code>this.in</code> so as
     * to remember it for later use.
     *
     * @param   in   the underlying input stream.
     */
    public UniversalFilterInputStream(InputStream in) {
        this.in = in;
        if(in.markSupported()) {
            try {
                in.mark(in.available());
                marked = true;
            } catch(IOException x) {
            }
        }
    }

    /**
     * Creates a <code>UniversalFilterInputStream</code>
     * by assigning the  argument <code>in</code>
     * to the field <code>this.in</code> so as
     * to remember it for later use. Also accepts a Connection
     * parameter that must be closed after the stream is closed
     *
     * @param   con the connection which must be closed when this stream is closed
     *
     * @param   in   the underlying input stream.
     */
    public UniversalFilterInputStream(Connection con, InputStream in) {
        this(in);
        this.con = con;
    }

    /**
     * Reads the next byte of data from this input stream. The value
     * byte is returned as an <code>int</code> in the range
     * <code>0</code> to <code>255</code>. If no byte is available
     * because the end of the stream has been reached, the value
     * <code>-1</code> is returned. This method blocks until input data
     * is available, the end of the stream is detected, or an exception
     * is thrown.
     * <p>
     * This method
     * simply performs <code>in.read()</code> and returns the result.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterInputStream#in
     */
    public int read() throws IOException {
       return in.read();
    }

    /**
     * Reads up to <code>len</code> bytes of data from this input stream
     * into an array of bytes. This method blocks until some input is
     * available.
     * <p>
     * This method simply performs <code>in.read(b, off, len)</code>
     * and returns the result.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterInputStream#in
     */
    public int read(byte b[], int off, int len) throws IOException {
        return in.read(b, off, len);
    }

    /**
     * Skips over and discards <code>n</code> bytes of data from the
     * input stream. The <code>skip</code> method may, for a variety of
     * reasons, end up skipping over some smaller number of bytes,
     * possibly <code>0</code>. The actual number of bytes skipped is
     * returned.
     * <p>
     * This method
     * simply performs <code>in.skip(n)</code>.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    public long skip(long n) throws IOException {
        return in.skip(n);
    }

    /**
     * Returns the number of bytes that can be read from this input
     * stream without blocking.
     * <p>
     * This method
     * simply performs <code>in.available(n)</code> and
     * returns the result.
     *
     * @return     the number of bytes that can be read from the input stream
     *             without blocking.
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterInputStream#in
     */
    public int available() throws IOException {
        return in.available();
    }

    /**
     * Closes this input stream and releases any system resources
     * associated with the stream.
     * This
     * method simply performs <code>in.close()</code>.
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterInputStream#in
     */
    public void close() throws IOException {
        in.close();
        if(con != null) {
            con.close();
        }
    }

    /**
     * Marks the current position in this input stream. A subsequent
     * call to the <code>reset</code> method repositions this stream at
     * the last marked position so that subsequent reads re-read the same bytes.
     * <p>
     * The <code>readlimit</code> argument tells this input stream to
     * allow that many bytes to be read before the mark position gets
     * invalidated.
     * <p>
     * This method simply performs <code>in.mark(readlimit)</code>.
     *
     * @param   readlimit   the maximum limit of bytes that can be read before
     *                      the mark position becomes invalid.
     * @see     javax.microedition.io.UniversalFilterInputStream#in
     * @see     javax.microedition.io.UniversalFilterInputStream#reset()
     */
    public synchronized void mark(int readlimit) {
        marked = false; /* If the caller is marking then we cannot */
        in.mark(readlimit);
    }

    /**
     * Repositions this stream to the position at the time the
     * <code>mark</code> method was last called on this input stream.
     * <p>
     * This method
     * simply performs <code>in.reset()</code>.
     * <p>
     * Stream marks are intended to be used in
     * situations where you need to read ahead a little to see what's in
     * the stream. Often this is most easily done by invoking some
     * general parser. If the stream is of the type handled by the
     * parse, it just chugs along happily. If the stream is not of
     * that type, the parser should toss an exception when it fails.
     * If this happens within readlimit bytes, it allows the outer
     * code to reset the stream and try another parser.
     *
     * @exception  IOException  if the stream has not been marked or if the
     *               mark has been invalidated.
     * @see        javax.microedition.io.UniversalFilterInputStream#in
     * @see        javax.microedition.io.UniversalFilterInputStream#mark(int)
     */
    public synchronized void reset() throws IOException {
        in.reset();
    }

    /**
     * Tests if this input stream supports the <code>mark</code>
     * and <code>reset</code> methods.
     * This method
     * simply performs <code>in.markSupported()</code>.
     *
     * @return  <code>true</code> if this stream type supports the
     *          <code>mark</code> and <code>reset</code> method;
     *          <code>false</code> otherwise.
     * @see     javax.microedition.io.UniversalFilterInputStream#in
     * @see     java.io.InputStream#mark(int)
     * @see     java.io.InputStream#reset()
     */
    public boolean markSupported() {
        return in.markSupported();
    }

    /**
     * Seek to a position in the stream
     *
     * @param pos the offset in bytes from the start of the data
     */
    public void seek(long pos) throws IOException, IllegalAccessException {
        if(marked) {
            in.reset();
            in.skip((int)pos);
        } else {
            throw new IllegalAccessException();
        }
    }
}
