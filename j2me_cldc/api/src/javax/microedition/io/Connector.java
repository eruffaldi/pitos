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
import com.sun.cldc.io.*;

/**
 * This class is a placeholder for the static methods used to create
 * all the connection objects.
 * <p>
 * This is done by dynamically looking up a class the name of which is formed from the
 * platform name and the protocol of the requested connection.
 *
 * The parameter string describing the target conforms to the URL format as described in RFC 2396.
 * This takes the general form:
 * <p>
 * <code>{scheme}:[{target}][{parms}]</code>
 * <p>
 * Where <code>{scheme}</code> is the name of a protocol such as <i>http</i>}.<p>
 * The <code>{target}</code> is normally some kind of network address, but protocols may
 * regard this as a fairly flexible field when the connection is not network oriented.<p>
 * Any <code>{parms}</code> are formed as a series of equates on the form ";x=y" such as
 * ;type=a.
 * <p>
 * An option second parameter may be specified to the open function. The is a mode flag that
 * indicated to the protocol handler the intentions of the calling code. The options
 * here are to specify if the connection is going to be read (READ), written (WRITE),
 * or both (READ_WRITE). The validity of these flag settings is protocol dependent.
 * For instance a connection for a printer would not allow read access, and would
 * throw an IllegalArgumentException if this was attempted. Omitting this parameter
 * results in READ_WRITE being used by default.
 * <p>
 * An optional third parameter is a boolean flag to indicate if the calling code has
 * been written in such a way as to handle timeout exceptions. If this is selected
 * the protocol may throw an InterruptedIOException when it detects a timeout condition.
 * This flag is only a hint to the protocol handler and it is no guarantee that such exceptions
 * will be throws. Omitting this parameter results in no exceptions being thrown. The timeout
 * period is not specified in the open call because this is protocol specific. Protocol
 * implementors can either hardwire an appropriate value or read them from an external source
 * such as the system properties.
 * <p>
 * Because of the common occurrence of opening connections just to gain access to an input or
 * output stream four functions are provided for this purpose.
 *
 * See also: {@link DatagramConnection DatagramConnection}
 * for information relating to datagram addressing
 *
 * @author  Nik Shaylor
 * @version 1.1 1/7/2000
 */

public class Connector {

/*
 * Implementation note. the open parameter is used to form a class name in the form:
 * <p>
 * <code>com.sun.cldc.io.{platform}.{protocol}.Protocol</code>
 * <p>
 * The platform name is derived from the system by looking for the system property "j2me.platform".
 * If this property key is not found or the associated class is not present then one of two default
 * directories are used. These are called "j2me" and "j2se". If the property "j2me.configuration"
 * is non-null then "j2me" is used, otherwise "j2se" is assumed.
 * <p>
 * The system property "microedition.platform" can be used to change the root of the class space
 * used to lookup the classes.
 * <p>
 * The protocol name is derived from the parameter string describing the target of the
 * connection. This takes the from:
 * <p>
 * <code> {protocol}:[{target}][ {parms}] </code>
 * <p>
 * The protocol name is used to find the appropriate class and stripped from the target
 * name before being passed to open() method of a new installation of the class.
 */

    /**
     * Access mode
     */
    public final static int READ  = 1;

    /**
     * Access mode
     */
    public final static int WRITE = 2;

    /**
     * Access mode
     */
    public final static int READ_WRITE = (READ|WRITE);

    /**
     * The platform name
     */
    private static String platform;

    /**
     * True if we are running on a J2ME system
     */
    private static boolean j2me = false;

    /**
     * The root of the classes
     */
    private static String classRoot;

    /**
     * Class initializer
     */
    static {
        /* Work out if we are running on a J2ME system */
        if(System.getProperty("microedition.configuration") != null) {
            j2me = true;
        }

        /* Setup the platform name */
        platform = System.getProperty("microedition.platform");

        /* See if there is an alternate protocol class root */
        classRoot = System.getProperty("microedition.protocolpath");
        if(classRoot == null) {
            classRoot = "com.sun.cldc.io";
        }
    }


    /**
     * Prevent instantiation
     */
    private Connector(){}

