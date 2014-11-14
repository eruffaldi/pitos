/*
 *
 * @(#)HttpConnection.java      1.0 99/11/15
 * Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 */
package com.sun.cldc.io.connections;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.DataInputStream;
import javax.microedition.io.ContentConnection;

/**
 * This interface defines the necessary methods and constants
 * for an HTTP connection.
 * <P>
 * HTTP is a request-response protocol in which the parameters of
 * request must be set before the request is sent.
 * The connection exists in one of two states:
 * <UL>
 * <LI> Setup, in which the connection has not been made to the server.
 * <LI> Connected, in which the connection has been made, request parameters
 * have been sent and the response is expected.
 * </UL>
 * The following methods may be invoked only in the Setup state:
 * <UL>
 * <LI> <CODE> setRequestMethod</CODE>
 * <LI> <CODE> setRequestProperty</CODE>
 * </UL>
 *
 * The transition from Setup to Connected is caused by any method that
 * requires data to be sent to or received from the server. <p>
 *
 * The following methods cause the transition to the Connected state
 * <UL>
 * <LI> <CODE> openInputStream</CODE>
 * <LI> <CODE> openOutputStream</CODE>
 * <LI> <CODE> getLength</CODE>
 * <LI> <CODE> getType</CODE>
 * <LI> <CODE> getEncoding</CODE>
 * <LI> <CODE> getHeaderField</CODE>
 * <LI> <CODE> getHeaderFieldInt</CODE>
 * <LI> <CODE> getExpiration</CODE>
 * <LI> <CODE> getDate</CODE>
 * <LI> <CODE> getLastModified</CODE>
 * <LI> <CODE> getResponseCode</CODE>
 * <LI> <CODE> getResponseMessage</CODE>
 * <LI> <CODE> getErrorStream</CODE>
 * </UL>
 * <P>
 * The following methods may be invoked at any time.
 * <UL>
 * <LI> <CODE> close</CODE>
 * <LI> <CODE> getRequestMethod</CODE>
 * <LI> <CODE> getRequestProperty</CODE>
 * <LI> <CODE> getURL</CODE>
 * <LI> <CODE> getProtocol</CODE>
 * <LI> <CODE> getHost</CODE>
 * <LI> <CODE> getFile</CODE>
 * <LI> <CODE> getRef</CODE>
 * <LI> <CODE> getPort</CODE>
 * <LI> <CODE> getQuery</CODE>
 * </UL>
 *
 * Please note the following: The Connector class defines the following
 * convenience methods for retrieving an input or output stream directly
 * for a specified URL:
 *
 * <UL>
 * <LI> <CODE> InputStream openDataInputStream(String url) </CODE>
 * <LI> <CODE> DataInputStream openDataInputStream(String url) </CODE>
 * <LI> <CODE> OutputStream openOutputStream(String url) </CODE>
 * <LI> <CODE> DataOutputStream openDataOutputStream(String url) </CODE>
 * </UL>
 *
 * Please be aware that using these methods implies certain restrictions.
 * You will not get a reference to the actual connection, but rather just
 * references to the input or output stream of the connection. Not having
 * a reference to the connection means that you will not be able to manipulate
 * or query the connection directly. This in turn means that you will not
 * be able to call any of the following methods:
 *
 * <UL>
 * <LI> <CODE> getRequestMethod() </CODE>
 * <LI> <CODE> setRequestMethod() </CODE>
 * <LI> <CODE> getRequestProperty() </CODE>
 * <LI> <CODE> setRequestProperty() </CODE>
 * <LI> <CODE> getLength() </CODE>
 * <LI> <CODE> getType() </CODE>
 * <LI> <CODE> getEncoding() </CODE>
 * <LI> <CODE> getHeaderField() </CODE>
 * <LI> <CODE> getResponseCode() </CODE>
 * <LI> <CODE> getResponseMessage() </CODE>
 * <LI> etc.
 * </UL>
 *
 * If you require that level of control, you need to create connections in
 * the following way:
 *
 * <P><HR><BLOCKQUOTE><PRE>
 * HttpConnection conn = (HttpConnection)Connector.open(url, mode, timouts);
 * int responseCode = conn.getResponseCode();
 * if (responseCode == HttpConnection.HTTP_OK) {
 *     InputStream in = conn.getInputStream();
 *     ...
 * }
 * ...
 * </PRE></BLOCKQUOTE><HR>
 *
 */
public interface TestHttpConnection extends ContentConnection {
    /**  HTTP Head method */
    public final static String HEAD = "HEAD";
    /**  HTTP Get method */
    public final static String GET = "GET";
    /**  HTTP Post method */
    public final static String POST = "POST";

    /* 2XX: generally "OK" */
    public static final int HTTP_OK = 200;
    public static final int HTTP_CREATED = 201;
    public static final int HTTP_ACCEPTED = 202;
    public static final int HTTP_NOT_AUTHORITATIVE = 203;
    public static final int HTTP_NO_CONTENT = 204;
    public static final int HTTP_RESET = 205;
    public static final int HTTP_PARTIAL = 206;

