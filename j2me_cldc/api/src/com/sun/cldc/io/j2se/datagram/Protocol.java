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

package com.sun.cldc.io.j2se.datagram;

import java.io.*;
import java.net.*;
import javax.microedition.io.*;
import com.sun.cldc.io.j2se.*;
import com.sun.cldc.io.*;

/**
 * This implements the "datagram://" protocol for J2SE in a not very
 * efficient way.
 *
 * @author  Nik Shaylor
 * @version 1.1 11/19/99
 */
public class Protocol extends ConnectionBase implements DatagramConnection {

    DatagramSocket endpoint;

    /**
     * Machine name
     */
    private String host = null;

    /**
     * Port
     */
    private int port;

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

        return Integer.parseInt(name.substring(colon+1));
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
    public void open(String name, int mode, boolean timeout) throws IOException {

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

        if(host == null) {
            /* Open the socket */
            endpoint = new DatagramSocket(port);
        } else {
            endpoint = new DatagramSocket(0);
        }
    }

   /**
     * Get the address represented by this datagram endpoint
     *
     * @return    address      The datagram addre4ss
     */
     public String getAddress() {
         InetAddress addr = endpoint.getLocalAddress();
         return "datagram://" + addr.getHostName() + ":" + addr.getHostAddress();
     }

    /**
     * Get the maximum length a datagram can be.
     *
     * @return    address      The length
     */
    public int getMaximumLength() throws IOException {
        try {
            return endpoint.getReceiveBufferSize();
        } catch(java.net.SocketException x) {
            throw new IOException(x.getMessage());
        }
    }

    /**
     * Get the nominal length of a datagram.
     *
     * @return    address      The length
     */
    public int getNominalLength() throws IOException {
        return getMaximumLength();
    }

    /**
     * Change the timeout period
     *
     * @param     milliseconds The maximum time to wait
     */
    public void setTimeout(int milliseconds) {
    }

    /**
     * Send a datagram
     *
     * @param     dgram        A datagram
     * @exception IOException  If an I/O error occurs
     */
    public void send(Datagram dgram) throws IOException {
        DatagramObject dh = (DatagramObject)dgram;
        endpoint.send(dh.dgram);
    }

    /**
     * Receive a datagram
     *
     * @param     dgram        A datagram
     * @exception IOException  If an I/O error occurs
     */
    public void receive(Datagram dgram) throws IOException {
        DatagramObject dh = (DatagramObject)dgram;
        endpoint.receive(dh.dgram);
        dh.pointer = 0;
    }

    /**
     * Close the connection to the target.
     *
     * @exception IOException  If an I/O error occurs
     */
    public void close() throws IOException {
         endpoint.close();
    }

    /**
     * Get a new datagram object
     *
     * @return                 A new datagram
     */
    public Datagram newDatagram(int size) {
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
    public Datagram newDatagram(byte[] buf, int size) {
        DatagramObject dg = new DatagramObject(new DatagramPacket(buf,size));
        if(host != null) {
            try {
                dg.setAddress("datagram://"+host+":"+port);
            } catch(IOException x) {
                throw new RuntimeException("IOException in datagram::newDatagram");
            }
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
        DatagramObject dh = (DatagramObject)newDatagram(buf, size);
        dh.setAddress(addr);
        return dh;
    }

}
