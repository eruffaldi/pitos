/*
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


package com.sun.cldc.io.j2me.testhttp;

import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.util.Hashtable;
import java.util.Enumeration;
import java.io.ByteArrayOutputStream;
import javax.microedition.io.Connector;
import com.sun.cldc.io.connections.TestHttpConnection;
import com.sun.cldc.io.NetworkConnectionBase;
import com.sun.cldc.io.DateParser;
import javax.microedition.io.StreamConnection;

/**
 * This class implements the necessary functionality
 * for an HTTP connection. 
 */
public class Protocol extends NetworkConnectionBase implements TestHttpConnection {
    private int index;
    private String url;
    private String protocol;
    private String host;
    private String file;
    private String ref;
    private String query;
    private int port = 80;
    private int responseCode;
    private String responseMsg;
    private Hashtable reqProperties;
    private Hashtable headerFields;
    private String method;
    private int opens;
    private boolean connected;
    private PrivateOutputStream poster;
    private int mode;
    private StreamConnection netIO;
    private DataOutputStream output;
    private DataInputStream error;
    private DataInputStream input;

    /**
     * create a new instance of this class.
     * We are initially unconnected.
     */
    public Protocol() {
        reqProperties = new Hashtable();
        headerFields = new Hashtable();
        opens = 0;
        connected = false;
        method = GET;
        responseCode = -1;
        protocol = "http";
    }

    public void open(String url, int mode, boolean timeouts) throws IOException {
        if (opens > 0) {
            throw new IOException("already connected");
        }

        opens++;

        if (mode != Connector.READ && mode != Connector.WRITE
            && mode != Connector.READ_WRITE) {
            throw new IOException("illegal mode: " + mode);
        }

        this.url = new String(url);
        this.mode = mode;

        parseURL();
    }

    public void close() throws IOException {
        if (--opens == 0 && connected) disconnect();
    }

    public InputStream openInputStream() throws IOException {
        return openDataInputStream();
    }

    public DataInputStream openDataInputStream() throws IOException {
        connect();
        opens++;
        return new DataInputStream(new PrivateInputStream(input));
    }

    public OutputStream openOutputStream() throws IOException {
        return openDataOutputStream();
    }

    public DataOutputStream openDataOutputStream() throws IOException {
        if (mode != Connector.WRITE && mode != Connector.READ_WRITE) {
            throw new IOException("read-only connection");
        }

        opens++;
        poster = new PrivateOutputStream();
        return new DataOutputStream(poster);
    }

    class PrivateInputStream extends InputStream {
        private InputStream worker;

        public PrivateInputStream(InputStream worker) {
            this.worker = worker;
        }

        public int read() throws IOException {
            return worker.read();
        }

        public int read(byte[] buf, int offset, int length) throws IOException {
            return worker.read(buf, offset, length);
        }

        public int available() throws IOException {
            return worker.available();
        }

        public void close() throws IOException {
            worker.close();

            if (--opens == 0 && connected) disconnect();
        }
    }

    class PrivateOutputStream extends OutputStream {
        private ByteArrayOutputStream output;

        public PrivateOutputStream() {
            output = new ByteArrayOutputStream();
        }

        public void write(int b) throws IOException {
            output.write(b);
        }

        public void flush() throws IOException {
            if (output.size() > 0) connect();
        }

        public byte[] toByteArray() {
            return output.toByteArray();
        }

        public int size() {
            return output.size();
        }

        public void close() throws IOException {
            flush();

            if (--opens == 0 && connected) disconnect();
        }
    }

    public DataInputStream openErrorStream() throws IOException {
        if (getResponseCode() < HTTP_MULT_CHOICE) return null;
        else return new DataInputStream(error);
    }

    public String getURL() {
        return url;
    }

    public String getProtocol() {
        return protocol;
    }

    public String getHost() {
        return host;
    }

    public String getFile() {
        return file;
    }

    public String getRef() {
        return ref;
    }

    public String getQuery() {
        return query;
    }

    public int getPort() {
        return port;
    }

    public String getRequestMethod() {
        return method;
    }

    public void setRequestMethod(String method) throws IOException {
        if (connected) throw new IOException("connection already open");

        if (!method.equals(HEAD) && !method.equals(GET) && !method.equals(POST)) {
            throw new IOException("unsupported method: " + method);
        }

        this.method = new String(method);
    }

    public String getRequestProperty(String key) {
        return (String)reqProperties.get(key);
    }