    /* 3XX: relocation/redirect */
    public static final int HTTP_MULT_CHOICE = 300;
    public static final int HTTP_MOVED_PERM = 301;
    public static final int HTTP_MOVED_TEMP = 302;
    public static final int HTTP_SEE_OTHER = 303;
    public static final int HTTP_NOT_MODIFIED = 304;
    public static final int HTTP_USE_PROXY = 305;

    /* 4XX: client error */
    public static final int HTTP_BAD_REQUEST = 400;
    public static final int HTTP_UNAUTHORIZED = 401;
    public static final int HTTP_PAYMENT_REQUIRED = 402;
    public static final int HTTP_FORBIDDEN = 403;
    public static final int HTTP_NOT_FOUND = 404;
    public static final int HTTP_BAD_METHOD = 405;
    public static final int HTTP_NOT_ACCEPTABLE = 406;
    public static final int HTTP_PROXY_AUTH = 407;
    public static final int HTTP_CLIENT_TIMEOUT = 408;
    public static final int HTTP_CONFLICT = 409;
    public static final int HTTP_GONE = 410;
    public static final int HTTP_LENGTH_REQUIRED = 411;
    public static final int HTTP_PRECON_FAILED = 412;
    public static final int HTTP_ENTITY_TOO_LARGE = 413;
    public static final int HTTP_REQ_TOO_LONG = 414;
    public static final int HTTP_UNSUPPORTED_TYPE = 415;

    /* 5XX: server error */
    public static final int HTTP_SERVER_ERROR = 500;
    public static final int HTTP_INTERNAL_ERROR = 501;
    public static final int HTTP_BAD_GATEWAY = 502;
    public static final int HTTP_UNAVAILABLE = 503;
    public static final int HTTP_GATEWAY_TIMEOUT = 504;
    public static final int HTTP_VERSION = 505;

    /**
     * Open the connection to http or https.
     * @param name the full http URL to be opened.
     * @param mode READ, READ_WRITE, or WRITE as defined in Connector
     * @exception IOException is throws if any error occurs.
     */
    public void open(String name, int mode, boolean timeouts) throws IOException;

    /**
     * Close the connection.
     * @exception IOException is thrown if any error occurs.
     */
    public void close() throws IOException;

    /**
     * Open the input stream.
     * Opening the stream flushes the any request headers and content
     * that have been written to the OutputStream.
     * @returns an InputStream with the response to the request.
     * @exception IOException is throws if any error occurs.
     */
    public InputStream openInputStream() throws IOException;

    /**
     * Open the OutputStream.
     * If the Connection has been opened for read only (Connector.READ)
     * Then an IOException is thrown.
     * This opens the connection and writes out any request headers
     * that have been set.
     * @returns an OutputStream ready to which the contents can be written.
     * @exception IOException is throws if any error occurs.
     */
    public OutputStream openOutputStream() throws IOException;

   /**
    * Returns the error stream if the connection failed
    * but the server sent useful data nonetheless. The
    * typical example is when an HTTP server responds
    * with a 404, but the server sent an HTML
    * help page with suggestions as to what to do.
    *
    * <p>This method will not cause a connection to be initiated.
    * If there the connection was not connected, or if the server
    * did not have an error while connecting or if the server did
    * have an error but there no error data was sent, this method
    * will return null. This is the default.
    *
    * @return an error stream if any, null if there have been
    * no errors, the connection is not connected or the server
    * sent no useful data.
    */
    public DataInputStream openErrorStream() throws IOException;

    /**
     * Returns the value of the <code>content-length</code> header field.
     *
     * @return  the content length of the resource that this connection's URL
     *          references, or <code>-1</code> if the content length is
     *          not known.
     */
    public long getLength();

    /**
     * Returns the value of the <code>content-type</code> header field.
     *
     * @return  the content type of the resource that the URL references,
     *          or <code>null</code> if not known.
     */
    public String getType();

    /**
     * Returns the value of the <code>content-encoding</code> header field.
     *
     * @return  the content encoding of the resource that the URL references,
     *          or <code>null</code> if not known.
     */
    public String getEncoding();

    /**
     * Returns the name of the specified header field.
     *
     * @param   name   the name of a header field.
     * @return  the value of the named header field, or <code>null</code>
     *          if there is no such field in the header.
     */
    public String getHeaderField(String name);

    /** Return a string representation of the URL for this connection. */
    public String getURL();

    /**
     * Returns the protocol name of the URL of this <code>HttpConnection</code>.
     *
     * @return  the protocol of the URL of of this <code>HttpConnection</code>.
     */
    public String getProtocol();

    /**
     * Returns the host name of the URL of this <code>HttpConnection</code>.
     *
     * @return  the host name of the URL of of this <code>HttpConnection</code>.
     */
    public String getHost();

    /**
     * Returns the file portion of the URL of this <code>HttpConnection</code>.
     *
     * @return  the file portion of the URL of of this <code>HttpConnection</code>.
     */
    public String getFile();