    /**
     * Create and open a Connection
     *
     * @param string           The URL for the connection.
     * @return                 A new Connection object.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException if the requested connection
     *   cannot be make, or the protocol type does not exist.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static Connection open(String name) throws IOException {
        return open(name, READ_WRITE);
    }

    /**
     * Create and open a Connection
     *
     * @param string           The URL for the connection.
     * @param mode             The access mode.
     * @return                 A new Connection object.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException if the requested connection
     *   cannot be make, or the protocol type does not exist.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static Connection open(String name, int mode) throws IOException {
        return open(name, mode, false);
    }

    /**
     * Create and open a Connection
     *
     * @param string           The URL for the connection
     * @param mode             The access mode
     * @param timeouts         A flag to indicate that the called wants timeout exceptions
     * @return                 A new Connection object
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException if the requested connection
     * cannot be make, or the protocol type does not exist.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static Connection open(String name, int mode, boolean timeouts) throws IOException {

        /* If the "microedition.platform" property was found then try to get the class from there */
        if(platform != null) {
            try {
                return openPrim(name, mode, timeouts, platform);
            } catch(ClassNotFoundException x ) {
            }
        }

        /* Drop back to the default classes of "j2me" or "j2se" */
        try {
            return openPrim(name, mode, timeouts, j2me ? "j2me" : "j2se");
        } catch(ClassNotFoundException x ) {
        }

        throw new ConnectionNotFoundException("The requested protocol does not exist "+name);
    }

    /**
     * Create and open a Connection
     *
     * @param string           The URL for the connection
     * @param mode             The access mode
     * @param timeouts         A flag to indicate that the called wants timeout exceptions
     * @return                 A new Connection object
     *
     * @exception ClassNotFoundException  If the protocol cannot be found.
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException If the connection cannot be found.
     * @exception IOException If some other kind of I/O error occurs.

     * @exception IOException  If an I/O error occurs

     * @exception IllegalArgumentException If a parameter is invalid
     */
    private static Connection openPrim(String name, int mode, boolean timeouts, String platfrm)
                                     throws IOException, ClassNotFoundException {
        /* Test for null argument */
        if(name == null) {
            throw new IllegalArgumentException("Null URL");
        }

        /* Look for : as in "http:", "file:", or whatever */
        int colon = name.indexOf(':');

        /* Test for null argument */
        if(colon < 1) {
            throw new IllegalArgumentException("no ':' in URL");
        }

        try {
            String protocol;

            /* Strip off the protocol name */
            protocol = name.substring(0, colon);

            /* Strip the protocol name from the rest of the string */
            name = name.substring(colon+1);

            /* "file" is always an alias for "storage" */
            if(protocol.equals("file")) {
                protocol = "storage";
            }

            /* Using the platform and protocol names lookup a class to implement the connection */
            Class clazz = Class.forName(classRoot+"."+platfrm+"."+protocol+".Protocol");

            /* Construct a new instance */
            ConnectionBaseInterface uc = (ConnectionBaseInterface)clazz.newInstance();

            /* Open the connection, and return it */
            return uc.openPrim(name, mode, timeouts);

        } catch(InstantiationException x) {
            throw new IOException(x.toString());
        } catch(IllegalAccessException x) {
            throw new IOException(x.toString());
        } catch(ClassCastException x) {
            throw new IOException(x.toString());
        }
    }


    /**
     * Create and open a connection input stream
     *
     * @param  string          The URL for the connection.
     * @return                 A DataInputStream.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException If the connection cannot be found.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static DataInputStream openDataInputStream(String name) throws IOException {
        InputConnection con = (InputConnection)Connector.open(name, Connector.READ);
        DataInputStream is = con.openDataInputStream();
        con.close();
        return is;
    }

    /**
     * Create and open a connection output stream
     *
     * @param  string          The URL for the connection.
     * @return                 A DataOutputStream.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException If the connection cannot be found.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static DataOutputStream openDataOutputStream(String name) throws IOException {
        OutputConnection con = (OutputConnection)Connector.open(name, Connector.WRITE);
        DataOutputStream os = con.openDataOutputStream();
        con.close();
        return os;
    }

    /**
     * Create and open a connection input stream
     *
     * @param  string          The URL for the connection.
     * @return                 An InputStream.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException If the connection cannot be found.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static InputStream openInputStream(String name) throws IOException {
        return openDataInputStream(name);
    }

    /**
     * Create and open a connection output stream
     *
     * @param  string          The URL for the connection.
     * @return                 An OutputStream.
     *
     * @exception IllegalArgumentException If a parameter is invalid.
     * @exception ConnectionNotFoundException If the connection cannot be found.
     * @exception IOException  If some other kind of I/O error occurs.
     */
    public static OutputStream openOutputStream(String name) throws IOException {
        return openDataOutputStream(name);
    }

}