    public void setRequestProperty(String key, String value) throws IOException {
        if (connected) throw new IOException("connection already open");
        reqProperties.put(key, value);
    }

    public int getResponseCode() throws IOException {
        connect();
        return responseCode;
    }

    public String getResponseMessage() throws IOException {
        connect();
        return responseMsg;
    }

    public long getLength() {
        try {connect();}
        catch (IOException x) {return -1;}
	    return getHeaderFieldInt("content-length", -1);
    }

    public String getType() {
        try {connect();}
        catch (IOException x) {return null;}
	    return getHeaderField("content-type");
    }

    public String getEncoding() {
        try {connect();}
        catch (IOException x) {return null;}
	    return getHeaderField("content-encoding");
    }

    public long getExpiration() {
	    return getHeaderFieldDate("expires", 0);
    }

    public long getDate() {
	    return getHeaderFieldDate("date", 0);
    }

    public long getLastModified() {
	    return getHeaderFieldDate("last-modified", 0);
    }

    public String getHeaderField(String name) {
        try {connect();}
        catch (IOException x) {return null;}
	    return (String)headerFields.get(toLowerCase(name));
    }

    public int getHeaderFieldInt(String name, int def) {
        try {connect();}
        catch (IOException x) {return def;}

    	try {
    	    return Integer.parseInt(getHeaderField(name));
    	} catch(Throwable t) {}

	    return def;
    }

    public long getHeaderFieldDate(String name, long def) {
        try {connect();}
        catch (IOException x) {return def;}

        try {
	    return DateParser.parse(getHeaderField(name));
    	} catch(Throwable t) {}

    	return def;
    }

    protected void connect() throws IOException {
        if (connected) return;

        // System.out.println("debug: connect >");

	// Open socket connection
	netIO = (StreamConnection)Connector.open("socket://" + host + ":" + port);

        output = netIO.openDataOutputStream();

        if (poster != null && getRequestProperty("Content-Length") == null) {
            setRequestProperty("Content-Length", "" + poster.size());
        }

        String reqLine = method + " " + getFile()
            + (getRef().equals("") ? "" : "#" + getRef())
            + (getQuery().equals("") ? "" : "?" + getQuery())
            + " HTTP/1.0\n";
        // System.out.print("debug: request line = " + reqLine);
        output.write((reqLine).getBytes());

        Enumeration reqKeys = reqProperties.keys();
        while (reqKeys.hasMoreElements()) {
            String key = (String)reqKeys.nextElement();
            String reqPropLine = key + ": " + reqProperties.get(key) + "\n";
            // System.out.print("debug: request prop = " + reqPropLine);
            output.write((reqPropLine).getBytes());
        }
        output.write("\n".getBytes());

        if (poster != null) {
            output.write(poster.toByteArray());
            // output.write("\n".getBytes());
            // System.out.println("debug: request body = " + new String(poster.toByteArray()));
        }
        output.flush();

        input = netIO.openDataInputStream();
        readResponseMessage(input);
        readHeaders(input);

        if (responseCode < HTTP_OK || responseCode > HTTP_PARTIAL) {
            error = input;
            input = null;
        }

        connected = true;
        // System.out.println("debug: connect <");
    }

    private void readResponseMessage(InputStream in) throws IOException {
        String line = readLine(in);
        int httpEnd, codeEnd;

        responseCode = -1;
        responseMsg = null;

 malformed: {
            if (line == null) break malformed;
            httpEnd = line.indexOf(' ');
            if (httpEnd < 0) break malformed;
    
            String httpVer = line.substring(0, httpEnd);
            if (!httpVer.startsWith("HTTP")) break malformed;
            if(line.length() <= httpEnd) break malformed;
    
            codeEnd = line.substring(httpEnd + 1).indexOf(' ');
            if (codeEnd < 0) break malformed;
            codeEnd += (httpEnd + 1);
            if(line.length() <= codeEnd) break malformed;
    
            try {responseCode = Integer.parseInt(line.substring(httpEnd + 1, codeEnd));}
            catch (NumberFormatException nfe) {break malformed;}
    
            responseMsg = line.substring(codeEnd + 1);
            // System.out.println("debug: response code = " + responseCode);
            // System.out.println("debug: response mesg = " + responseMsg);
            return;
        }  

        throw new IOException("malformed response message");
    }

