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

package com.sun.cldc.i18n.j2me;

import java.io.*;
import com.sun.cldc.i18n.*;

/**
 * Default class for writing output streams.
 *
 * @author  Nik Shaylor
 * @version 1.0 10/18/99
 */
public class ISO8859_1_Writer extends StreamWriter {

    /**
     * Write a single character.
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(int c) throws IOException {
        if(c > 255) {
            c = '?';                // was ---->    throw new RuntimeException("Unknown character "+c);
        }
        out.write(c);
    }

    /**
     * Write a portion of an array of characters.
     *
     * @param  cbuf  Buffer of characters to be written
     * @param  off   Offset from which to start reading characters
     * @param  len   Number of characters to be written
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(char cbuf[], int off, int len) throws IOException {
        while(len-- > 0) {
            write(cbuf[off++]);
        }
    }

    /**
     * Write a portion of a string.
     *
     * @param  str  String to be written
     * @param  off  Offset from which to start reading characters
     * @param  len  Number of characters to be written
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(String str, int off, int len) throws IOException {
        for (int i = 0 ; i < len ; i++) {
            write(str.charAt(off + i));
        }
    }

    /**
     * Get the size in bytes of an array of chars
     */
    public int sizeOf(char[] array, int offset, int length) {
        return length;
    }

}
