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
 *
 * This interface defines the capabilities that a datagram connection
 * must have.<p>
 * The parameter string describing the target of the connection takes the
 * form:
 * <pre>{protocol}:[//{host}]:{port}</pre>
 *
 * A datagram connection can be opened in a "client" mode or a "server" mode.
 * If the "//{host}" is missing then it is opened as a "server" (by
 * "server", this means that a client application initiates communication).
 * When the "//{host}" is specified the connection is opened as a client.
 * <p>
 * Examples:
 * <p>
 *  A datagram connection for accepting datagrams<br>
 *  datagram://:1234<p>
 *  A datagram connection for sending to a server:<br>
 *  datagram://123.456.789.12:1234<p>
 *
 * Note that the port number in "server mode" (unspecified host name) is that
 * of the receiving port. The port number in "client mode" (host name
 * specified) is that of the target port. The reply to port in both cases is
 * never unspecified. In "server mode",
 * the same port number is used for both receiving and sending.
 * In "client mode", the reply-to port is always dynamically
 * allocated.<p>
 *
 * @author Brian Modra
 * @author Nik Shaylor
 * @version 1.1 1/7/2000
 */
public interface DatagramConnection extends Connection {
//    /**
//     * Get the address represented by this datagram endpoint. This is the
//     * address at which implementations of this interface receives datagrams.
//     *
//     * @return    address      The datagram address.
//     */
//     public String getAddress() throws IOException;

    /**
     * Get the maximum length a datagram can be.
     *
     * @return    address      The length.
     */
    public int getMaximumLength() throws IOException;

    /**
     * Get the nominal length of a datagram.
     *
     * @return    address      The length.
     */
    public int getNominalLength() throws IOException;

    /**
     * Send a datagram
     *
     * @param     dgram        A datagram.
     * @exception IOException  If an I/O error occurs.
     * @exception InterruptedIOException Timeout or upon closing the connection with outstanding I/O.
     */
    public void send(Datagram dgram) throws IOException;

    /**
     * Receive a datagram
     *
     * @param     dgram        A datagram.
     * @exception IOException  If an I/O error occurs.
     * @exception InterruptedIOException Timeout or upon closing the connection with outstanding I/O.
     */
    public void receive(Datagram dgram) throws IOException;

    /**
     * Make a new datagram object automatically allocating a buffer
     *
     * @param  size            The length of the buffer to be allocated for the datagram
     * @return                 A new datagram
     */
    public Datagram newDatagram(int size) throws IOException;

    /**
     * Make a new datagram object
     *
     * @param  size            The length of the buffer to be used
     * @param  addr            The address to which the datagram must go
     * @return                 A new datagram
     */
    public Datagram newDatagram(int size, String addr) throws IOException;

    /**
     * Make a new datagram object
     *
     * @param  buf             The buffer to be used in the datagram
     * @param  size            The length of the buffer to be allocated for the datagram
     * @return                 A new datagram
     * @exception IllegalArgumentException if the length is negative or larger than the buffer
     */
    public Datagram newDatagram(byte[] buf, int size) throws IOException;

    /**
     * Make a new datagram object
     *
     * @param  buf             The buffer to be used in the datagram
     * @param  size            The length of the buffer to be used
     * @param  addr            The address to which the datagram must go
     * @return                 A new datagram
     * @exception IllegalArgumentException if the length is negative or larger than the buffer
     */
    public Datagram newDatagram(byte[] buf, int size, String addr) throws IOException;

}