    private void readHeaders(InputStream in) throws IOException {
        String line, key, value;
        int index;

        for (;;) {
            line = readLine(in);
            if (line == null || line.equals("")) return;

            index = line.indexOf(':');
            if (index < 0) throw new IOException("malformed header field");

            key = line.substring(0, index);
            if (key.length() == 0) throw new IOException("malformed header field");

            if (line.length() <= index + 2) value = "";
            else value = line.substring(index + 2);
            headerFields.put(toLowerCase(key), value);
        }
    }

    private String readLine(InputStream in) {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        int c;

        for (;;) {
            try {
                c = in.read();
                if (c < 0) return null;
                if (c == '\r') continue;
            } catch (IOException ioe) {return null;}
            if (c == '\n') break;
            bos.write(c);
        }

        // System.out.println("debug: read line = \"" + new String(bos.toByteArray()) + "\"");
        return new String(bos.toByteArray(), 0, bos.size());
    }

    protected void disconnect() throws IOException {
        //if (!connected) return;

        if (netIO != null) {
	    input.close();
	    output.close();
    	    netIO.close();
    	}

        responseCode = -1;
        responseMsg = null;
        connected = false;
    }

    private String parseProtocol() throws IOException {
        int n = url.indexOf(':');
        if (n <= 0) throw new IOException("malformed URL");
        String token = url.substring(0, n);
        if (!token.equals("http")) {
            throw new IOException("protocol must be 'http'");
        }
        index = n + 1;
        return token;
    }

    private String parseHostname() throws IOException {
        String buf = url.substring(index);
        if (buf.startsWith("//")) {
            buf = buf.substring(2);
            index += 2;
        }
        int n = buf.indexOf(':');
        if (n < 0) n = buf.indexOf('/');
        if (n < 0) n = buf.length();
        String token = buf.substring(0, n);
        if (token.length() == 0) {
            throw new IOException("host name cannot be empty");
        }
        index += n;
        return token;
    }

    private int parsePort() throws IOException {
        int p = 80;
        String buf = url.substring(index);
        if (!buf.startsWith(":")) return p;
        buf = buf.substring(1);
        index++;
        int n = buf.indexOf('/');
        if (n < 0) n = buf.length();
        try { 
            p = Integer.parseInt(buf.substring(0, n));
            if (p <= 0) {
                throw new NumberFormatException();
            }
        }catch (NumberFormatException nfe) {
            throw new IOException("invalid port");
        }
        index += n;
        return p;
    }

    private String parseFile() throws IOException {
        String token = "/";
        String buf = url.substring(index);
        if (buf.length() == 0) return token;
        if (!buf.startsWith(token)) {
            throw new IOException("invalid path");
        }
        int n = buf.indexOf('#');
        if (n < 0) n = buf.indexOf('?');
        if (n < 0) n = buf.length();
        token = buf.substring(0, n);
        index += n;
        return token;
    }

    private String parseRef() throws IOException {
        String buf = url.substring(index);
        if (buf.length() == 0 || buf.charAt(0) == '?') return "";
        if (!buf.startsWith("#")) {
            throw new IOException("invalid ref");
        }
        int n = buf.indexOf('?');
        if (n < 0) n = buf.length();
        index += n;
        return buf.substring(1, n);
    }

    private String parseQuery() throws IOException {
        String buf = url.substring(index);
        if (buf.length() == 0) return "";
        if (!buf.startsWith("?")) {
            throw new IOException("invalid query");
        }
        return buf.substring(1);
    }

    protected synchronized void parseURL() throws IOException {
        index = 0;
        // protocol = parseProtocol();
        // System.out.println("parseURL(): protocol = " + protocol);
        // System.out.println("index = " + index);
        host = parseHostname();
        // System.out.println("parseURL(): host = " + host);
        // System.out.println("index = " + index);
        port = parsePort();
        // System.out.println("parseURL(): port = " + port);
        // System.out.println("index = " + index);
        file = parseFile();
        // System.out.println("parseURL(): file = " + file);
        // System.out.println("index = " + index);
        ref = parseRef();
        // System.out.println("parseURL(): ref = " + ref);
        // System.out.println("index = " + index);
        query = parseQuery();
        // System.out.println("parseURL(): query = " + query);
        // System.out.println("index = " + index);
    }

    private String toLowerCase(String string) {
        StringBuffer low = new StringBuffer();

        for (int i=0; i<string.length(); i++) {
            low.append(Character.toLowerCase(string.charAt(i)));
        }

        return low.toString();
    }

}

