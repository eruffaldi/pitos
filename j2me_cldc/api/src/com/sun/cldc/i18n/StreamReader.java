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

package com.sun.cldc.i18n;

import java.io.*;


/**
 * General prototype for character converting stream readers.
 *
 * @author  Nik Shaylor
 * @version 1.0 11/16/99
 */
public abstract class StreamReader extends Reader {

    /** Input stream to read from */
    public InputStream in;

    /**
     * Open the reader
     */
    public Reader open(InputStream in, String enc) throws UnsupportedEncodingException {
        this.in = in;
        return this;
    }

    /**
     * Tell whether the underlying byte stream is ready to be read.  Return
     * false for those streams that do not support available(), such as the
     * Win32 console stream.
     */
    public boolean ready() {
        try {
            return in.available() > 0;
        } catch (IOException x) {
            return false;
        }
    }

    /**
     * Close the stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void close() throws IOException {
        in.close();
    }

    /**
     * Get the size in chars of an array of bytes
     */
    public abstract int sizeOf(byte[] array, int offset, int length);



}