    /**
     * Returns the ref portion of the URL of this <code>HttpConnection</code>.
     *
     * @return  the ref portion of the URL of of this <code>HttpConnection</code>.
     */
    public String getRef();

    /**
     * Returns the query portion of the URL of this <code>HttpConnection</code>.
     *
     * @return  the query portion of the URL of of this <code>HttpConnection</code>.
     */
    public String getQuery();

    /**
     * Returns the TCP/IP port of the URL of this <code>HttpConnection</code>.
     *
     * @return  the TCP/IP port of the URL of of this <code>HttpConnection</code>.
     */
    public int getPort();

    /**
     * Get the request method.
     * @return the HTTP request method
     */
    public String getRequestMethod();

    /**
     * Set the method for the URL request, one of:
     * <UL>
     *  <LI>GET
     *  <LI>POST
     *  <LI>HEAD
     * </UL> are legal, subject to protocol restrictions.  The default
     * method is GET.
     *
     * @param method the HTTP method
     * @exception IOException if the method cannot be reset or if
     *              the requested method isn't valid for HTTP.
     */
    public void setRequestMethod(String method) throws IOException;

    /**
     * Returns the value of the named general request property for this
     * connection.
     *
     * @param key the keyword by which the request is known (e.g., "accept").
     * @return  the value of the named general request property for this
     *           connection.
     */
    public String getRequestProperty(String key);

    /**
     * Sets the general request property. If a property with the key already
     * exists, overwrite its value with the new value.
     *
     * <p> NOTE: HTTP requires all request properties which can
     * legally have multiple instances with the same key
     * to use a comma-separated list syntax which enables multiple
     * properties to be appended into a single property.
     *
     * @param   key     the keyword by which the request is known
     *                  (e.g., "<code>accept</code>").
     * @param   value   the value associated with it.
     */
    public void setRequestProperty(String key, String value) throws IOException;

    /**
     * Returns the HTTP response status code. It parses
     * responses like:
     * <PRE>
     * HTTP/1.0 200 OK
     * HTTP/1.0 401 Unauthorized </PRE>
     * and extracts the integers 200 and 401 respectively.
     * Returns -1 if the response code cannot be discerned
     * from the response (i.e., the response is not valid HTTP).
     * @throws IOException if an error occurred connecting to the server.
     * @return the HTTP Status-Code
     */
    public int getResponseCode() throws IOException;

    /**
     * Gets the HTTP response message, if any, returned along with the
     * response code from a server.  From responses like:
     * <PRE>
     * HTTP/1.0 200 OK
     * HTTP/1.0 404 Not Found
     * </PRE>
     * Extracts the Strings "OK" and "Not Found" respectively.
     * Returns null if none could be discerned from the responses
     * (the result was not valid HTTP).
     * @throws IOException if an error occurred connecting to the server.
     * @return the HTTP response message, or <code>null</code>
     */
    public String getResponseMessage() throws IOException;

    /**
     * Returns the value of the <code>expires</code> header field.
     *
     * @return  the expiration date of the resource that this URL references,
     *          or 0 if not known. The value is the number of milliseconds since
     *          January 1, 1970 GMT.
     */
    public long getExpiration();

    /**
     * Returns the value of the <code>date</code> header field.
     *
     * @return  the sending date of the resource that the URL references,
     *          or <code>0</code> if not known. The value returned is the
     *          number of milliseconds since January 1, 1970 GMT.
     */
    public long getDate();

    /**
     * Returns the value of the <code>last-modified</code> header field.
     * The result is the number of milliseconds since January 1, 1970 GMT.
     *
     * @return  the date the resource referenced by this
     *          <code>HttpConnection</code> was last modified, or 0 if not known.
     */
    public long getLastModified();

    /**
     * Returns the value of the named field parsed as a number.
     * <p>
     * This form of <code>getHeaderField</code> exists because some
     * connection types (e.g., <code>http-ng</code>) have pre-parsed
     * headers. Classes for that connection type can override this method
     * and short-circuit the parsing.
     *
     * @param   name      the name of the header field.
     * @param   Default   the default value.
     * @return  the value of the named field, parsed as an integer. The
     *          <code>Default</code> value is returned if the field is
     *          missing or malformed.
     */
    public int getHeaderFieldInt(String name, int def);

    /**
     * Returns the value of the named field parsed as date.
     * The result is the number of milliseconds since January 1, 1970 GMT
     * represented by the named field.
     * <p>
     * This form of <code>getHeaderField</code> exists because some
     * connection types (e.g., <code>http-ng</code>) have pre-parsed
     * headers. Classes for that connection type can override this method
     * and short-circuit the parsing.
     *
     * @param   name     the name of the header field.
     * @param   Default   a default value.
     * @return  the value of the field, parsed as a date. The value of the
     *          <code>Default</code> argument is returned if the field is
     *          missing or malformed.
     */
    public long getHeaderFieldDate(String name, long def);
}

