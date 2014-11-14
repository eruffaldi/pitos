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
  * This class is a an extension of the J2SE class FilterOutputStream. It has the
  * same interface as FilterOutputStream but in addition it extends
  * UniversalOutputStream and so provided all the functionality therein. This class
  * is useful to convert a regular OutputStream into a UniversalOutputStream.
  *
  * @author  Nik Shaylor
  * @version 1.0 1/7/2000
 */
public
class UniversalFilterOutputStream extends OutputStream {
    /**
     * The underlying output stream to be filtered.
     */
    protected OutputStream out;

    /**
     * Connection object that need closing
     */
    protected Connection con;

    /**
     * Creates an output stream filter built on top of the specified
     * underlying output stream.
     *
     * @param   out   the underlying output stream to be assigned to
     *                the field <tt>this.out</tt> for later use, or
     *                <code>null</code> if this instance is to be
     *                created without an underlying stream.
     */
    public UniversalFilterOutputStream(OutputStream out) {
        this.out = out;
    }

    /**
     * Creates an output stream filter built on top of the specified
     * underlying output stream.
     *
     * @param   con the connection which must be closed when this stream is closed
     *
     * @param   out   the underlying output stream to be assigned to
     *                the field <tt>this.out</tt> for later use, or
     *                <code>null</code> if this instance is to be
     *                created without an underlying stream.
     */
    public UniversalFilterOutputStream(Connection con, OutputStream out) {
        this.out = new BufferedOutputStream(out);
        this.con = con;
    }

    /**
     * Writes the specified <code>byte</code> to this output stream.
     * <p>
     * The <code>write</code> method of <code>UniversalFilterOutputStream</code>
     * calls the <code>write</code> method of its underlying output stream,
     * that is, it performs <tt>out.write(b)</tt>.
     * <p>
     * Implements the abstract <tt>write</tt> method of <tt>OutputStream</tt>.
     *
     * @param      b   the <code>byte</code>.
     * @exception  IOException  if an I/O error occurs.
     */

    public void write(int b) throws IOException {
        out.write(b);
    }

    /**
     * Writes <code>b.length</code> bytes to this output stream.
     * <p>
     * The <code>write</code> method of <code>UniversalFilterOutputStream</code>
     * calls its <code>write</code> method of three arguments with the
     * arguments <code>b</code>, <code>0</code>, and
     * <code>b.length</code>.
     * <p>
     * Note that this method does not call the one-argument
     * <code>write</code> method of its underlying stream with the single
     * argument <code>b</code>.
     *
     * @param      b   the data to be written.
     * @exception  IOException  if an I/O error occurs.
     * @~~~see        java.io.UniversalFilterOutputStream#write(byte[], int, int)
     */
    public void write(byte b[]) throws IOException {
        write(b, 0, b.length);
    }

    /**
     * Writes <code>len</code> bytes from the specified
     * <code>byte</code> array starting at offset <code>off</code> to
     * this output stream.
     * <p>
     * The <code>write</code> method of <code>UniversalFilterOutputStream</code>
     * calls the <code>write</code> method of one argument on each
     * <code>byte</code> to output.
     * <p>
     * Note that this method does not call the <code>write</code> method
     * of its underlying input stream with the same arguments. Subclasses
     * of <code>UniversalFilterOutputStream</code> should provide a more efficient
     * implementation of this method.
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterOutputStream#write(int)
     */
    public void write(byte b[], int off, int len) throws IOException {
        if ((off | len | (b.length - (len + off)) | (off + len)) < 0)
            throw new IndexOutOfBoundsException();

        for (int i = 0 ; i < len ; i++) {
            write(b[off + i]);
        }
    }

    /**
     * Flushes this output stream and forces any buffered output bytes
     * to be written out to the stream.
     * <p>
     * The <code>flush</code> method of <code>UniversalFilterOutputStream</code>
     * calls the <code>flush</code> method of its underlying output stream.
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterOutputStream#out
     */
    public void flush() throws IOException {
        out.flush();
    }

    /**
     * Closes this output stream and releases any system resources
     * associated with the stream.
     * <p>
     * The <code>close</code> method of <code>UniversalFilterOutputStream</code>
     * calls its <code>flush</code> method, and then calls the
     * <code>close</code> method of its underlying output stream.
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        javax.microedition.io.UniversalFilterOutputStream#flush()
     * @see        javax.microedition.io.UniversalFilterOutputStream#out
     */
    public void close() throws IOException {
        try {
          flush();
        } catch (IOException ignored) {
        }
        out.close();
        if(con != null) {
            con.close();
        }
    }
}
