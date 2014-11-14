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

package javax.microedition.io;

import java.io.*;

/**
 * This is the generic datagram interface. It represents an object that will act as
 * the holder of data to be send or received from a datagram connection.
 * The DataInput and DataOutput interfaces are extended by
 * this interface to provide a simple way to read and write binary data in and out of
 * the datagram buffer. A special function reset() may be called to reset the read/write
 * point to the beginning of the buffer.
 *
 * @author  Brian Modra
 * @author  Nik Shaylor
 * @version 1.2 1/17/2000
 */
public interface Datagram extends DataInput, DataOutput {

        /**
         * Get the address in the datagram
         *
         * @return the address in string form, or null if no address was set
         */
        public String getAddress();

        /**
         * Get the buffer
         *
         * @return the data buffer
         */
        public byte[] getData();

        /**
         * Get the length
         *
         * @return the length of the data
         */
        public int getLength();

        /**
         * Get the offset
         *
         * @return the offset into the data buffer
         */
        public int getOffset();

        /**
         * Set datagram address.
         *
         * The parameter string describing the target of the datagram
         * takes the form:
         * <pre>
         *     {protocol}:{target}
         * </pre>
         * E.g. The "target" can be "//{host}:{port}" (but is not necessarily
         * limited to this.)<br>
         * So in this example a datagram connection for sending to a server
         * could be addressed as so:
         * <pre>
         *     datagram://123.456.789.12:1234
         * </pre>
         *
         * Note that if the address of a datagram is not specified, then it
         * defaults to that of the connection.
         *
         * @param addr the new target address as a URL
         * @exception IllegalArgumentException if the address is not valid
         */
        public void setAddress(String addr) throws IOException;

        /**
         * Set datagram address, copying the address from another datagram.
         *
         * @param reference the datagram who's address will be copied as
         * the new target address for this datagram.
         * @exception IllegalArgumentException if the address is not valid
         */
        public void setAddress(Datagram reference);

        /**
         * Set the length
         *
         * @param len the new length of the data
         * @exception IllegalArgumentException if the length is negative or larger than the buffer
         */
        public void setLength(int len);

        /**
         * Set the buffer, offset and length
         *
         * @param addr the data buffer
         * @param offset the offset into the data buffer
         * @param len the length of the data in the buffer
         * @exception IllegalArgumentException if the length or offset fall outside the buffer
         */
        public void setData(byte[] buffer, int offset, int len);

        /**
         * Reset the read/write pointer and zeros the offset and length parameters.
         */
        public void reset();

}
