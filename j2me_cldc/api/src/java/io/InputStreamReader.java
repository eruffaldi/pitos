/*
 * @(#)InputStreamReader.java   1.24 99/12/04
 *
 * Copyright 1996-1999 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the proprietary information of Sun Microsystems, Inc.
 * Use is subject to license terms.
 *
 */

package java.io;

import com.sun.cldc.i18n.*;

/**
 * An InputStreamReader is a bridge from byte streams to character streams: It
 * reads bytes and translates them into characters according to a specified <a
 * href="../lang/package-summary.html#charenc">character encoding</a>.  The
 * encoding that it uses may be specified by name, or the platform's default
 * encoding may be accepted.
 *
 * <p> Each invocation of one of an InputStreamReader's read() methods may
 * cause one or more bytes to be read from the underlying byte-input stream.
 * To enable the efficient conversion of bytes to characters, more bytes may
 * be read ahead from the underlying stream than are necessary to satisfy the
 * current read operation.
 *
 * @version     1.0, 12/15/99
 * @author      Nik Shaylor
 */

public class InputStreamReader extends Reader {

    /**
     * The underlying character-input stream.
     */
    private Reader in;

    /**
     * Create an InputStreamReader that uses the default character encoding.
     *
     * @param  in   An InputStream
     */
    public InputStreamReader(InputStream is) {
        in = Helper.getStreamReader(is);
    }

    /**
     * Create an InputStreamReader that uses the named character encoding.
     *
     * @param  in   An InputStream
     * @param  enc  The name of a supported
     *
     * @exception  UnsupportedEncodingException
     *             If the named encoding is not supported
     */
    public InputStreamReader(InputStream is, String enc)
        throws UnsupportedEncodingException
    {
        in = Helper.getStreamReader(is, enc);
    }

    /** Check to make sure that the stream has not been closed */
    private void ensureOpen() throws IOException {
        if (in == null) {
            throw new IOException("Stream closed");
        }
    }

    /**
     * Read a single character.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public int read() throws IOException {
        ensureOpen();
        return in.read();
    }

    /**
     * Read characters into a portion of an array.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public int read(char cbuf[], int off, int len) throws IOException {
        ensureOpen();
        if ((off < 0) || (off > cbuf.length) || (len < 0) ||
            ((off + len) > cbuf.length) || ((off + len) < 0)) {
            throw new IndexOutOfBoundsException();
        } else if (len == 0) {
            return 0;
        }
        return in.read(cbuf, off, len);
    }

    /**
     * Skip characters.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public long skip(long n) throws IOException {
        ensureOpen();
        return in.skip(n);
    }

    /**
     * Tell whether this stream is ready to be read.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public boolean ready() throws IOException {
        ensureOpen();
        return in.ready();
    }

    /**
     * Tell whether this stream supports the mark() operation.
     */
    public boolean markSupported() {
        if(in == null) {
            return false;
        }
        return in.markSupported();
    }

    /**
     * Mark the present position in the stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void mark(int readAheadLimit) throws IOException {
        ensureOpen();
        in.mark(readAheadLimit);
    }

    /**
     * Reset the stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void reset() throws IOException {
        ensureOpen();
        in.reset();
    }

    /**
     * Close the stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void close() throws IOException {
        if(in != null) {
            in.close();
            in = null;
        }
    }

}
