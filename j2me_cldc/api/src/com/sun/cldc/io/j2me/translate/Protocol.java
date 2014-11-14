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

package com.sun.cldc.io.j2me.translate;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;
import com.sun.cldc.io.*;
import com.sun.cldc.io.connections.*;

/**
 * This implements the default "translate:" protocol
 * <p>
 * This is an experimental type of DirectoryConnection that could be used to translate localized strings.
 * It implements a strange sort of write-only memory, that uses the name of the selected output item
 * to get a string from the system properties, and then mearges in data written to the connection.
 * The data forms parameters for the selected localized string. When all the parameters are written the
 * resulting string can be returned  using the <code>dc.toString();</code> method.
 * <p>
 * Example:
 * <pre>
 *    DirectoryConnection dc = (DirectoryConnection)Connector.open("translate:US", Connector.WRITE);
 *    dc.selectItem("test");
 *    dc.writeInt(0);
 *    dc.writeChars("1");
 *    dc.writeChar('2');
 *    dc.writeLong(3);
 *    dc.writeShort(4);
 *    dc.writeByte(5);
 *    dc.writeBoolean(true);
 *    dc.writeBoolean(false);
 *    dc.writeChar('8');
 *    dc.writeChars("nine");
 *    String result = dc.toString();
 * </pre>
 * In the above example the call <code>dc.selectItem("test");</code> selects the string to be translated.
 * This string would normally be fetched from the system properties, but in this example the text string is
 * hardcoded in this class. The prototype string is:
 * <pre>
      "%% zero %0 one %1 two %2 three %3 four %4 five %5 six %6 seven %7 eight %8 nine %9 %!";
 * </pre>
 * Which results in the following string being returned by <code>dc.toString();</code>
 * <pre>
      "% zero 0 one 1 two 2 three 3 four 4 five 5 six true seven false eight 8 nine nine !";
 * </pre>
 * <p>
 *
 *
 * @author  Nik Shaylor
 * @version 2.0 2/21/2000
 */
public class Protocol extends com.sun.cldc.io.j2me.storage.ProtocolBase implements DirectoryConnection {

    private static final int NPARMS = 10;

    String language;
    String selection;
    int    index;
    String[] values = new String[NPARMS];

    public Connection open0(String openName, String parms, int mode) throws IOException {
        if(parms.length() > 0) {
            throw new IllegalArgumentException("No Parms allowed");
        }
        if(mode != Connector.WRITE) {
            throw new IllegalArgumentException("Mode != WRITE");
        }
        language = openName;
        return this;
    }

    public void close0() throws IOException{
    }

    public long getAvailableSpace0(){
        return Long.MAX_VALUE;
    }

    public int getItemCount0() {
        return 0;
    }

    public boolean selectFirstItem0() throws IOException {
        return false;
    }

    public boolean selectNextItem0() throws IOException {
        return false;
    }

    public boolean selectItem0(String name) throws IOException {
        selection = getSelection(name);
        if(selection == null) {
            selection = System.getProperty("microedition.string."+language+"."+name);
        }
        index = 0;
        return selection != null;
    }

    /*
     * selectItemByInt0
     */
    public boolean selectItemByInt0(int i) throws IOException {
        throw new SecurityException();
    }

    public void deselectItem0() throws IOException {
        selection = null;
    }

    public boolean isSelected0() {
        return selection != null;
    }

    public void create0() throws IOException{
        throw new SecurityException();
    }

    public void createName0(String name) throws IOException {
        throw new SecurityException();
    }

    public void createNameByInt0(int name) throws IOException {
        throw new SecurityException();
    }

    public void createDirectory0(String name) throws IOException {
        throw new SecurityException();
    }

    public void delete0() throws IOException {
        throw new SecurityException();
    }

    public void rename0(String name2) throws IOException {
        throw new SecurityException();
    }

    public void renameByInt0(int name2) throws IOException {
        throw new SecurityException();
    }


    public void renameDirectory0(String name2) throws IOException {
        throw new SecurityException();
    }

    public void renameBtInt0(int name2) throws IOException {
        throw new SecurityException();
    }

    public long getLength0() throws IOException {
        throw new SecurityException();
    }

    public void setLength0(long len) throws IOException {
        throw new SecurityException();
    }

    public long getModificationDate0() throws IOException {
        throw new SecurityException();
    }

    public String getItemName0() throws IOException {
        throw new SecurityException();
    }

    public int getItemNumber0() throws IOException {
        throw new SecurityException();
    }

    public boolean isDirectory0() throws IOException {
        throw new SecurityException();
    }

    public boolean canRead0() throws IOException {
        return false;
    }

    public boolean canWrite0() throws IOException {
        return true;
    }

    public void setReadable0(boolean tf) throws IOException {
        throw new SecurityException();
    }

    public void setWritable0(boolean tf) throws IOException {
    }

    public int available0() throws IOException {
        throw new SecurityException();
    }

    public void seek0(long pos) throws IOException {
        throw new SecurityException();
    }

    public long getPosition0() throws IOException {
        throw new SecurityException();
    }

    public int read0() throws IOException {
        throw new SecurityException();
    }

    public int readBytes0(byte b[], int off, int len) throws IOException {
        throw new SecurityException();
    }

    public void write0(int b) throws IOException {
        throw new SecurityException();
    }

    public void writeBytes0(byte b[], int off, int len) throws IOException {
        values[index++] = new String(b, off, len);
    }

    public void writeBoolean(boolean v) throws IOException {
        values[index++] = String.valueOf(v);
    }

    public void writeByte(int v) throws IOException {
        values[index++] = String.valueOf((byte)v);
    }

    public void writeShort(int v) throws IOException {
        values[index++] = String.valueOf((short)v);
    }

    public void writeChar(int v) throws IOException {
        values[index++] = String.valueOf((char)v);
    }

    public void writeInt(int v) throws IOException {
        values[index++] = String.valueOf(v);
    }

    public void writeLong(long v) throws IOException {
        values[index++] = String.valueOf(v);
    }

//    public void writeFloat(float v) throws IOException {
//        values[index++] = String.valueOf(v);
//    }

//    public void writeDouble(double v) throws IOException {
//        values[index++] = String.valueOf(v);
//    }

//    public void writeBytes(String s) throws IOException {
//        values[index++] = String.valueOf(v);
//    }

    public void writeChars(String s) throws IOException {
        values[index++] = s;
    }

    public String getSelection(String name) {
        if(name.equals("test")) {
            return "%% zero %0 one %1 two %2 three %3 four %4 five %5 six %6 seven %7 eight %8 nine %9 %!";
        }
        return null;
    }

    public String toString() {
        if(selection == null) {
            return "null";
        }
        StringBuffer sb = new StringBuffer();
        boolean escape = false;

        for(int i = 0 ; i < selection.length() ; i++) {
            int ch = selection.charAt(i);
            switch(ch) {

                case '%':
                    if(escape) {
                        sb.append('%');
                        escape = false;
                    } else {
                        escape = true;
                    }
                    break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    if(escape) {
                        sb.append(values[ch-'0']);
                        escape = false;
                        break;
                    }
                    /* drop thru */

                default:
                    sb.append((char)ch);
                    escape = false;
                    break;
            }
        }
        return sb.toString();
    }
}










