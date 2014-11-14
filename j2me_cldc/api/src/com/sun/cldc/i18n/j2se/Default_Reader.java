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
 * Default class reading input streams
 *
 * @author  Nik Shaylor
 * @version 1.0 1/17/2000
 */

public class Default_Reader extends StreamReader {

    public Reader open(InputStream in, String enc) throws UnsupportedEncodingException {
        if(enc == null) {
            return new InputStreamReader(in);
        } else {
            return new InputStreamReader(in, enc);        
        }
    }

    /**
     * Read characters into a portion of an array.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public int read(char cbuf[], int off, int len) throws IOException {
        throw new RuntimeException("Illegal call to J2SE_Reader::read");
    }

    /**
     * Get the size in chars of an array of bytes
     */
    public int sizeOf(byte[] array, int offset, int length) {
        throw new RuntimeException("Illegal call to J2SE_Reader::sizeOf");
    }
    
}
