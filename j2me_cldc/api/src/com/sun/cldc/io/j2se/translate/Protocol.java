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

package com.sun.cldc.io.j2se.translate;

import java.io.*;
import java.util.*;
import javax.microedition.io.*;
import com.sun.cldc.io.j2se.*;

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
public class Protocol extends com.sun.cldc.io.j2me.translate.Protocol {
}










