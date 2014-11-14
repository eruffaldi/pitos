/*
 *	LineNumberTableEntry.java	1.3	97/05/27 SMI
 *
 * Copyright (c) 1997 Sun Microsystems, Inc. All Rights Reserved.
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

package components;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/*
 * Classes compiled with -g have line number attributes on their
 * code. These give the correspondence between Java bytecode relative PC
 * and source lines (when used with the filename, too!)
 */
public
class LineNumberTableEntry {
    public int startPC;
    public int lineNumber;

    public static final int size = 4; // bytes in class files

    LineNumberTableEntry( int s, int l ){
	startPC = s;
	lineNumber = l;
    }

    public void write( DataOutput o ) throws IOException {
	o.writeShort( startPC );
	o.writeShort( lineNumber );
    }
}

