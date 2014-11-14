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

package com.sun.cldc.i18n.j2se;

import java.io.*;
import com.sun.cldc.i18n.*;

/**
 * Default class for writing output streams.
 *
 * @author  Nik Shaylor
 * @version 1.0 1/17/2000
 */
public class Default_Writer extends StreamWriter {

    public Writer open(OutputStream in, String enc)  throws UnsupportedEncodingException {
        if(enc == null) {
            return new OutputStreamWriter(in);
        } else {
            return new OutputStreamWriter(in, enc);        
        }
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
    public void write(char cbuf[], int off, int len) throws IOException {
        throw new RuntimeException("Illegal call to J2SE_Writer::write");
    }


    /**
     * Get the size in bytes of an array of chars
     */
    public int sizeOf(char[] array, int offset, int length) {
        throw new RuntimeException("Illegal call to J2SE_Writer::sizeOf");
    }

}
