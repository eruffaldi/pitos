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

package com.sun.cldc.io.j2me.datagram;

import java.io.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;

/**
 * This class is required because the J2SE Datagram class is final.
 */
public class DatagramObject extends GeneralBase implements Datagram {

    Protocol parent;
    byte[] buf;
    int off = 0;
    int len;

    int ipNumber;
    String host;
    int port;

    public DatagramObject(Protocol parent, byte[] buf, int len) {
        this.parent = parent;
        setData(buf, 0, len);
    }

    public String getAddress() {
        if(host == null) {
            return null;
        } else {
            return "datagram://"+host+":"+port;
        }
    }

    public byte[] getData() {
        return buf;
    }

    public int getOffset() {
        return off;
    }

    public int getLength() {
        return len;
    }

    synchronized public void setAddress(String addr) throws IOException {
        ipNumber = 0;
        if(!addr.startsWith("datagram://")) {
            throw new IllegalArgumentException("Invalid datagram address"+addr);
        }
        String address = addr.substring(11);
        try {
            host = Protocol.getAddress(address);
            port = Protocol.getPort(address);
        } catch(NumberFormatException x) {
            throw new IllegalArgumentException("Invalid datagram address"+addr);
        }
    }

    synchronized public void setAddress(Datagram reference) {
        DatagramObject ref = (DatagramObject)reference;
        host = ref.host;
        port = ref.port;
        ipNumber = 0;
    }

    synchronized public void setData(byte[] buf, int offset, int length) {
        /* this will check to see if buf is null */
        if (length < 0 || offset < 0 ||
            ((length + offset) > buf.length)) {
            throw new IllegalArgumentException("illegal length or offset");
        }
        this.buf = buf;
        this.off = offset;
        this.len = length;
    }

    public void setLength(int length) {
        setData(buf, off, length);
    }

/*
    public void setOffset(int offset) {
        dgram.setOffset(offset);
    }
*/


//
// Read / write functions.
//

    int pointer;

    public void reset() {
        setData(buf, 0, 0);
        pointer = 0;
    }

    public long skip(long n) {
        len = Math.min((int)n, len);
        pointer += len;
        return len;
    }

    public int read() throws IOException {
        if(pointer >= len) {
            return -1;
        }
        return buf[off+(pointer++)] & 0xFF;
    }

    public void write(int ch) throws IOException {
        if(pointer >= buf.length) {
            throw new IndexOutOfBoundsException();
        }
        buf[pointer++] = (byte)ch;
        off = 0;
        len = pointer;
    }


}
