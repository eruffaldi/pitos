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
 * This is the default "datagram://" protocol for J2ME that maps onto UDP
 *
 * @author  Nik Shaylor
 * @version 1.1 11/19/99
 */
public class Protocol extends NetworkConnectionBase implements DatagramConnection {

    /**
     * Used by native code
     */
    private int handle;

    /**
     * Machine name
     */
    private String host = null;

    /**
     * Port
     */
    private int port;

    /**
     * Open flag
     */
    private boolean open;

    /**
     * Local function to get the machine address from a string
     */
    protected static String getAddress(String name) throws IOException {
        /* Look for the : */
        int colon = name.indexOf(':');

        if(colon < 0) {
            throw new IllegalArgumentException("No ':' in protocol name "+name);
        }

        if(colon == 0) {
            return null;
        } else {
            return name.substring(0, colon);
        }
    }

    /**
     * Local function to get the port number from a string
     */
    protected static int getPort(String name) throws IOException, NumberFormatException {
        /* Look for the : */
        int colon = name.indexOf(':');

        if(colon < 0) {
            throw new IllegalArgumentException("No ':' in protocol name "+name);
        }

        int res = Integer.parseInt(name.substring(colon+1));
        if(res < 0 || res > 0xFFFF) {
            throw new IllegalArgumentException("Bad port number "+name);
        }
        return res;
    }

    /**
     * Open a connection to a target. <p>
     *
     * The name string for this protocol should be:
     * "[address:][port]"
     *
     * @param name the target of the connection
     * @param writeable a flag that is true if the caller intends to
     *        write to the connection.
     * @param timeouts A flag to indicate that the called wants timeout exceptions
     */
    public void open(String name, int mode, boolean timeouts) throws IOException {

        if(name.charAt(0) != '/' || name.charAt(1) != '/') {
            throw new IllegalArgumentException("Protocol must start with \"//\" "+name);
        }

        name = name.substring(2);
        host = getAddress(name);
        port = getPort(name);

       /*
        * If 'name' == null then we are a server endpoint at port 'port'
        *
        * If 'name' != null we are a client endpoint at an port decided by the system
        *              and the default address for datagrams to be send is 'host':'port'
        */

        if(port <= 0) {
            throw new IllegalArgumentException("Bad port number \"//\" "+name);
        }

        open0(mode, timeouts, (host == null) ? port : 0);
        open = true;
    }

    /**
     * Ensure connection is open
     */
    void ensureOpen() throws IOException {
        if(!open) {
            throw new IOException("Connection closed");
        }
    }

    /**
     * Get the address represented by this datagram endpoint
     *
     * @return  The datagram address
     */
    public String getAddress() throws IOException {
        ensureOpen();
        throw new RuntimeException("Function not implemented"); // I think we should remove this from the interface
    }

    /**
     * Get the maximum length a datagram can be.
     *
     * @return  The length
     */
    public int getMaximumLength() throws IOException {
        ensureOpen();
        return getMaximumLength();
    }

    /**
     * Get the nominal length of a datagram.
     *
     * @return    address      The length
     */
    public int getNominalLength() throws IOException {
        ensureOpen();
        return getNominalLength0();
    }


    /**
     * Send a datagram
     *
     * @param     dgram        A datagram
     * @exception IOException  If an I/O error occurs
     */
    synchronized public void send(Datagram dgram) throws IOException {
        ensureOpen();
        DatagramObject dh = (DatagramObject)dgram;
        if(dh.ipNumber == 0) {
            dh.ipNumber = getIpNumber((dh.host==null) ? "localhost" : dh.host);
        }

        while(true) {
            int res = send0(dh.ipNumber, dh.port, dh.buf, dh.off, dh.len);
            if(res == dh.len) {
                break;
            }
            if(res != 0) {
                throw new IOException("Failed to send datagram");
            }
            Thread.yield();
            if(!open) {
                throw new InterruptedIOException("Socket closed");
            }
        }
    }


    /**
     * Receive a datagram
     *
     * @param     dgram        A datagram
     * @exception IOException  If an I/O error occurs
     */
    synchronized public void receive(Datagram dgram) throws IOException {
        ensureOpen();
        DatagramObject dh = (DatagramObject)dgram;
        if(dh.ipNumber == 0) {
            dh.ipNumber = getIpNumber((dh.host==null) ? "localhost" : dh.host);
        }

        if(dh.len == 0) {
            throw new IOException("Bad datagram length");
        }

        long res;
        int count;
        while(true) {
            res = receive0(dh.buf, dh.off, dh.len);
            count = ((int)res) & 0xFFFF;
            if(count != 0) {
                break;
            }
            Thread.yield();
            if(!open) {
                throw new InterruptedIOException("Socket closed");
            }
        }

        dh.len      = count;
        dh.ipNumber = (int)((res >> 32));
        dh.port     = (int)((res >> 16)) & 0xFFFF;
        dh.pointer  = 0;
    }

    /**
     * Close the connection to the target.
     *
     * @exception IOException  If an I/O error occurs
     */
    public void close() throws IOException {
        if(open) {
            open = false;
            close0();
        }
    }

    /**
     * Get a new datagram object
     *
     * @return                 A new datagram
     */
    public Datagram newDatagram(int size) throws IOException {
        return newDatagram(new byte[size], size);
    }

    /**
     * Get a new datagram object
     *
     * @param     addr         The address to which the datagram must go
     * @return                 A new datagram
     */
    public Datagram newDatagram(int size, String addr) throws IOException {
        return newDatagram(new byte[size], size, addr);
    }

    /**
     * Get a new datagram object
     *
     * @return                 A new datagram
     */
    public Datagram newDatagram(byte[] buf, int size) throws IOException {
        ensureOpen();
        DatagramObject dg = new DatagramObject(this, buf, size);
        if(host != null) {
            dg.host = host; // Default host
            dg.port = port; // and port #
        }
        return dg;
    }

    /**
     * Get a new datagram object
     *
     * @param     addr         The address to which the datagram must go
     * @return                 A new datagram
     */
    public Datagram newDatagram(byte[] buf, int size, String addr) throws IOException {
        ensureOpen();
        DatagramObject dh = new DatagramObject(this, buf, size);
        dh.setAddress(addr);
        return dh;
    }

    native void open0(int mode, boolean timeouts, int port) throws IOException;
    native int getIpNumber(String s) throws IOException;
    native int send0(int ipnumber, int port, byte[] buf, int off, int len) throws IOException;
    native int getMaximumLength0() throws IOException;
    native int getNominalLength0() throws IOException;
    native long receive0(byte[] buf, int off, int len) throws IOException;
    native void close0() throws IOException;

}
