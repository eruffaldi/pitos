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

/*
 * WARNING - THIS IS AN EXPERIMENTAL FEATURE OF KVM THAT MAY, OR MAY NOT
 * EXIST IN A FUTURE VERSION OF THIS PRODUCT. IT IS NOT A PART OF THE
 * CLDC SPECIFICATION AND IS PROVIDED FOR ILLUSTRATIVE PURPOSES ONLY
 */

package com.sun.cldc.io.connections;

import  java.io.*;
import  javax.microedition.io.*;

/**
 * A connection to a random access memory
 *
 * @author  Nik Shaylor
 * @version 1.0 2/8/2000
 */
public interface RandomAccessConnection extends InputConnection, OutputConnection, DataInput, DataOutput {

//
// Selection
//

    /**
     * Test to see if a record in the collection is selected. Until this is done none of the
     * data access methods below will work. Some random access connection are always
     * selected, but other ones, such as a MetadataConncetion may require a specific
     * record to be selected before I/O can commence.
     *
     * @return true if the connection is selected, otherwise false.
     */
    public boolean isSelected() throws IOException;

//
// Seeking
//

    /**
     * Sets the position pointer offset, measured from the beginning of the
     * data, at which the next read or write occurs.  The offset may be
     * set beyond the end of the data. Setting the offset beyond the end
     * of the data does not change the data length.  The data length will
     * change only by writing after the offset has been set beyond the end
     * of the data.
     *
     * @param      pos   the offset position, measured in bytes from the
     *                   beginning of the data, at which to set the position
     *                   pointer.
     * @exception  IOException  if <code>pos</code> is less than
     *                          <code>0</code>, if an I/O error occurs, or
     *                          there is an input or output stream open on the data.
     */
    public void seek(long pos) throws IOException;

    /**
     * Returns the current offset into the data.
     *
     * @return     the offset from the beginning of the data, in bytes,
     *             at which the next read or write occurs.
     * @exception  IOException  if an I/O error occurs.
     */
    public long getPosition() throws IOException;

//
// Size management
//

    /**
     * Get the length of the data
     *
     * @return the length of the data
     */
    public long getLength() throws IOException;

    /**
     * Set the length of the data (for truncation).
     *
     * @param len the new length of the data
     */
    public void setLength(long len) throws IOException;

//
// I/O
//

    /**
     * Reads a byte of data.  The byte is returned as an
     * integer in the range 0 to 255 (<code>0x00-0x0ff</code>). This
     * method blocks if no input is yet available.
     * <p>
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             data has been reached.
     * @exception  IOException  if an I/O error occurs. Not thrown if
     *                          end-of-data has been reached.
     */
    public int read() throws IOException;

    /**
     * Reads up to <code>len</code> bytes of data into an
     * array of bytes. This method blocks until at least one byte of input
     * is available.
     *
     * @param      b     the buffer into which the data is read.
     * @param      off   the start offset of the data.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the data has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public int read(byte b[], int off, int len) throws IOException;

    /**
     * Writes the specified byte to this file. The write starts at
     * the current position pointer.
     *
     * @param      b   the <code>byte</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(int b) throws IOException;

    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to the data
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(byte b[], int off, int len) throws IOException;

}
